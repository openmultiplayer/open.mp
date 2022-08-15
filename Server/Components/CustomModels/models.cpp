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

#include "crc32.hpp"

using namespace Impl;

constexpr uint8_t RPC_FinishedDownload = 184u;
constexpr uint8_t RPC_RequestTXD = 182u;
constexpr uint8_t RPC_RequestDFF = 181u;

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

class ModelInfo final : public PoolIDProvider, public NoCopy {
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

    const uint32_t getID() { return poolID; }

    const ModelFile& getTXD() { return txd_; }
    const ModelFile& getDFF() { return dff_; }

    void write(NetCode::RPC::ModelRequest& modelInfo) const
    {
        modelInfo.poolID = poolID;
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

class CustomModelsComponent final : public ICustomModelsComponent, public PlayerEventHandler {
private:

    ICore* core = nullptr;
    IPlayerPool* players = nullptr;

    WebServer* webServer = nullptr;

    DynamicPoolStorage<ModelInfo, ModelInfo, 0, MAX_CUSTOM_MODELS> storage;
    FlatHashMap<uint32_t, uint16_t> baseModels;
    FlatHashMap<uint32_t, std::pair<ModelDownloadType, ModelInfo*>> checksums;

    bool enabled = true;
    String modelsPath = "models";
    String cdn = "";

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

            auto& [type, model] = itr->second;
            const auto file = type == ModelDownloadType::DFF ? model->getDFF() : model->getTXD();

            NetCode::RPC::ModelUrl urlRPC(self.getWebUrl().data() + file.name, static_cast<uint8_t>(type), file.checksum);
            PacketHelper::send(urlRPC, peer);

            return true;
        }
    } requestDownloadLinkHandler;

    struct FinishDownloadHandler : public SingleNetworkInEventHandler {
        bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
        {
            // TODO: dispatch event

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
    {
    }

    ~CustomModelsComponent()
    {
        core->removePerRPCInEventHandler<RPC_RequestTXD>(&requestDownloadLinkHandler);
        core->removePerRPCInEventHandler<RPC_RequestDFF>(&requestDownloadLinkHandler);
        core->removePerRPCInEventHandler<RPC_FinishedDownload>(&finishDownloadHandler);

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

        enabled = *core->getConfig().getBool("artwork.enabled");
        modelsPath = String(core->getConfig().getString("artwork.models_path"));
        cdn = String(core->getConfig().getString("artwork.cdn"));
        
        core->addPerRPCInEventHandler<RPC_RequestTXD>(&requestDownloadLinkHandler);
        core->addPerRPCInEventHandler<RPC_RequestDFF>(&requestDownloadLinkHandler);
        core->addPerRPCInEventHandler<RPC_FinishedDownload>(&finishDownloadHandler);

        if (!cdn.empty()) {
            if (cdn.back() != '/')
                cdn.push_back('/');

            core->logLn(LogLevel::Message, "[artwork:info] Using CDN %.*s", PRINT_VIEW(cdn));
            return;
        }

        if (!enabled)
            return;

        webServer = new WebServer(core, modelsPath, core->getConfig().getString("bind"), *core->getConfig().getInt("port"));
    }

    void onReady() override
    {
        if (!enabled)
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
        storage.clear();
        checksums.clear();
        baseModels.clear();

        // TODO: Check if we need to send stop download RPC to connected players.
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
            core->logLn(LogLevel::Error, "[artwork:error] Bad file: %.*s", PRINT_VIEW(txdName));
            return false;
        }

        core->logLn(LogLevel::Message, "[artwork:crc] %.*s CRC = 0x%X", PRINT_VIEW(txdName), txd.checksum);
        core->logLn(LogLevel::Message, "[artwork:crc] %.*s CRC = 0x%X", PRINT_VIEW(dffName), dff.checksum);

        auto model = storage.emplace(type, id, baseId, dff, txd, virtualWorld, timeOn, timeOff);

        if (!model)
            return false;

        NetCode::RPC::ModelRequest modelInfo(storage.entries().size());
        model->write(modelInfo);

        for (IPlayer* player : players->entries()) {
            if (player->getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL)
                continue;

            PacketHelper::send(modelInfo, *player);
        }

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

    void sendModels(IPlayer& player) override
    {
        if (player.getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL)
            return;

        const auto modelsCount = storage.entries().size();
        for (ModelInfo* model : storage.entries()) {
            NetCode::RPC::ModelRequest modelInfo(modelsCount);
            model->write(modelInfo);
            PacketHelper::send(modelInfo, player);
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new CustomModelsComponent();
}