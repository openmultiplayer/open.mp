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
#include <httplib.h>
#include <ghc/filesystem.hpp>
#include "crc32.hpp"
#include <regex>
#include <shared_mutex>
#include "utils.hpp"

static auto rAddCharModel = std::regex(R"(AddCharModel\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*\"(.+)\"\s*,\s*\"(.+)\"\s*\)\s*;*)");
static auto rAddSimpleModel = std::regex(R"(AddSimpleModel\s*\(\s*(-?\d+)\s*,\s*(\d+)\s*,\s*(-\d+)\s*,\s*\"(.+)\"\s*,\s*\"(.+)\"\s*\)\s*;*)");
static auto rAddSimpleModelTimed = std::regex(R"(AddSimpleModelTimed\s*\(\s*(-?\d+)\s*,\s*(\d+)\s*,\s*(-\d+)\s*,\s*\"(.+)\"\s*,\s*\"(.+)\"\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*;*)");
static auto rUri = std::regex("[A-Za-z0-9-._~:/?#\\[\\]@!$&'()*+,;=]+"); // very loose interpretation of rfc3986

using namespace Impl;

struct ModelFile
{
	String name;
	uint32_t checksum;
	size_t size;

	ModelFile(StringView modelsPath, StringView fileName)
		: name(fileName)
		, size(GetFileCRC32Checksum(String(modelsPath) + "/" + String(fileName), checksum))
	{
	}
};

class ModelInfo final
{
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

	const int32_t getId() { return newId_; }
};

class WebServer
{
private:
	httplib::Server svr;
	std::thread thread;

	String address_ = "";
	uint16_t port_ = 0;

	String url = "";

	FlatHashMap<uint32_t, uint16_t> allowedIPs_;
	std::shared_mutex mutex_;

public:
	WebServer(ICore* core, StringView modelsPath, StringView bind, uint16_t port, StringView publicAddr, uint16_t threadsCount)
		: port_(port)
	{

		if (!bind.empty())
		{
			address_ = String(bind.data());
			url = "http://" + (publicAddr.empty() ? address_ : String(publicAddr.data())) + ':' + std::to_string(port) + '/';

			if (!publicAddr.empty())
			{
				core->logLn(LogLevel::Message, "Using public address %.*s:%d (local: %.*s:%d)", PRINT_VIEW(publicAddr), port, PRINT_VIEW(address_), port);
			}
			else
			{
				core->logLn(LogLevel::Warning, "No public address provided. If your public address differs from bind one please provide it in server config.");
			}
		}
		else
		{
			address_ = "127.0.0.1";
			url = "http://127.0.0.1:" + std::to_string(port) + '/';

			core->logLn(LogLevel::Warning, "No bind address provided. Attempting to start webserver on 127.0.0.1:%d", port);
		}

		svr.new_task_queue = [&]
		{
			return new httplib::ThreadPool(threadsCount);
		};

		svr.set_pre_routing_handler([&](const auto& req, auto& res)
			{
				if (req.method != "GET" || !req.has_header("User-Agent") || req.get_header_value("User-Agent") != "SAMP/0.3" || ((req.path.rfind(".txd") == std::string::npos) && (req.path.rfind(".dff") == std::string::npos)))
				{
					res.status = 401;
					return httplib::Server::HandlerResponse::Handled;
				}

				if (req.sockaddr.ss_family == AF_INET)
				{
					auto ip = reinterpret_cast<const struct sockaddr_in*>(&req.sockaddr)->sin_addr.s_addr;
					std::shared_lock<std::shared_mutex> lock(mutex_);
					if (allowedIPs_.find(ip) == allowedIPs_.end())
					{
						res.status = 401;
						return httplib::Server::HandlerResponse::Handled;
					}
				}
				else if (req.sockaddr.ss_family == AF_INET6)
				{
					// TODO: Add IPV6 support.
				}

				return httplib::Server::HandlerResponse::Unhandled;
			});

		if (svr.set_mount_point("/", modelsPath.data()))
		{
			thread = std::thread(&WebServer::run, this);
			thread.detach();

			// Wait some time.
			std::this_thread::sleep_for(Milliseconds(500));
		}
		else
		{
			core->logLn(LogLevel::Error, "Failed to start webserver: Unable to mount models path (%.*s).", PRINT_VIEW(modelsPath));
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

	void allowIPAddress(uint32_t ipAddress)
	{
		auto itr = allowedIPs_.find(ipAddress);

		std::unique_lock lock(mutex_);

		if (itr == allowedIPs_.end())
		{
			allowedIPs_.insert({ ipAddress, 1 });
		}
		else
		{
			++itr->second;
		}
	}

	void removeIPAddress(uint32_t ipAddress)
	{
		auto itr = allowedIPs_.find(ipAddress);
		if (itr == allowedIPs_.end())
		{
			return;
		}

		std::unique_lock lock(mutex_);

		if (itr->second > 1)
		{
			--itr->second;
		}
		else
		{
			allowedIPs_.erase(itr);
		}
	}
};

class PlayerCustomModelsData final : public IPlayerCustomModelsData
{
private:
	IPlayer& player;
	uint32_t skin_ = 0;
	std::pair<ModelDownloadType, uint32_t> requestedFile_;

public:
	PlayerCustomModelsData(IPlayer& player)
		: player(player)
		, requestedFile_ { ModelDownloadType::NONE, 0 }
	{
	}

	uint32_t getCustomSkin() const override
	{
		return skin_;
	}

	void setCustomSkin(const uint32_t skinModel) override
	{
		skin_ = skinModel;
	}

	void setRequestedFile(ModelDownloadType type, uint32_t checksum)
	{
		requestedFile_ = { type, checksum };
	}

	virtual bool sendDownloadUrl(StringView url) const override
	{
		if (requestedFile_.first == ModelDownloadType::NONE)
		{
			return false;
		}
		NetCode::RPC::ModelUrl downloadLink(url, static_cast<uint8_t>(requestedFile_.first), requestedFile_.second);
		PacketHelper::send(downloadLink, player);
		return true;
	}

	void reset() override
	{
		skin_ = 0;
		requestedFile_ = { ModelDownloadType::NONE, 0 };
	}

	void freeExtension() override
	{
		delete this;
	}
};

class CustomModelsComponent final : public ICustomModelsComponent, public PlayerConnectEventHandler
{
private:
	ICore* core = nullptr;
	IPlayerPool* players = nullptr;

	WebServer* webServer = nullptr;

	std::vector<ModelInfo*> storage;
	FlatHashMap<uint32_t, uint16_t> baseModels;
	FlatHashMap<uint32_t, std::pair<ModelDownloadType, ModelInfo*>> checksums;

	bool enabled = true;
	uint16_t modelsPort = 7777;
	String modelsPath = "models";
	String webServerBindAddress = "";
	String cdn = "";
	bool usingCdn = false;
	uint16_t httpThreads = 50; // default max_players is 50
	bool showCRCLogs = false;

	DefaultEventDispatcher<PlayerModelsEventHandler> eventDispatcher;

	struct RequestDownloadLinkEventHandler : public SingleNetworkInEventHandler
	{
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

			const auto& type = itr->second.first;

			PlayerCustomModelsData* data = queryExtension<PlayerCustomModelsData>(peer);
			if (data != nullptr)
			{
				data->setRequestedFile(type, checksum);
			}
			if (self.eventDispatcher.stopAtFalse(
					[&peer, &type, &checksum](PlayerModelsEventHandler* handler)
					{
						return handler->onPlayerRequestDownload(peer, type, checksum);
					}))
			{
				const auto& model = itr->second.second;
				const auto& file = type == ModelDownloadType::DFF ? model->getDFF() : model->getTXD();
				NetCode::RPC::ModelUrl urlRPC(httplib::detail::encode_url(self.getWebUrl().data() + file.name), static_cast<uint8_t>(type), file.checksum);
				PacketHelper::send(urlRPC, peer);
			}
			if (data != nullptr)
			{
				data->setRequestedFile(ModelDownloadType::NONE, 0);
			}
			return true;
		}
	} requestDownloadLinkHandler;

	struct FinishDownloadHandler : public SingleNetworkInEventHandler
	{
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
		if (!core)
		{
			return;
		}

		NetCode::RPC::RequestTXD::removeEventHandler(*core, &requestDownloadLinkHandler);
		NetCode::RPC::RequestDFF::removeEventHandler(*core, &requestDownloadLinkHandler);
		NetCode::RPC::FinishDownload::removeEventHandler(*core, &finishDownloadHandler);
		players->getPlayerConnectDispatcher().removeEventHandler(this);

		if (webServer)
		{
			delete webServer;
		}
	}

	void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) override
	{
		if (defaults)
		{
			config.setBool("artwork.enable", enabled);
			config.setString("artwork.cdn", cdn);
			config.setString("artwork.models_path", modelsPath);
			config.setInt("network.http_threads", httpThreads);
			config.setInt("artwork.port", modelsPort);
			config.setString("artwork.web_server_bind", webServerBindAddress);
			config.setBool("artwork.show_crc_logs", showCRCLogs);
		}
		else
		{
			// Set default values if options are not set.
			// We don't want to crash.
			if (config.getType("artwork.enable") == ConfigOptionType_None)
			{
				config.setBool("artwork.enable", enabled);
			}
			if (config.getType("artwork.cdn") == ConfigOptionType_None)
			{
				config.setString("artwork.cdn", cdn);
			}
			if (config.getType("artwork.models_path") == ConfigOptionType_None)
			{
				config.setString("artwork.models_path", modelsPath);
			}
			if (config.getType("network.http_threads") == ConfigOptionType_None)
			{
				config.setInt("network.http_threads", httpThreads);
			}
			// Add the web server port to support some old TCP plugin like Incognito's audio plugin
			if (config.getType("artwork.port") == ConfigOptionType_None)
			{
				config.setInt("artwork.port", modelsPort);
			}
			// We provide a way for users to set webserver bind address, sometimes they want it
			// To be different rather than 127.0.0.1 or server's public IP. For example, in some
			// Situations you want to bind it to 0.0.0.0, like in a docker container.
			if (config.getType("artwork.web_server_bind") == ConfigOptionType_None)
			{
				config.setString("artwork.web_server_bind", webServerBindAddress);
			}
			if (config.getType("artwork.show_crc_logs") == ConfigOptionType_None)
			{
				config.setBool("artwork.show_crc_logs", showCRCLogs);
			}
		}
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		players = &core->getPlayers();
		players->getPlayerConnectDispatcher().addEventHandler(this);

		enabled = *core->getConfig().getBool("artwork.enable");
		modelsPath = String(trim(core->getConfig().getString("artwork.models_path")));
		cdn = String(trim(core->getConfig().getString("artwork.cdn")));
		httpThreads = *core->getConfig().getInt("network.http_threads");
		webServerBindAddress = String(core->getConfig().getString("artwork.web_server_bind"));
		showCRCLogs = *core->getConfig().getBool("artwork.show_crc_logs");

		NetCode::RPC::RequestTXD::addEventHandler(*core, &requestDownloadLinkHandler);
		NetCode::RPC::RequestDFF::addEventHandler(*core, &requestDownloadLinkHandler);
		NetCode::RPC::FinishDownload::addEventHandler(*core, &finishDownloadHandler);

		if (!enabled)
			return;

		// Attempt to create models folder.
		try
		{
			ghc::filesystem::create_directory(modelsPath);
		}
		catch (ghc::filesystem::filesystem_error exception)
		{
			enabled = false;
			core->logLn(LogLevel::Error, "[artwork:error] Unable to create models path (%.*s).", PRINT_VIEW(modelsPath));
			core->logLn(LogLevel::Error, "%s", exception.what());
			return;
		}

		loadArtConfig();

		if (!cdn.empty())
		{
			if (!std::regex_match(cdn, rUri))
			{
				core->logLn(LogLevel::Warning, "[artwork:warn] CDN URL '%.*s' seems to be invalid, the CDN feature has been disabled.", PRINT_VIEW(cdn));
				return;
			}

			if (cdn.back() != '/')
			{
				cdn.push_back('/');
			}
			core->logLn(LogLevel::Message, "[artwork:info] Using CDN '%.*s'", PRINT_VIEW(cdn));
			usingCdn = true;
		}
	}

	void loadArtConfig()
	{
		std::ifstream artconfig { modelsPath + "/artconfig.txt" };

		if (artconfig.is_open())
		{
			core->logLn(LogLevel::Message, "[artwork:info] Loading artconfig.txt");
			std::string line;
			std::smatch match;
			while (std::getline(artconfig, line))
			{
				if (std::regex_match(line, match, rAddCharModel))
				{
					addCustomModel(ModelType::Skin, std::atoi(match[2].str().c_str()), std::atoi(match[1].str().c_str()), match[3].str(), match[4].str());
				}
				else if (std::regex_match(line, match, rAddSimpleModel))
				{
					addCustomModel(ModelType::Object, std::atoi(match[3].str().c_str()), std::atoi(match[2].str().c_str()), match[4].str(), match[5].str(), std::atoi(match[1].str().c_str()));
				}
				else if (std::regex_match(line, match, rAddSimpleModelTimed))
				{
					addCustomModel(ModelType::Object, std::atoi(match[3].str().c_str()), std::atoi(match[2].str().c_str()), match[4].str(), match[5].str(), std::atoi(match[1].str().c_str()), std::atoi(match[6].str().c_str()), std::atoi(match[7].str().c_str()));
				}
			}
		}
	}

	void startWebServer()
	{
		if (usingCdn || webServer)
		{
			return;
		}

		StringView bindAddress;
		StringView networkBindAddress = core->getConfig().getString("network.bind");
		if (webServerBindAddress.size())
		{
			bindAddress = StringView(webServerBindAddress.c_str(), webServerBindAddress.size());
		}
		else
		{
			if (!networkBindAddress.empty())
			{
				bindAddress = networkBindAddress;
			}
		}

		webServer = new WebServer(core, modelsPath, bindAddress, *core->getConfig().getInt("artwork.port"), core->getConfig().getString("network.public_addr"), httpThreads);

		if (webServer->is_running())
		{
			core->logLn(LogLevel::Message, "Web server is running on %.*s", PRINT_VIEW(webServer->getUrl()));

			const auto maxPlayers = *core->getConfig().getInt("max_players");
			if (httpThreads < maxPlayers / 2)
			{
				core->logLn(LogLevel::Warning, "HTTP threads count value (%d) is much lower than MAX_PLAYERS (%d). Players may encounter slow download times.", httpThreads, maxPlayers);
			}
		}
		else
		{
			core->logLn(LogLevel::Error, "Failed to start web server");
		}
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Don't reset pool here - SAMP client can't handle it and will crash.
		// Why can't we have nice things? :(

		// Someone complained about this. Let's parse artconfig again.
		loadArtConfig();
	}

	StringView getWebUrl()
	{
		return cdn.empty() ? webServer->getUrl() : cdn;
	}

	bool addCustomModel(ModelType type, int32_t id, int32_t baseId, StringView dffName, StringView txdName, int32_t virtualWorld = -1, uint8_t timeOn = 0, uint8_t timeOff = 0) override
	{
		if (!enabled)
		{
			return false;
		}

		if (type == ModelType::Skin && !(id >= MIN_CUSTOM_SKIN_ID && id <= MAX_CUSTOM_SKIN_ID))
			return false;
		else if (type == ModelType::Object && !(id >= MIN_CUSTOM_OBJECT_ID && id <= MAX_CUSTOM_OBJECT_ID))
			return false;
		else if (baseModels.find(id) != baseModels.end())
		{
			// Sadly this error will be displayed on gmx. Dunno what do about it.
			core->logLn(LogLevel::Error, "[artwork:error] Model %d is already in use", id);
			return false;
		}

		ModelFile dff(modelsPath, dffName);
		ModelFile txd(modelsPath, txdName);

		if (!dff.size)
		{
			core->logLn(LogLevel::Error, "[artwork:error] Cannot add custom model. %.*s doesn't exist", PRINT_VIEW(dffName));
			return false;
		}

		if (!txd.size)
		{
			core->logLn(LogLevel::Error, "[artwork:error] Cannot add custom model. %.*s doesn't exist", PRINT_VIEW(txdName));
			return false;
		}

		if (showCRCLogs)
		{
			core->logLn(LogLevel::Message, "[artwork:crc] %.*s CRC = 0x%X", PRINT_VIEW(txdName), txd.checksum);
			core->logLn(LogLevel::Message, "[artwork:crc] %.*s CRC = 0x%X", PRINT_VIEW(dffName), dff.checksum);
		}

		auto model = storage.emplace_back(new ModelInfo(type, id, baseId, dff, txd, virtualWorld, timeOn, timeOff));

		if (!model)
			return false;

		auto size = storage.size() - 1;
		NetCode::RPC::ModelRequest modelInfo(size - 1, size);
		model->write(modelInfo);

		for (IPlayer* player : players->entries())
		{
			if (player->getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL)
				continue;

			PacketHelper::send(modelInfo, *player);
		}

		baseModels.emplace(id, baseId);
		checksums.emplace(dff.checksum, std::make_pair(ModelDownloadType::DFF, model));
		checksums.emplace(txd.checksum, std::make_pair(ModelDownloadType::TXD, model));

		// Start web server if needed.
		startWebServer();
		return true;
	}

	bool getBaseModel(uint32_t& baseModelIdOrInput, uint32_t& customModel) override
	{
		// Check if model is default one (base).
		// If so, there's no custom model to be returned.
		if (baseModelIdOrInput >= 0 && baseModelIdOrInput < MIN_CUSTOM_SKIN_ID)
			return false;

		// Check if input is valid custom model.
		// If not treat is as base model.
		auto itr = baseModels.find(baseModelIdOrInput);
		if (itr == baseModels.end())
		{
			return false;
		}

		// Input is a custom model.
		customModel = baseModelIdOrInput;
		baseModelIdOrInput = itr->second;
		return true;
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
		for (auto i = 0; i != modelsCount; ++i)
		{
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
		player.addExtension(new PlayerCustomModelsData(player), true);

		if (player.getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL || !webServer)
		{
			return;
		}

		webServer->allowIPAddress(player.getNetworkData().networkID.address.v4);
	}

	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override
	{
		if (player.getClientVersion() != ClientVersion::ClientVersion_SAMP_03DL || !webServer)
		{
			return;
		}

		webServer->removeIPAddress(player.getNetworkData().networkID.address.v4);
	}

	bool isValidCustomModel(int32_t modelId) const override
	{
		return baseModels.find(modelId) != baseModels.end();
	}

	bool getCustomModelPath(int32_t modelId, StringView& dffPath, StringView& txdPath) const override
	{
		auto model = std::find_if(storage.begin(), storage.end(), [&](const auto& model)
			{
				return model->getId() == modelId;
			});

		if (model != storage.end())
		{
			dffPath = (*model)->getDFF().name;
			txdPath = (*model)->getTXD().name;
			return true;
		}
		return false;
	}
};

COMPONENT_ENTRY_POINT()
{
	return new CustomModelsComponent();
}
