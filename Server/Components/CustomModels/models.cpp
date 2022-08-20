/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/pool_impl.hpp>
#include <Server/Components/CustomModels/custommodels.hpp>
#include <sdk.hpp>
#include <netcode.hpp>
#include <httplib/httplib.h>
#include <filesystem>
#include "crc32.hpp"

using namespace Impl;

constexpr uint8_t RPC_FinishedDownload = 184u;
constexpr uint8_t RPC_RequestTXD = 182u;
constexpr uint8_t RPC_RequestDFF = 181u;

enum class ModelDownloadType : uint8_t {
    DFF = 1,
    TXD = 2
};

struct ModelFile {
    String name;
    uint32_t checksum;
    size_t size;

    ModelFile(StringView modelsPath, StringView fileName)
        : name(fileName)
        , size(GetFileCRC32Checksum(String(modelsPath) + "/" + String(fileName), checksum))
    {
    }
};

class ModelInfo final {
private:
    ModelType type_;
    int32_t baseId_;
    int32_t newId_;
    ModelFile dff_;
    ModelFile txd_;
    int32_t worldId_;
    uint8_t timeOn_;
    uint8_t timeOff_;

public:
    ModelInfo(ModelType type, int32_t id, int32_t baseId, ModelFile dff, ModelFile txd, int32_t worldId, uint8_t timeOn, uint8_t timeOff)
        : type_(type)
        , baseId_(baseId)
        , newId_(id)
        , dff_(dff)
        , txd_(txd)
        , worldId_(worldId)
        , timeOn_(timeOn)
        , timeOff_(timeOff)
    {
    }

    const ModelFile& getTXD() { return txd_; }
    const ModelFile& getDFF() { return dff_; }

    void write(NetCode::RPC::ModelRequest& modelInfo) const
    {
        modelInfo.type = static_cast<uint8_t>(type_);
        modelInfo.virtualWorld = worldId_;
        modelInfo.baseId = baseId_;
        modelInfo.newId = newId_;
        modelInfo.dffChecksum = dff_.checksum;
        modelInfo.txdChecksum = txd_.checksum;
        modelInfo.dffSize = dff_.size;
        modelInfo.txdSize = txd_.size;
        modelInfo.timeOn = timeOn_;
        modelInfo.timeOff = timeOff_;
    }
};

class WebServer {
private:
    httplib::Server svr;
    std::thread thread;

    String address_ = "";
    uint16_t port_ = 0;

    String url = "";

public:
    WebServer(ICore* core, StringView modelsPath, StringView bind, uint16_t port)
        : address_(String(!bind.empty() ? bind.data() : "127.0.0.1"))
        , port_(port)
        , url("http://" + address_ + ':' + std::to_string(port) + '/')
    {

        svr.set_pre_routing_handler([&](const auto& req, auto& res) {
            if (req.method != "GET" || !req.has_header("User-Agent") || req.get_header_value("User-Agent") != "SAMP/0.3") {
                res.status = 401;
                return httplib::Server::HandlerResponse::Handled;
            }
            return httplib::Server::HandlerResponse::Unhandled;
        });

        if (svr.set_mount_point("/", modelsPath.data())) {
            thread = std::thread(&WebServer::run, this);
            thread.detach();

            // Wait some time.
            std::this_thread::sleep_for(Milliseconds(500));
        } else {
            core->logLn(LogLevel::Error, "Unable to mount models path (%.*s).", PRINT_VIEW(modelsPath));
        }
    }

    ~WebServer()
    {
        stop();
    }

    void run()
    {
        svr.listen(address_.c_str(), port_);
    }

    bool is_running()
    {
        return svr.is_running();
    }

    void stop()
    {
        if (svr.is_running())
            svr.stop();
    }

    StringView getUrl() const
    {
        return url;
    }
};

class PlayerCustomModelsData final : public IPlayerCustomModelsData {
private:
    uint32_t skin_ = 0;

public:
    uint32_t getCustomSkin() const
    {
        return skin_;
    }

    void setCustomSkin(const uint32_t skinModel)
    {
        skin_ = skinModel;
    }

    void reset()
    {
        skin_ = 0;
    }

    void freeExtension()
    {
        delete this;
    }
};

class CustomModelsComponent final : public ICustomModelsComponent, public PlayerEventHandler {
private:
    ICore* core = nullptr;
    IPlayerPool* players = nullptr;

    WebServer* webServer = nullptr;

    std::vector<ModelInfo*> storage;
    FlatHashMap<uint32_t, uint16_t> baseModels;
    FlatHashMap<uint32_t, std::pair<ModelDownloadType, ModelInfo*>> checksums;

    bool enabled = true;
    String modelsPath = "models";
    String cdn = "";

    DefaultEventDispatcher<PlayerModelsEventHandler> eventDispatcher;

    struct RequestDownloadLinkEventHandler : public SingleNetworkInEventHandler {
        CustomModelsComponent& self;

        RequestDownloadLinkEventHandler(CustomModelsComponent& modelsComponent)
            : self(modelsComponent)
        {
        }

        bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
        {
            uint32_t checksum;

            if (!bs.readUINT32(checksum))
                return false;

            const auto& itr = self.checksums.find(checksum);

            if (itr == self.checksums.end())
                return true;

            const auto& [type, model] = itr->second;
            const auto& file = type == ModelDownloadType::DFF ? model->getDFF() : model->getTXD();

            NetCode::RPC::ModelUrl urlRPC(self.getWebUrl().data() + file.name, static_cast<uint8_t>(type), file.checksum);
            PacketHelper::send(urlRPC, peer);

            return true;
        }
    } requestDownloadLinkHandler;

    struct FinishDownloadHandler : public SingleNetworkInEventHandler {
        CustomModelsComponent& self;

        FinishDownloadHandler(CustomModelsComponent& component)
            : self(component)
        {
        }

        bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
        {
            self.eventDispatcher.dispatch(
                &PlayerModelsEventHandler::onPlayerFinishedDownloading,
                peer);

            NetCode::RPC::DownloadCompleted dlcompleted;
            PacketHelper::send(dlcompleted, peer);
            return true;
        }
    } finishDownloadHandler;

public:
    StringView componentName() const override
    {
        return "CustomModels";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
    }

    CustomModelsComponent()
        : requestDownloadLinkHandler(*this)
        , finishDownloadHandler(*this)
    {
    }

    ~CustomModelsComponent()
    {
        core->removePerRPCInEventHandler<RPC_RequestTXD>(&requestDownloadLinkHandler);
        core->removePerRPCInEventHandler<RPC_RequestDFF>(&requestDownloadLinkHandler);
        core->removePerRPCInEventHandler<RPC_FinishedDownload>(&finishDownloadHandler);
        players->getEventDispatcher().removeEventHandler(this);

        if (webServer)
            delete webServer;
    }

    void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) override
    {
        if (defaults) {
            config.setBool("artwork.enabled", enabled);
            config.setString("artwork.cdn", cdn);
            config.setString("artwork.models_path", modelsPath);
        }
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        players = &core->getPlayers();
        players->getEventDispatcher().addEventHandler(this);

        enabled = *core->getConfig().getBool("artwork.enabled");
        modelsPath = String(core->getConfig().getString("artwork.models_path"));
        cdn = String(core->getConfig().getString("artwork.cdn"));

        core->addPerRPCInEventHandler<RPC_RequestTXD>(&requestDownloadLinkHandler);
        core->addPerRPCInEventHandler<RPC_RequestDFF>(&requestDownloadLinkHandler);
        core->addPerRPCInEventHandler<RPC_FinishedDownload>(&finishDownloadHandler);

        if (!cdn.empty()) {
            if (cdn.back() != '/') {
                cdn.push_back('/');
            }
            core->logLn(LogLevel::Message, "[artwork:info] Using CDN %.*s", PRINT_VIEW(cdn));
            return;
        }

        if (!enabled)
            return;

        if (!std::filesystem::exists(modelsPath.c_str()) || !std::filesystem::is_directory(modelsPath.c_str())) {
            std::filesystem::create_directory(modelsPath.c_str());
        }

        webServer = new WebServer(core, modelsPath, core->getConfig().getString("bind"), *core->getConfig().getInt("port"));
    }

    void onReady() override
    {
        if (!enabled || !webServer)
            return;

        if (webServer->is_running()) {
            core->logLn(LogLevel::Message, "[artwork:info] Web server is running on %.*s", PRINT_VIEW(webServer->getUrl()));
        } else {
            core->logLn(LogLevel::Error, "[artwork:error] Failed to start web server");
        }
    }

    void free() override
    {
        delete this;
    }

    void reset() override
    {
        for (auto ptr : storage) {
            delete ptr;
        }
        storage.clear();
        checksums.clear();
        baseModels.clear();
    }

    StringView getWebUrl()
    {
        return cdn.empty() ? webServer->getUrl() : cdn;
    }

    bool addCustomModel(ModelType type, int32_t id, int32_t baseId, StringView dffName, StringView txdName, int32_t virtualWorld = -1, uint8_t timeOn = 0, uint8_t timeOff = 0) override
    {
        if (!enabled) {
            return false;
        }

        if (type == ModelType::Skin && !(id >= 20000 && id <= 30000))
            return false;
        else if (type == ModelType::Object && !(id >= -30000 && id <= -1000))
            return false;
        else if (baseModels.find(id) != baseModels.end())
            return false;

        ModelFile dff(modelsPath, dffName);
        ModelFile txd(modelsPath, txdName);

        if (checksums.find(dff.checksum) != checksums.end()) {
            core->logLn(LogLevel::Error, "[artwork:error] There's already another custom model loaded with the same checksum as: %.*s", PRINT_VIEW(dffName));
            return false;
        }

        if (checksums.find(txd.checksum) != checksums.end()) {
            core->logLn(LogLevel::Error, "[artwork:error] There's already another custom model loaded with the same checksum as: %.*s", PRINT_VIEW(txdName));
            return false;
        }

        if (!dff.size) {
            core->logLn(LogLevel::Error, "[artwork:error] Bad file: %.*s", PRINT_VIEW(dffName));
            return false;
        }

        if (!txd.size) {
            core->logLn(LogLevel::Error, "[artwork:error] Cannot add custom model. %.*s doesn't exist", PRINT_VIEW(txdName));
            return false;
        }

        core->logLn(LogLevel::Message, "[artwork:crc] %.*s CRC = 0x%X", PRINT_VIEW(txdName), txd.checksum);
        core->logLn(LogLevel::Message, "[artwork:crc] %.*s CRC = 0x%X", PRINT_VIEW(dffName), dff.checksum);

        auto model = storage.emplace_back(new ModelInfo(type, id, baseId, dff, txd, virtualWorld, timeOn, timeOff));

        if (!model)
            return false;

        auto size = storage.size() - 1;
        NetCode::RPC::ModelRequest modelInfo(size - 1, size);
        model->write(modelInfo);

        for (IPlayer* player : players->entries()) {
            if (player->getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL)
                continue;

            PacketHelper::send(modelInfo, *player);
        }

        storage.emplace_back(model);
        baseModels.emplace(id, baseId);
        checksums.emplace(dff.checksum, std::make_pair(ModelDownloadType::DFF, model));
        checksums.emplace(txd.checksum, std::make_pair(ModelDownloadType::TXD, model));

        return true;
    }

    uint16_t getBaseModelId(int32_t modelId) const override
    {
        if (modelId >= 0 && modelId <= 20000)
            return modelId;

        auto itr = baseModels.find(modelId);

        if (itr == baseModels.end())
            return INVALID_MODEL_ID;

        return itr->second;
    }

    StringView getModelNameFromChecksum(uint32_t checksum) const override
    {
        const auto& itr = checksums.find(checksum);

        if (itr == checksums.end())
            return StringView();

        const auto& [type, model] = itr->second;
        const auto& file = type == ModelDownloadType::DFF ? model->getDFF() : model->getTXD();
        return file.name;
    }

    void onPlayerClientInit(IPlayer& player) override
    {
        if (player.getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL)
            return;

        const auto modelsCount = storage.size();
        for (auto i = 0; i != modelsCount; ++i) {
            NetCode::RPC::ModelRequest modelInfo(i, modelsCount);
            storage[i]->write(modelInfo);
            PacketHelper::send(modelInfo, player);
        }

        // If client reconnected (lost connection to the server) let's force it to download files if there are any.
        NetCode::RPC::SetPlayerVirtualWorld setWorld;
        setWorld.worldId = player.getVirtualWorld() + 1;
        PacketHelper::send(setWorld, player);
        setWorld.worldId--;
        PacketHelper::send(setWorld, player);
    }

    IEventDispatcher<PlayerModelsEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    void onPlayerConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerCustomModelsData(), true);

        if (player.getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL)
            return;
    }
};

COMPONENT_ENTRY_POINT()
{
    return new CustomModelsComponent();
}