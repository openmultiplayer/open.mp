/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "player_pool.hpp"
#include "util.hpp"
#include <Impl/network_impl.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Console/console.hpp>
#include <Server/Components/Unicode/unicode.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/LegacyConfig/legacyconfig.hpp>
#include <Server/Components/CustomModels/custommodels.hpp>
#include <cstdarg>
#include <cxxopts.hpp>
#include <events.hpp>
#include <ghc/filesystem.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <pool.hpp>
#include <sstream>
#include <thread>
#include <variant>
#include <utils.hpp>

#ifdef OMP_VERSION_HASH
#define OMP_VERSION_HASH_STR STRINGIFY(OMP_VERSION_HASH)
#else
#define OMP_VERSION_HASH_STR "0000000000000000000000000000000000000000"
#endif

using namespace Impl;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#pragma clang diagnostic pop

#include <openssl/sha.h>

typedef std::variant<int, String, float, DynamicArray<String>, bool> ConfigStorage;

static const std::map<String, ConfigStorage> Defaults {
	{ "announce", true },
	{ "chat_input_filter", true },
	{ "enable_query", true },
	{ "language", String("") },
	{ "max_bots", 0 },
	{ "max_players", 50 },
	{ "name", String("open.mp server") },
	{ "password", String("") },
	{ "sleep", 5.0f },
	{ "use_dyn_ticks", true },
	{ "website", String("open.mp") },
	// game
	{ "game.allow_interior_weapons", true },
	{ "game.chat_radius", 200.0f },
	{ "game.death_drop_amount", 0 },
	{ "game.gravity", 0.008f },
	{ "game.map", String("") },
	{ "game.mode", String("") },
	{ "game.nametag_draw_radius", 70.0f },
	{ "game.player_marker_mode", PlayerMarkerMode_Global },
	{ "game.player_marker_draw_radius", 250.f },
	{ "game.time", 12 },
	{ "game.use_chat_radius", false },
	{ "game.use_entry_exit_markers", true },
	{ "game.use_instagib", false },
	{ "game.use_nametag_los", true },
	{ "game.use_nametags", true },
	{ "game.use_player_marker_draw_radius", false },
	{ "game.use_player_ped_anims", false },
	{ "game.use_stunt_bonuses", true },
	{ "game.use_manual_engine_and_lights", false },
	{ "game.use_vehicle_friendly_fire", false },
	{ "game.use_zone_names", false },
	{ "game.vehicle_respawn_time", 10000 },
	{ "game.weather", 10 },
	{ "game.validate_animations", true },
	{ "game.use_all_animations", true },
	{ "game.lag_compensation_mode", LagCompMode_Enabled },
	{ "game.group_player_objects", false },
	// logging
	{ "logging.enable", true },
	{ "logging.file", String("log.txt") },
	{ "logging.log_chat", true },
	{ "logging.log_connection_messages", true },
	{ "logging.log_cookies", false },
	{ "logging.log_deaths", true },
	{ "logging.log_queries", false },
	{ "logging.log_sqlite", false },
	{ "logging.log_sqlite_queries", false },
	{ "logging.timestamp_format", String("[%Y-%m-%dT%H:%M:%S%z]") },
	{ "logging.use_timestamp", true },
	{ "logging.use_prefix", true },
	// network
	{ "network.bind", String("") },
	{ "network.public_addr", String("") }, // Used by webserver
	{ "network.port", 7777 },
	{ "network.acks_limit", 3000 },
	{ "network.aiming_sync_rate", 30 },
	{ "network.cookie_reseed_time", 300000 },
	{ "network.in_vehicle_sync_rate", 30 },
	{ "network.limits_ban_time", 60000 },
	{ "network.message_hole_limit", 3000 },
	{ "network.messages_limit", 500 },
	{ "network.minimum_connection_time", 0 },
	{ "network.mtu", 576 },
	{ "network.multiplier", 10 },
	{ "network.on_foot_sync_rate", 30 },
	{ "network.player_marker_sync_rate", 2500 },
	{ "network.player_timeout", 10000 },
	{ "network.stream_radius", 200.f },
	{ "network.stream_rate", 1000 },
	{ "network.time_sync_rate", 30000 },
	{ "network.use_lan_mode", false },
	{ "network.allow_037_clients", true },
	{ "network.grace_period", 5000 },
	// rcon
	{ "rcon.allow_teleport", false },
	{ "rcon.enable", false },
	{ "rcon.password", String("") }, // Set default to empty instead of changeme, so server starts with disabled rcon without config file
	// banners
	{ "banners.light", String("") },
	{ "banners.dark", String("") },
	{ "logo", String("") },
	// discord
	{ "discord.invite", String("") },
};

// Provide automatic Defaults → JSON conversion in Config
namespace nlohmann
{
template <typename... Args>
struct adl_serializer<std::variant<Args...>>
{
	static void to_json(ordered_json& j, std::variant<Args...> const& v)
	{
		std::visit([&](auto&& value)
			{
				j = std::forward<decltype(value)>(value);
			},
			v);
	}
};
}

struct VarlistEnumCallback : OptionEnumeratorCallback
{
	IConsoleComponent& console;
	IConfig& config;
	const ConsoleCommandSenderData& sender;

	VarlistEnumCallback(IConsoleComponent& console, IConfig& config, const ConsoleCommandSenderData& sender)
		: console(console)
		, config(config)
		, sender(sender)
	{
	}

	bool proc(StringView n, ConfigOptionType type) override
	{
		String name(n);

		switch (type)
		{
		case ConfigOptionType_Int:
			console.sendMessage(sender, name + " = " + std::to_string(*config.getInt(name)) + " (int)");
			break;
		case ConfigOptionType_Float:
			console.sendMessage(sender, name + " = " + std::to_string(*config.getFloat(name)) + " (float)");
			break;
		case ConfigOptionType_Bool:
			console.sendMessage(sender, name + " = " + std::to_string(*config.getBool(name)) + " (bool)");
			break;
		case ConfigOptionType_String:
			console.sendMessage(sender, name + " = \"" + String(config.getString(name)) + "\" (string)");
			break;
		case ConfigOptionType_Strings:
		{
			size_t count = config.getStringsCount(name);

			if (count)
			{
				DynamicArray<StringView> output(count);
				config.getStrings(name, Span<StringView>(output.data(), output.size()));

				String strings_list = "";

				for (auto& string : output)
				{
					strings_list += String(string) + ' ';
				}

				if (strings_list.back() == ' ')
				{
					strings_list.pop_back();
				}

				console.sendMessage(sender, name + " = \"" + strings_list + "\" (strings)");
			}
			else
			{
				console.sendMessage(sender, name + " = \"\" (strings)");
			}
		}
		break;
		default:
			break;
		}
		return true;
	}
};

class ComponentList : public IComponentList
{
public:
	using IComponentList::queryComponent;

	IComponent* queryComponent(UID id) override
	{
		auto it = components.find(id);
		return it == components.end() ? nullptr : it->second;
	}

	void configure(ICore& core, IEarlyConfig& config, bool defaults)
	{
		std::for_each(components.begin(), components.end(),
			[&core, &config, defaults](const robin_hood::pair<UID, IComponent*>& pair)
			{
				pair.second->provideConfiguration(core, config, defaults);
			});
	}

	void load(ICore* core)
	{
		std::for_each(components.begin(), components.end(),
			[core](const robin_hood::pair<UID, IComponent*>& pair)
			{
				pair.second->onLoad(core);
			});
	}

	void init()
	{
		std::for_each(components.begin(), components.end(),
			[this](const robin_hood::pair<UID, IComponent*>& pair)
			{
				pair.second->onInit(this);
			});
	}

	void reset()
	{
		std::for_each(components.begin(), components.end(),
			[](const robin_hood::pair<UID, IComponent*>& pair)
			{
				pair.second->reset();
			});
	}

	void ready()
	{
		std::for_each(components.begin(), components.end(),
			[](const robin_hood::pair<UID, IComponent*>& pair)
			{
				pair.second->onReady();
			});
	}

	void free()
	{
		for (auto it = components.begin(); it != components.end();)
		{
			std::for_each(components.begin(), components.end(),
				[it](const robin_hood::pair<UID, IComponent*>& pair)
				{
					pair.second->onFree(it->second);
				});
			it->second->free();
			it = components.erase(it);
		}
	}

	auto add(IComponent* component)
	{
		return components.try_emplace(component->getUID(), component);
	}

	size_t size() const
	{
		return components.size();
	}

private:
	FlatHashMap<UID, IComponent*> components;
};

static constexpr const char* TimeFormat = "%Y-%m-%dT%H:%M:%S%z";

class Config final : public IEarlyConfig
{
private:
	static constexpr const char* BansFileName = "bans.json";

	IUnicodeComponent* unicode = nullptr;
	ICore& core;
	String ConfigFileName = "config.json";

	std::map<String, ConfigStorage> defaults;

	void processNode(const nlohmann::json::object_t& node, String ns = "")
	{
		for (const auto& kv : node)
		{
			String key = ns + kv.first;
			const nlohmann::json& v = kv.second;
			if (v.is_number_integer())
			{
				processed[key].emplace<int>(v.get<int>());
			}
			else if (v.is_boolean())
			{
				processed[key].emplace<bool>(v.get<bool>());
			}
			else if (v.is_number_float())
			{
				processed[key].emplace<float>(v.get<float>());
			}
			else if (v.is_string())
			{
				processed[key].emplace<String>(v.get<String>());
			}
			else if (v.is_array())
			{
				auto& vec = processed[key].emplace<DynamicArray<String>>();
				const auto& arr = v.get<nlohmann::json::array_t>();
				for (const auto& arrVal : arr)
				{
					if (arrVal.is_string())
					{
						vec.emplace_back(arrVal.get<String>());
					}
				}
			}
			else if (v.is_object())
			{
				processNode(v.get<nlohmann::json::object_t>(), key + '.');
			}
		}
	}

public:
	Config(ICore& core, bool defaultsOnly = false, const cxxopts::ParseResult* cmd = nullptr)
		: core(core)
	{
		if (!defaultsOnly)
		{
			{
				if (cmd && cmd->count("config-path"))
				{
					ConfigFileName = cmd->operator[]("config-path").as<String>();
				}
				std::ifstream ifs(ConfigFileName);
				if (ifs.good())
				{
					nlohmann::json props;
					try
					{
						props = nlohmann::json::parse(ifs, nullptr, true /* allow_exceptions */, true /* ignore_comments */);
					}
					catch (nlohmann::json::exception const& e)
					{
						std::cout << "Error while parsing config file: " << e.what() << '\n';
						// We can't use core's logging system since config is not fully yet
						// To decide whether logging is enabled or not
						// core.printLn("Error parsing config file:");
						// core.printLn("%s", e.what());
					}
					catch (std::ios_base::failure const& e)
					{
						std::cout << "Error while parsing config file: " << e.what() << '\n';
						// We can't use core's logging system since config is not fully yet
						// To decide whether logging is enabled or not
						// core.printLn("Error parsing config file:");
						// core.printLn("%s", e.what());
					}
					// Is a directory?
					if (props.is_null() || props.is_discarded() || !props.is_object())
					{
						processed = Defaults;
					}
					else
					{
						const auto& root = props.get<nlohmann::json::object_t>();
						processNode(root);

						// Fill any values missing in config with defaults.
						// Fill default value if invalid type is provided.
						for (const auto& kv : Defaults)
						{
							auto itr = processed.find(kv.first);
							if (itr != processed.end())
							{
								if (itr->second.index() != kv.second.index())
								{
									itr->second = kv.second;
								}
								continue;
							}
							processed.emplace(kv.first, kv.second);
						}
					}
				}
				else
				{
					processed = Defaults;
				}
			}

			loadBans();
		}
		else
		{
			processed = Defaults;
		}
		defaults = processed;
	}

	void init(IComponentList& components)
	{
		unicode = components.queryComponent<IUnicodeComponent>();
	}

	const StringView getString(StringView key) const override
	{
		const ConfigStorage* res = nullptr;
		if (!getFromKey(key, ConfigOptionType_String, res))
		{
			return StringView();
		}
		return StringView(std::get<String>(*res));
	}

	int* getInt(StringView key) override
	{
		ConfigStorage* res = nullptr;
		if (!getFromKey(key, ConfigOptionType_Int, res))
		{
			return 0;
		}
		return &std::get<int>(*res);
	}

	bool* getBool(StringView key) override
	{
		ConfigStorage* res = nullptr;
		if (!getFromKey(key, ConfigOptionType_Bool, res))
		{
			return 0;
		}
		return &std::get<bool>(*res);
	}

	float* getFloat(StringView key) override
	{
		ConfigStorage* res = nullptr;
		if (!getFromKey(key, ConfigOptionType_Float, res))
		{
			return 0;
		}
		return &std::get<float>(*res);
	}

	size_t getStringsCount(StringView key) const override
	{
		const ConfigStorage* res = nullptr;
		if (!getFromKey(key, ConfigOptionType_Strings, res))
		{
			return 0;
		}
		return std::get<DynamicArray<String>>(*res).size();
	}

	size_t getStrings(StringView key, Span<StringView> output) const override
	{
		if (!output.size())
		{
			return 0;
		}

		const ConfigStorage* res = nullptr;
		if (!getFromKey(key, ConfigOptionType_Strings, res))
		{
			return 0;
		}

		const auto& strings = std::get<DynamicArray<String>>(*res);
		const size_t size = std::min(output.size(), strings.size());
		for (size_t i = 0; i < size; ++i)
		{
			output[i] = strings[i];
		}
		return size;
	}

	const DynamicArray<String>* getStrings(StringView key) const
	{
		const ConfigStorage* res = nullptr;
		if (!getFromKey(key, ConfigOptionType_Strings, res))
		{
			return nullptr;
		}
		return &std::get<DynamicArray<String>>(*res);
	}

	ConfigOptionType getType(StringView key) const override
	{
		auto it = processed.find(String(key));
		if (it == processed.end())
		{
			return ConfigOptionType_None;
		}
		return ConfigOptionType(it->second.index());
	}

	void setString(StringView key, StringView value) override
	{
		processed[String(key)] = String(value);
	}

	void setInt(StringView key, int value) override
	{
		processed[String(key)] = value;
	}

	void setBool(StringView key, bool value) override
	{
		processed[String(key)] = value;
	}

	void setFloat(StringView key, float value) override
	{
		processed[String(key)] = value;
	}

	void setStrings(StringView key, Span<const StringView> value) override
	{
		DynamicArray<String> newStrings;
		for (const StringView v : value)
		{
			newStrings.emplace_back(String(v));
		}
		processed[String(key)].emplace<DynamicArray<String>>(std::move(newStrings));
	}

	void addBan(const BanEntry& entry) override
	{
		bans.emplace_back(entry);
	}

	void removeBan(const BanEntry& entry) override
	{
		auto ban_itr = std::find_if(bans.begin(), bans.end(), [&](const BanEntry& ban)
			{
				return ban.address == entry.address;
			});

		if (ban_itr != bans.end())
		{
			bans.erase(ban_itr);
			writeBans();
		}
	}

	void removeBan(size_t index) override
	{
		bans.erase(bans.begin() + index);
	}

	void reloadBans() override
	{
		for (INetwork* network : core.getNetworks())
		{
			for (BanEntry ban : bans)
			{
				network->unban(ban);
			}
		}

		bans.clear();
		loadBans();
	}

	void writeBans() override
	{
		optimiseBans();

		nlohmann::json top = nlohmann::json::array();
		for (const BanEntry& entry : bans)
		{
			nlohmann::json obj;
			OptimisedString addressUTF8 = unicode ? unicode->toUTF8(entry.address) : OptimisedString(entry.address);
			OptimisedString nameUTF8 = unicode ? unicode->toUTF8(entry.name) : OptimisedString(entry.name);
			OptimisedString reasonUTF8 = unicode ? unicode->toUTF8(entry.reason) : OptimisedString(entry.reason);
			obj["address"] = StringView(addressUTF8);
			obj["player"] = StringView(nameUTF8);
			obj["reason"] = StringView(reasonUTF8);
			char iso8601[28] = { 0 };
			std::time_t now = WorldTime::to_time_t(entry.time);
			std::strftime(iso8601, sizeof(iso8601), TimeFormat, std::localtime(&now));
			obj["time"] = iso8601;
			top.push_back(obj);
		}
		std::ofstream file(BansFileName);
		if (file.good())
		{
			file << top.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore);
		}
	}

	void clearBans() override
	{
		bans.clear();
		writeBans();
	}

	bool isBanned(const BanEntry& entry) const override
	{
		return std::any_of(bans.begin(), bans.end(), [&](const BanEntry& ban)
			{
				return entry.address == ban.address;
			});
	}

	size_t getBansCount() const override
	{
		return bans.size();
	}

	const BanEntry& getBan(size_t index) const override
	{
		return bans[index];
	}

	void optimiseBans()
	{
		std::sort(bans.begin(), bans.end());
		bans.erase(std::unique(bans.begin(), bans.end()), bans.end());
	}

	bool writeDefault(ComponentList& components)
	{
		core.printLn("Generating %s...", ConfigFileName.c_str());

		// Creates default config.json file if it doesn't exist
		// Returns true if a config file was written, false otherwise
		std::ifstream ifs(ConfigFileName);
		if (ifs.good())
		{
			return false;
		}

		std::ofstream ofs(ConfigFileName);
		nlohmann::ordered_json json;

		Config config(core, true /* defaultsOnly */);
		components.configure(core, config, true /* defaults */);
		config.setString("rcon.password", "changeme");

		if (ofs.good())
		{
			for (const auto& kv : config.options())
			{
				nlohmann::ordered_json* sub = &json;
				size_t cur = String::npos, prev = 0;
				// Process hierarchy
				while ((cur = kv.first.find('.', prev)) != String::npos)
				{
					String substr = kv.first.substr(prev, cur - prev);
					sub = &(*sub)[substr];
					prev = cur + sizeof('.');
				}
				// Set the leaf's value
				(*sub)[kv.first.substr(prev, cur - prev)] = kv.second;
			}
			ofs << json.dump(4) << std::endl;
		}
		return true;
	}

	bool writeCurrent()
	{
		core.printLn("Generating %s...", ConfigFileName.c_str());

		// Creates default config.json file if it doesn't exist
		// Returns true if a config file was written, false otherwise
		std::ifstream ifs(ConfigFileName);
		if (ifs.good())
		{
			return false;
		}

		std::ofstream ofs(ConfigFileName);
		nlohmann::ordered_json json;

		if (ofs.good())
		{
			for (const auto& kv : options())
			{
				nlohmann::ordered_json* sub = &json;
				size_t cur = String::npos, prev = 0;
				// Process hierarchy
				while ((cur = kv.first.find('.', prev)) != String::npos)
				{
					String substr = kv.first.substr(prev, cur - prev);
					sub = &(*sub)[substr];
					prev = cur + sizeof('.');
				}
				// Set the leaf's value
				(*sub)[kv.first.substr(prev, cur - prev)] = kv.second;
			}
			ofs << json.dump(4) << std::endl;
		}
		return true;
	}

	void addAlias(StringView alias, StringView key, bool deprecated = false) override
	{
		if (key != alias)
		{
			aliases[String(alias)] = std::make_pair(deprecated, String(key));
		}
	}

	Pair<bool, StringView> getNameFromAlias(StringView alias) const override
	{
		auto it = aliases.find(String(alias));
		if (it == aliases.end())
		{
			return std::make_pair(false, StringView());
		}
		return std::make_pair(it->second.first, StringView(it->second.second));
	}

	void enumOptions(OptionEnumeratorCallback& callback) const override
	{
		for (auto& kv : processed)
		{
			if (!callback.proc(StringView(kv.first), ConfigOptionType(kv.second.index())))
			{
				break;
			}
		}
	}

private:
	void loadBans()
	{
		std::ifstream ifs(BansFileName);
		if (ifs.good())
		{
			nlohmann::json props = nlohmann::json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
			if (!props.is_null() && !props.is_discarded() && props.is_array())
			{
				const auto& arr = props.get<nlohmann::json::array_t>();
				for (const auto& arrVal : arr)
				{
					std::tm time = {};
					std::istringstream(arrVal["time"].get<String>()) >> std::get_time(&time, TimeFormat);
					time_t t =
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
						_mkgmtime(&time);
#else
						timegm(&time);
#endif

					bans.emplace_back(BanEntry(arrVal["address"].get<String>(), arrVal["player"].get<String>(), arrVal["reason"].get<String>(), WorldTime::from_time_t(t)));
				}
			}
		}
	}

	bool getFromKey(StringView input, int index, const ConfigStorage*& output) const
	{
		auto it = processed.find(String(input));
		if (it == processed.end())
		{
			return false;
		}
		if (it->second.index() != index)
		{
			return false;
		}

		output = &it->second;
		return true;
	}

	bool getFromKey(StringView input, int index, ConfigStorage*& output)
	{
		auto it = processed.find(String(input));
		if (it == processed.end())
		{
			return false;
		}
		if (it->second.index() != index)
		{
			return false;
		}

		output = &it->second;
		return true;
	}

	const std::map<String, ConfigStorage>& options() const
	{
		return processed;
	}

	DynamicArray<BanEntry> bans;
	std::map<String, ConfigStorage> processed;
	FlatHashMap<String, Pair<bool, String>> aliases;
};

class HTTPAsyncIO
{
public:
	HTTPAsyncIO(HTTPResponseHandler* handler, HTTPRequestType type, StringView url, StringView data, bool force_v4 = false, StringView bindAddr = "")
		: handler(handler)
		, type(type)
		, url(url)
		, data(data)
		, force_v4(force_v4)
		, bindAddr(bindAddr)
		, finished(false)
		, response(0)
	{
		thread = std::thread(&threadProc, this);
	}

	~HTTPAsyncIO()
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	bool tryExec()
	{
		if (finished)
		{
			handler->onHTTPResponse(response, body);
			return true;
		}
		else
		{
			return false;
		}
	}

private:
	static void threadProc(HTTPAsyncIO* params)
	{
		constexpr StringView http = "http://";
		constexpr StringView https = "https://";

		HTTPRequestType type = params->type;
		StringView url = params->url;
		StringView data = params->data;

		// Deconstruct because a certain someone decided it would be a good idea to have http:// be optional
		StringView urlNoPrefix = url;
		bool secure = false;
		int idx;
		if ((idx = url.find(http)) == 0)
		{
			urlNoPrefix = url.substr(http.size());
			secure = false;
		}
		else if ((idx = url.find(https)) == 0)
		{
			urlNoPrefix = url.substr(https.size());
			secure = true;
		}

		// Deconstruct further
		StringView domain = urlNoPrefix;
		StringView path = "/";
		if ((idx = urlNoPrefix.find_first_of('/')) != StringView::npos)
		{
			domain = urlNoPrefix.substr(0, idx);
			path = urlNoPrefix.substr(idx);
		}

		// Reconstruct
		String domainStr = String(secure ? https : http) + String(domain);

		// Set up request
		httplib::Client request(domainStr.c_str());
		request.set_default_headers({ { "User-Agent", "open.mp server" } });
		request.enable_server_certificate_verification(true);
		request.set_follow_location(true);
		request.set_connection_timeout(Seconds(5));
		request.set_read_timeout(Seconds(60));
		request.set_write_timeout(Seconds(5));
		request.set_keep_alive(true);

		if (params->force_v4)
			request.set_address_family(AF_INET);

		if (!params->bindAddr.empty())
			request.set_interface(params->bindAddr);

		// Run request
		httplib::Result res(nullptr, httplib::Error::Canceled);
		switch (type)
		{
		case HTTPRequestType_Get:
			res = request.Get(path.data());
			break;
		case HTTPRequestType_Post:
			res = request.Post(path.data(), String(data), "application/x-www-form-urlencoded");
			break;
		case HTTPRequestType_Head:
			res = request.Head(path.data());
			break;
		}

		if (res)
		{
			params->body = res.value().body;
			params->response = res.value().status;
		}
		else
		{
			params->response = int(res.error());
			if (params->response < 100)
			{
				params->body = httplib::detail::internal_error_to_string(res.error());
			}
		}

		params->finished.store(true);
	}

	std::thread thread;
	HTTPResponseHandler* handler;
	HTTPRequestType type;
	String url;
	String data;

	bool force_v4;
	String bindAddr;

	std::atomic_bool finished;
	int response;
	String body;
};

class Core final : public ICore, public PlayerConnectEventHandler, public ConsoleEventHandler
{
private:
	DefaultEventDispatcher<CoreEventHandler> eventDispatcher;
	PlayerPool players;
	Microseconds sleepTimer;
	Microseconds sleepDuration;
	bool _useDynTicks;
	FlatPtrHashSet<INetwork> networks;
	ComponentList components;
	Config config;
	IConsoleComponent* console;
	ICustomModelsComponent* models;
	FILE* logFile;
	std::atomic_bool run_;
	unsigned ticksPerSecond;
	unsigned ticksThisSecond;
	TimePoint ticksPerSecondLastUpdate;
	std::set<HTTPAsyncIO*> httpFutures;

	bool* EnableZoneNames;
	bool* UsePlayerPedAnims;
	bool* AllowInteriorWeapons;
	bool* UseLimitGlobalChatRadius;
	float* LimitGlobalChatRadius;
	bool* EnableStuntBonus;
	float* SetNameTagDrawDistance;
	bool* EnableInteriorEnterExits;
	bool* EnableNameTagLOS;
	bool* ManualVehicleEngineAndLights;
	bool* ShowNameTags;
	int* ShowPlayerMarkers;
	int* SetWorldTime;
	int* SetWeather;
	float* SetGravity;
	bool* LanMode;
	int* SetDeathDropAmount;
	bool* Instagib;
	int* OnFootRate;
	int* InCarRate;
	int* WeaponRate;
	int* Multiplier;
	int* LagCompensation;
	bool* EnableVehicleFriendlyFire;
	bool reloading_ = false;

	bool EnableLogTimestamp;
	bool EnableLogPrefix;
	String LogTimestampFormat;
	String LogFileName;

	void addComponent(IComponent* component)
	{
		auto res = components.add(component);
		if (!res.second)
		{
			printLn("Tried to add plug-ins %.*s and %.*s with conflicting UID %16llx", PRINT_VIEW(component->componentName()), PRINT_VIEW(res.first->second->componentName()), component->getUID());
		}
		if (component->componentType() == ComponentType::Network)
		{
			networks.insert(static_cast<INetworkComponent*>(component)->getNetwork());
		}
	}

	IComponent* loadComponent(const ghc::filesystem::path& path, bool highPriority = false)
	{
		printLn("Loading component %s", path.filename().u8string().c_str());
		auto componentLib = highPriority ? LIBRARY_OPEN_GLOBAL(path.u8string().c_str()) : LIBRARY_OPEN(path.u8string().c_str());
		if (componentLib == nullptr)
		{
			printLn("\tFailed to load component: %s.", utils::GetLastErrorAsString().c_str());
			return nullptr;
		}
		ComponentEntryPoint_t OnComponentLoad = reinterpret_cast<ComponentEntryPoint_t>(LIBRARY_GET_ADDR(componentLib, "ComponentEntryPoint"));
		if (OnComponentLoad == nullptr)
		{
			void* isSAMPPlugin = reinterpret_cast<void*>(LIBRARY_GET_ADDR(componentLib, "Supports"));
			printLn(
				"\tFailed to load component: %s.",
				isSAMPPlugin
					? "it is a SA-MP plugin, put it in plugins/ folder"
					: "it is neither an open.mp component nor a SA-MP plugin");
			LIBRARY_FREE(componentLib);
			return nullptr;
		}
		IComponent* component = OnComponentLoad();
		if (component == nullptr)
		{
			printLn("\tFailed to load component.");
			LIBRARY_FREE(componentLib);
			return nullptr;
		}
		int supports = component->supportedVersion();
		if (supports != OMP_VERSION_MAJOR)
		{
			printLn("\tFailed to load component: Built for open.mp version %d, now on %d.", supports, OMP_VERSION_MAJOR);
			LIBRARY_FREE(componentLib);
			return nullptr;
		}
		SemanticVersion ver = component->componentVersion();
		printLn(
			"\tSuccessfully loaded component %.*s (%u.%u.%u.%u) with UID %016llx",
			PRINT_VIEW(component->componentName()),
			ver.major,
			ver.minor,
			ver.patch,
			ver.prerel,
			component->getUID());
		return component;
	}

	void loadComponents(const ghc::filesystem::path& path)
	{
		ghc::filesystem::create_directory(path);

		auto componentsCfg = config.getStrings("components");
		auto excludeCfg = config.getStrings("exclude");

		Impl::DynamicArray<ghc::filesystem::path> highPriorityComponents;
		Impl::DynamicArray<ghc::filesystem::path> normalComponents;

		const auto shouldLoad = [&](const ghc::filesystem::path& p)
		{
			if (excludeCfg && !excludeCfg->empty())
			{
				ghc::filesystem::path rel = ghc::filesystem::relative(p, path);
				rel.replace_extension();
				// Is this in the "don't load" list?
				const auto isExcluded = [rel = std::move(rel)](const String& exclude)
				{
					return ghc::filesystem::path(exclude) == rel;
				};
				if (std::find_if(excludeCfg->begin(), excludeCfg->end(), isExcluded)
					!= excludeCfg->end())
				{
					return false;
				}
			}
			return true;
		};

		if (!componentsCfg || componentsCfg->empty())
		{
			for (auto& de : ghc::filesystem::recursive_directory_iterator(path))
			{
				ghc::filesystem::path p = de.path();
				if (p.filename().string().at(0) == '$')
				{
					highPriorityComponents.push_back(p);
				}
				else
				{
					normalComponents.push_back(p);
				}
			}

			for (auto& p : highPriorityComponents)
			{
				if (p.extension() == LIBRARY_EXT)
				{
					if (!shouldLoad(p))
					{
						continue;
					}

					IComponent* component = loadComponent(p, true);
					if (component)
					{
						addComponent(component);
					}
				}
			}

			for (auto& p : normalComponents)
			{
				if (p.extension() == LIBRARY_EXT)
				{
					if (!shouldLoad(p))
					{
						continue;
					}

					IComponent* component = loadComponent(p);
					if (component)
					{
						addComponent(component);
					}
				}
			}
		}
		else
		{
			for (const StringView component : *componentsCfg)
			{
				auto file = ghc::filesystem::path(path) / component.data();
				if (file.has_extension())
				{
					file.replace_extension("");
				}

				if (file.filename().string().at(0) == '$')
				{
					highPriorityComponents.push_back(file);
				}
				else
				{
					normalComponents.push_back(file);
				}
			}

			for (auto& p : highPriorityComponents)
			{
				if (!shouldLoad(p))
				{
					continue;
				}

				// Now load it.
				p.replace_extension(LIBRARY_EXT);
				if (ghc::filesystem::exists(p))
				{
					IComponent* component = loadComponent(p, true);
					if (component)
					{
						addComponent(component);
					}
				}
				else
				{
					printLn("Loading component %s", p.filename().u8string().c_str());
					printLn("\tCould not find component");
				}
			}

			for (auto& p : normalComponents)
			{
				if (!shouldLoad(p))
				{
					continue;
				}

				// Now load it.
				p.replace_extension(LIBRARY_EXT);
				if (ghc::filesystem::exists(p))
				{
					IComponent* component = loadComponent(p);
					if (component)
					{
						addComponent(component);
					}
				}
				else
				{
					printLn("Loading component %s", p.filename().u8string().c_str());
					printLn("\tCould not find component");
				}
			}
		}

		std::string absPath = ghc::filesystem::canonical(path).string();
		printLnU8("Loaded %i component(s) from %.*s", components.size(), PRINT_VIEW(StringView(absPath)));
	}

	void playerInit(IPlayer& player)
	{
		players.playerConnectDispatcher.dispatch(&PlayerConnectEventHandler::onPlayerClientInit, player);

		NetCode::RPC::PlayerInit playerInitRPC;
		playerInitRPC.EnableZoneNames = *EnableZoneNames;
		playerInitRPC.UsePlayerPedAnims = *UsePlayerPedAnims;
		playerInitRPC.AllowInteriorWeapons = *AllowInteriorWeapons;
		playerInitRPC.UseLimitGlobalChatRadius = *UseLimitGlobalChatRadius;
		playerInitRPC.LimitGlobalChatRadius = *LimitGlobalChatRadius;
		playerInitRPC.EnableStuntBonus = *EnableStuntBonus;
		playerInitRPC.SetNameTagDrawDistance = *SetNameTagDrawDistance;
		playerInitRPC.DisableInteriorEnterExits = !*EnableInteriorEnterExits;
		playerInitRPC.DisableNameTagLOS = !*EnableNameTagLOS;
		playerInitRPC.ManualVehicleEngineAndLights = *ManualVehicleEngineAndLights;
		playerInitRPC.ShowNameTags = *ShowNameTags;
		playerInitRPC.ShowPlayerMarkers = *ShowPlayerMarkers;

		// Get player time & weather instead of global ones because they can be changed during OnPlayerConnect.
		playerInitRPC.SetWorldTime = player.getTime().first.count();
		playerInitRPC.SetWeather = player.getWeather();

		playerInitRPC.SetGravity = *SetGravity;
		playerInitRPC.LanMode = *LanMode;
		playerInitRPC.SetDeathDropAmount = *SetDeathDropAmount;
		playerInitRPC.Instagib = *Instagib;
		playerInitRPC.OnFootRate = *OnFootRate;
		playerInitRPC.InCarRate = *InCarRate;
		playerInitRPC.WeaponRate = *WeaponRate;
		playerInitRPC.Multiplier = *Multiplier;
		playerInitRPC.LagCompensation = *LagCompensation;
		playerInitRPC.ServerName = config.getString("name");
		IClassesComponent* classes = components.queryComponent<IClassesComponent>();
		playerInitRPC.SetSpawnInfoCount = classes ? classes->count() : 0;
		playerInitRPC.PlayerID = player.getID();
		IVehiclesComponent* vehicles = components.queryComponent<IVehiclesComponent>();
		static const StaticArray<uint8_t, 212> emptyModels { 0 };
		playerInitRPC.VehicleModels = vehicles ? vehicles->models() : emptyModels;
		playerInitRPC.EnableVehicleFriendlyFire = *EnableVehicleFriendlyFire;
		PacketHelper::send(playerInitRPC, player);

		// Send player his color. Fixes SetPlayerColor called during OnPlayerConnect.
		NetCode::RPC::SetPlayerColor RPC;
		RPC.PlayerID = player.getID();
		RPC.Col = player.getColour();
		PacketHelper::send(RPC, player);
	}

	bool setArrayFromString(StringView key, StringView value)
	{
		size_t len = value.length();
		char const* data = value.data();
		DynamicArray<StringView> view {};
		// Clear the old values.
		config.setStrings(key, view);

		for (size_t i = 0, start = 0, state = 0; i != len; ++i)
		{
			switch (state)
			{
			case 0:
				// Array not started yet.
				if (data[i] == '[')
				{
					state = 1;
				}
				else if (data[i] <= '\0' || data[i] > ' ')
				{
					return false;
				}
				break;
			case 1:
				// String not started yet.
				if (data[i] == ']')
				{
					config.setStrings(key, view);
					return true;
				}
				else if (data[i] == '"')
				{
					state = 2;
					start = i + 1;
				}
				else if (data[i] <= '\0' || data[i] > ' ')
				{
					return false;
				}
				break;
			case 2:
				// In a string, not escaped.
				if (data[i] == '\0')
				{
					return false;
				}
				else if (data[i] == '\\')
				{
					state = 3;
				}
				else if (data[i] == '"')
				{
					// Add the string.  Doesn't currently escape things.
					view.push_back(StringView(data + start, i - start));
					state = 4;
				}
				break;
			case 3:
				// In a string, not escaped.
				if (data[i] == '\0')
				{
					return false;
				}
				else
				{
					state = 2;
				}
				break;
			case 4:
				// String just ended.
				if (data[i] == ']')
				{
					config.setStrings(key, view);
					return true;
				}
				else if (data[i] == ',')
				{
					state = 1;
				}
				else if (data[i] <= '\0' || data[i] > ' ')
				{
					return false;
				}
				break;
			}
		}
		return false;
	}

	bool setConfigFromString(StringView conf)
	{
		size_t split = conf.find_first_of('=');
		if (split == StringView::npos)
		{
			// Invalid option.  No `=`.
			return false;
		}
		// Get the type of this option from the existing options (defaults and files).
		StringView key = trim(StringView(conf.data(), split));
		StringView value = trim(StringView(conf.data() + split + 1, conf.length() - split - 1));
		if (key.empty())
		{
			logLn(LogLevel::Warning, "No key supplied");
			return false;
		}
		if (value.empty())
		{
			logLn(LogLevel::Warning, "No value supplied");
			return false;
		}

		return setConfigFromString(conf, key, value);
	}

	bool setConfigFromString(StringView conf, StringView key, StringView value)
	{
		try
		{
			// Try the code twice - once for the given config, once for it translated from legacy.
			bool retry = false;
			do
			{
				switch (config.getType(key))
				{
				case ConfigOptionType_Int:
					*config.getInt(key) = std::stoi(value.data());
					return true;
				case ConfigOptionType_String:
					// TODO: This is a problem.  Most uses hold references to the internal config data,
					// which we can modify.  Strings don't.  Thus setting a string here won't update all the
					// uses of that string.
					config.setString(key, value);
					return true;
				case ConfigOptionType_Float:
					*config.getFloat(key) = std::stod(value.data());
					return true;
				case ConfigOptionType_Strings:
					// Unfortunately we're still setting up the config options so this may display
					// oddly (wrong place/prefix etc).
					setArrayFromString(key, value);
					return false;
				case ConfigOptionType_Bool:
					*config.getBool(key) = value == "true" || (value != "false" && !!std::stoi(value.data()));
					return true;
				default:
					// Try the loop again with a new key.
					auto legacyLookup = components.queryComponent<ILegacyConfigComponent>();
					if (retry)
					{
						// Check for the second time getting to here.  Shouldn't happen as that
						// means a legacy option resolved to an unknown config, but handle it.
						retry = false;
					}
					else if (legacyLookup)
					{
						// Did they type a legacy key?
						StringView nu = legacyLookup->getConfig(key);
						if (!nu.empty())
						{
							logLn(LogLevel::Warning, "Legacy key `%.*s` supplied, using `%s`", key.length(), key.data(), nu.data());
							key = nu;
							retry = true;
						}
					}
					break;
				}
			} while (retry);
		}
		catch (std::invalid_argument const& e)
		{
			// The value was wrong.
			logLn(LogLevel::Warning, "Value `%.*s` could not be parsed", value.length(), value.data());
			return false;
		}
		logLn(LogLevel::Warning, "Unknown config key `%.*s`", key.length(), key.data());
		return false;
	}

public:
	bool reloadLogFile()
	{
		if (!logFile)
		{
			return false;
		}

		fclose(logFile);
		logFile = ::fopen(LogFileName.c_str(), "a");
		return true;
	}

	void run()
	{
		sleepTimer = Microseconds(static_cast<long long>(*config.getFloat("sleep") * 1000.0f));
		_useDynTicks = *config.getBool("use_dyn_ticks");
		TimePoint prev = Time::now();
		sleepDuration = sleepTimer;

		while (run_)
		{
			const TimePoint now = Time::now();
			const Microseconds us = duration_cast<Microseconds>(now - prev);

			if (_useDynTicks)
			{
				sleepDuration += sleepTimer - us;
			}

			prev = now;

			if (now - ticksPerSecondLastUpdate >= Seconds(1))
			{
				ticksPerSecondLastUpdate = now;
				ticksPerSecond = ticksThisSecond;
				ticksThisSecond = 0u;
			}
			++ticksThisSecond;

			eventDispatcher.dispatch(&CoreEventHandler::onTick, us, now);

			for (auto it = httpFutures.begin(); it != httpFutures.end();)
			{
				HTTPAsyncIO* httpIO = *it;
				if (httpIO->tryExec())
				{
					delete httpIO;
					it = httpFutures.erase(it);
				}
				else
				{
					++it;
				}
			}

			std::this_thread::sleep_until(now + sleepDuration);
		}
	}

	void setThreadSleep(Microseconds value) override
	{
		sleepTimer = value;
	}

	void useDynTicks(const bool enable) override
	{
		_useDynTicks = enable;

		if (!enable)
		{
			sleepDuration = sleepTimer;
		}
	}

	void resetAll() override
	{
		reloading_ = true;
		NetCode::RPC::PlayerClose RPC;
		PacketHelper::broadcast(RPC, players);
		components.reset();
		players.removeSyncPacketsHandlers();

		for (auto p : players.entries())
		{
			Player* player = static_cast<Player*>(p);
			player->reset();
		}
	}

	void reloadAll() override
	{
		players.addSyncPacketsHandlers();
		reloading_ = false;
		for (auto p : players.entries())
		{
			Player* player = static_cast<Player*>(p);
			// Set weather & time back to server's ones.
			// https://github.com/openmultiplayer/server-beta/issues/167
			player->weather_ = *SetWeather;
			player->time_ = duration_cast<Minutes>(Hours(*SetWorldTime));
			playerInit(*p);
		}
	}

	void stop()
	{
		run_ = false;
	}

	Core(const cxxopts::ParseResult& cmd)
		: players(*this)
		, config(*this, false, &cmd)
		, console(nullptr)
		, models(nullptr)
		, logFile(nullptr)
		, run_(true)
		, ticksPerSecond(0u)
		, ticksThisSecond(0u)
		, EnableLogTimestamp(false)
	{
		// Initialize start time
		getTickCount();

		players.getPlayerConnectDispatcher().addEventHandler(this, EventPriority_FairlyLow);

		// Read config params before loading config file
		if (cmd.count("config"))
		{
			auto configs = cmd["config"].as<std::vector<std::string>>();
			// Loop through all the config options, get the key/value pair, and store it.
			for (auto const& conf : configs)
			{
				setConfigFromString(conf);
			}
		}

		if (config.getString("logging.file").length())
		{
			StringView logFilePath = config.getString("logging.file");
			LogFileName = logFilePath.data();
		}

		// Decide whether to enable logging early on
		if (*config.getBool("logging.enable"))
		{
			logFile = ::fopen(LogFileName.c_str(), "a");
		}

		printLn("Starting open.mp server (%u.%u.%u.%u) from commit %.*s", getVersion().major, getVersion().minor, getVersion().patch, getVersion().prerel, PRINT_VIEW(getVersionHash()));

#ifdef OMP_STATIC_OPENSSL
		logLn(LogLevel::Warning, "Running static OpenSSL build - plugins that use OpenSSL might crash (discord-connector, pawn-requests). It's recommended to use the dynssl build.");
#endif

		// Try to load components from the current directory
		loadComponents("components");

		// No components found in the current directory, try the executable path
		if (components.size() == 0)
		{
			auto componentsDir = utils::GetExecutablePath().remove_filename();
			componentsDir /= ghc::filesystem::path("components");
			loadComponents(componentsDir);
		}

		if (cmd.count("default-config"))
		{
			// Generate config
			config.writeDefault(components);
			stop();
			return;
		}

		components.configure(*this, config, false);

		config.setInt("max_players", std::clamp(*config.getInt("max_players"), 1, PLAYER_POOL_SIZE));

		// If logging was enabled by config file, open the file
		if (*config.getBool("logging.enable") && logFile == nullptr)
		{
			logFile = ::fopen(LogFileName.c_str(), "a");
		}

		// Overwrite config file data with config params
		if (cmd.count("config"))
		{
			auto configs = cmd["config"].as<std::vector<std::string>>();
			// Loop through all the config options, get the key/value pair, and store it.
			for (auto const& conf : configs)
			{
				setConfigFromString(conf);
			}
		}

		if (cmd.count("script"))
		{
			// Add the launch parameter to the start of the scripts list.
			// Something here corrupts the strings between loading and inserting.  Hence doing this
			// the hard way.  Yes, this is a copy.  On purpose, sadly.  The corruption was because
			// we got the old value, modified it, then passed it in for the new value.  But our
			// modified value was only a pointer to the old value, which was now being overridden.
			DynamicArray<String> const* mainScripts = config.getStrings("pawn.main_scripts");
			DynamicArray<StringView> view {};
			String entry_file = cmd["script"].as<String>();
			view.push_back(entry_file);
			if (mainScripts)
			{
				for (String const& v : *mainScripts)
				{
					view.push_back(v);
				}
			}
			config.setStrings("pawn.main_scripts", view);
		}
		// Don't use config before this point
		if (cmd.count("dump-config"))
		{
			// Generate config
			config.writeCurrent();
			stop();
			return;
		}

		const SemanticVersion version = getVersion();
		String versionStr = "open.mp " + std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch);
		if (version.prerel != 0)
		{
			versionStr += "." + std::to_string(version.prerel);
		}
		config.setString("version", versionStr);

		EnableZoneNames = config.getBool("game.use_zone_names");
		UsePlayerPedAnims = config.getBool("game.use_player_ped_anims");
		AllowInteriorWeapons = config.getBool("game.allow_interior_weapons");
		UseLimitGlobalChatRadius = config.getBool("game.use_chat_radius");
		LimitGlobalChatRadius = config.getFloat("game.chat_radius");
		EnableStuntBonus = config.getBool("game.use_stunt_bonuses");
		SetNameTagDrawDistance = config.getFloat("game.nametag_draw_radius");
		EnableInteriorEnterExits = config.getBool("game.use_entry_exit_markers");
		EnableNameTagLOS = config.getBool("game.use_nametag_los");
		ManualVehicleEngineAndLights = config.getBool("game.use_manual_engine_and_lights");
		ShowNameTags = config.getBool("game.use_nametags");
		ShowPlayerMarkers = config.getInt("game.player_marker_mode");
		SetWorldTime = config.getInt("game.time");
		SetWeather = config.getInt("game.weather");
		SetGravity = config.getFloat("game.gravity");
		LanMode = config.getBool("network.use_lan_mode");
		SetDeathDropAmount = config.getInt("game.death_drop_amount");
		Instagib = config.getBool("game.use_instagib");
		OnFootRate = config.getInt("network.on_foot_sync_rate");
		InCarRate = config.getInt("network.in_vehicle_sync_rate");
		WeaponRate = config.getInt("network.aiming_sync_rate");
		Multiplier = config.getInt("network.multiplier");
		LagCompensation = config.getInt("game.lag_compensation_mode");
		EnableVehicleFriendlyFire = config.getBool("game.use_vehicle_friendly_fire");

		EnableLogTimestamp = *config.getBool("logging.use_timestamp");
		EnableLogPrefix = *config.getBool("logging.use_prefix");
		LogTimestampFormat = String(config.getString("logging.timestamp_format"));

		config.optimiseBans();
		config.writeBans();
		components.load(this);
		config.init(components);
		players.init(components); // Players must ALWAYS be initialised before components
		components.init();

		console = components.queryComponent<IConsoleComponent>();
		if (console)
		{
			console->getEventDispatcher().addEventHandler(this);
		}

		models = components.queryComponent<ICustomModelsComponent>();
		components.ready();
	}

	~Core()
	{
		if (console)
		{
			console->getEventDispatcher().removeEventHandler(this);
			console = nullptr;
		}

		players.getPlayerConnectDispatcher().removeEventHandler(this);

		players.free();
		networks.clear();
		components.free();

		if (logFile)
		{
			fclose(logFile);
		}
	}

	IConfig& getConfig() override
	{
		return config;
	}

	unsigned tickRate() const override
	{
		return ticksPerSecond;
	}

	SemanticVersion getVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	StringView getVersionHash() const override
	{
		return OMP_VERSION_HASH_STR;
	}

	int getNetworkBitStreamVersion() const override
	{
		return NetworkBitStream::Version;
	}

	void printLn(const char* fmt, ...) override
	{
		va_list args;
		va_start(args, fmt);
		vprintLn(fmt, args);
		va_end(args);
	}

	void vprintLn(const char* fmt, va_list args) override
	{
		vlogLn(LogLevel::Message, fmt, args);
	}

	virtual void logLn(LogLevel level, const char* fmt, ...) override
	{
		va_list args;
		va_start(args, fmt);
		vlogLn(level, fmt, args);
		va_end(args);
	}

	void printLnU8(const char* fmt, ...) override
	{
		va_list args;
		va_start(args, fmt);
		vprintLnU8(fmt, args);
		va_end(args);
	}

	void vprintLnU8(const char* fmt, va_list args) override
	{
		vlogLnU8(LogLevel::Message, fmt, args);
	}

	virtual void logLnU8(LogLevel level, const char* fmt, ...) override
	{
		va_list args;
		va_start(args, fmt);
		vlogLnU8(level, fmt, args);
		va_end(args);
	}

	void logToStream(FILE* stream, const char* iso8601, const char* prefix, const char* message)
	{
		if (iso8601[0])
		{
			fputs(iso8601, stream);
			fputs(" ", stream);
		}
		if (prefix)
		{
			fputs(prefix, stream);
		}
		fputs(message, stream);
		fputs("\n", stream);
		fflush(stream);
	}

	virtual void vlogLn(LogLevel level, const char* fmt, va_list args) override
	{
		vlogLnInternal(level, false, fmt, args);
	}

	virtual void vlogLnU8(LogLevel level, const char* fmt, va_list args) override
	{
		vlogLnInternal(level, true, fmt, args);
	}

	virtual void vlogLnInternal(LogLevel level, bool utf8, const char* fmt, va_list args)
	{
#ifndef _DEBUG
		if (level == LogLevel::Debug)
		{
			return;
		}
#endif

#ifdef BUILD_WINDOWS
		_lock_locales();
		UINT oldCP = 0;
		wchar_t oldLocale[64] = { 0 };
		bool oldLocaleSaved = false;
		if (utf8)
		{
			oldCP = GetConsoleOutputCP();
			SetConsoleOutputCP(CP_UTF8);

			/* Getting current locale */
			const wchar_t* old_locale_ptr = _wsetlocale(LC_CTYPE, nullptr);
			if (old_locale_ptr != nullptr)
			{
				wcscpy_s(oldLocale, old_locale_ptr);
				oldLocaleSaved = true;

				std::setlocale(LC_CTYPE, ".UTF-8");
			}
		}
#endif

#ifdef BUILD_WINDOWS
		if (level == LogLevel::Debug)
		{
			char debugStr[4096] = { 0 };
			const int written = vsnprintf(debugStr, sizeof(debugStr) - 1, fmt, args);
			debugStr[written] = '\n';
			OutputDebugString(debugStr);
		}
#endif
		const char* prefix = nullptr;
		if (EnableLogPrefix)
		{
			switch (level)
			{
			case LogLevel::Debug:
				prefix = "[Debug] ";
				break;
			case LogLevel::Message:
				prefix = "[Info] ";
				break;
			case LogLevel::Warning:
				prefix = "[Warning] ";
				break;
			case LogLevel::Error:
				prefix = "[Error] ";
			}
		}

		char iso8601[32] = { 0 };
		if (EnableLogTimestamp && !LogTimestampFormat.empty())
		{
			std::time_t now = WorldTime::to_time_t(WorldTime::now());
			std::strftime(iso8601, sizeof(iso8601), LogTimestampFormat.c_str(), std::localtime(&now));
		}

		char main[4096];
		std::unique_ptr<char[]> fallback; // In case the string is larger than 4096
		Span<char> buf(main, sizeof(main));

		va_list args_copy;
		va_copy(args_copy, args);
		const int len = vsnprintf(nullptr, 0, fmt, args_copy);
		va_end(args_copy);

		if (len > sizeof(main))
		{
			// Stack won't fit our string; allocate space for it
			fallback.reset(new char[len]);
			buf = Span<char>(fallback.get(), len);
		}
		buf[0] = 0;
		vsnprintf(buf.data(), buf.size(), fmt, args);

		FILE* stream
			= stdout;
		if (level == LogLevel::Error)
		{
			stream = stderr;
		}
		logToStream(stream, iso8601, prefix, buf.data());
		if (logFile)
		{
			logToStream(logFile, iso8601, prefix, buf.data());
		}

#ifdef BUILD_WINDOWS
		if (utf8)
		{
			if (oldLocaleSaved)
			{
				_wsetlocale(LC_CTYPE, oldLocale);
			}
			SetConsoleOutputCP(oldCP);
		}
		_unlock_locales();
#endif
	}

	IPlayerPool& getPlayers() override
	{
		return players;
	}

	const FlatPtrHashSet<INetwork>& getNetworks() override
	{
		return networks;
	}

	unsigned getTickCount() const override
	{
		return utils::GetTickCount();
	}

	IEventDispatcher<CoreEventHandler>& getEventDispatcher() override
	{
		return eventDispatcher;
	}

	void setGravity(float gravity) override
	{
		*SetGravity = gravity;
		NetCode::RPC::SetPlayerGravity RPC;
		RPC.Gravity = gravity;
		PacketHelper::broadcast(RPC, players);

		for (IPlayer* player : players.entries())
		{
			static_cast<Player*>(player)->gravity_ = gravity;
		}

		updateNetworks();
	}

	float getGravity() const override
	{
		return *SetGravity;
	}

	void setWeather(int weather) override
	{
		*SetWeather = weather;
		NetCode::RPC::SetPlayerWeather RPC;
		RPC.WeatherID = weather;
		PacketHelper::broadcast(RPC, players);

		for (IPlayer* player : players.entries())
		{
			static_cast<Player*>(player)->weather_ = weather;
		}

		updateNetworks();
	}

	void setWorldTime(Hours time) override
	{
		time %= 24;
		*SetWorldTime = time.count();
		NetCode::RPC::SetPlayerWorldTime RPC;
		RPC.Time = time;
		PacketHelper::broadcast(RPC, players);

		for (IPlayer* player : players.entries())
		{
			static_cast<Player*>(player)->time_ = time;
		}

		updateNetworks();
	}

	void useStuntBonuses(bool enable) override
	{
		*EnableStuntBonus = enable;
		NetCode::RPC::EnableStuntBonusForPlayer RPC;
		RPC.Enable = enable;
		PacketHelper::broadcast(RPC, players);
	}

	void setData(SettableCoreDataType type, StringView data) override
	{
		switch (type)
		{
		case SettableCoreDataType::ServerName:
			config.setString("name", data);
			break;
		case SettableCoreDataType::ModeText:
			config.setString("game.mode", data);
			break;
		case SettableCoreDataType::MapName:
			config.setString("game.map", data);
			break;
		case SettableCoreDataType::Language:
			config.setString("language", data);
			break;
		case SettableCoreDataType::Password:
			config.setString("password", data);
			break;
		case SettableCoreDataType::AdminPassword:
			config.setString("rcon.password", data);
			break;
		case SettableCoreDataType::URL:
			config.setString("website", data);
			break;
		}

		updateNetworks();
	}

	StringView getWeaponName(PlayerWeapon weapon) override
	{
		int index = int(weapon);
		if (weapon < 0 || weapon > PlayerWeapon_End)
		{
			return "Invalid";
		}
		return PlayerWeaponNames[index];
	}

	void onPlayerConnect(IPlayer& player) override
	{
		playerInit(player);
		if (reloading_)
		{
			// Close the player again.
			NetCode::RPC::PlayerClose RPC;
			PacketHelper::send(RPC, player);
		}
	}

	void connectBot(StringView name, StringView script) override
	{
		StringView bind = config.getString("network.bind");
		int port = *config.getInt("network.port");
		StringView password = config.getString("password");
		std::string args = "-h " + (bind.empty() ? "127.0.0.1" : std::string(bind)) + " -p " + std::to_string(port) + " -n " + std::string(name) + " -m " + std::string(script);
		if (!password.empty())
		{
			args += " -z " + std::string(password);
		}
		utils::RunProcess(*this, config.getString("bot_exe"), args, true);
	}

	void requestHTTP(HTTPResponseHandler* handler, HTTPRequestType type, StringView url, StringView data) override
	{
		HTTPAsyncIO* httpIO = new HTTPAsyncIO(handler, type, url, data);
		httpFutures.emplace(httpIO);
	}

	bool sha256(StringView password, StringView salt, StaticArray<char, 64 + 1>& output) const override
	{
		String input(String(password) + String(salt));

		SHA256_CTX ctx {};
		if (!SHA256_Init(&ctx))
		{
			return false;
		}
		if (!SHA256_Update(&ctx, input.data(), input.length()))
		{
			return false;
		}
		unsigned char md[SHA256_DIGEST_LENGTH];
		if (!SHA256_Final(md, &ctx))
		{
			return false;
		}

		char* data = output.data();
		for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
		{
			sprintf(data + 2 * i, "%02X", md[i]);
		}
		data[64] = '\0';

		return true;
	}

	void onConsoleCommandListRequest(FlatHashSet<StringView>& commands) override
	{
		commands.emplace("exit");
		commands.emplace("reloadlog");
		commands.emplace("config");
		commands.emplace("varlist");
	}

	bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override
	{
		if (command == "exit")
		{
			stop();
			return true;
		}
		else if (command == "reloadlog")
		{
			if (reloadLogFile())
			{
				console->sendMessage(sender, "Reloaded log file \"" + String(LogFileName) + "\".");
			}
			return true;
		}
		else if (command == "config")
		{
			if (parameters.length() < 2 || *(parameters.data()) != '"' || *(parameters.data() + parameters.length() - 1) != '"')
			{
				setConfigFromString(parameters);
			}
			else
			{
				// Remove `"`s.
				setConfigFromString(StringView(parameters.data() + 1, parameters.length() - 2));
			}
			updateNetworks();
			return true;
		}
		else if (command == "varlist")
		{
			console->sendMessage(sender, "Console variables:");
			VarlistEnumCallback cb(*console, config, sender);
			config.enumOptions(cb);
			return true;
		}
		else // Process potential variable set
		{
			const auto alias = config.getNameFromAlias(command);
			const StringView name = (alias.first ? alias.second : command);
			const ConfigOptionType type = config.getType(name);
			if (type != ConfigOptionType_None)
			{
				if (parameters.empty())
				{
					VarlistEnumCallback(*console, config, sender).proc(name, type);
				}
				else
				{
					setConfigFromString(parameters, command, parameters);
					updateNetworks();
				}
				return true;
			}
		}
		return false;
	}

	void requestHTTP4(HTTPResponseHandler* handler, HTTPRequestType type, StringView url, StringView data) override
	{
		HTTPAsyncIO* httpIO = new HTTPAsyncIO(handler, type, url, data, true, config.getString("network.bind"));
		httpFutures.emplace(httpIO);
	}
};