#include <Server/Components/Console/console.hpp>
#include <sdk.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <fstream>

enum class ParamType {
	Int,
	Float,
	String,
	StringList,
	Custom
};

const FlatHashMap<StringView, ParamType> types = {
	{ "echo", ParamType::String },
	{ "rcon_password", ParamType::String },
	{ "rcon", ParamType::Int },
	{ "gamemode", ParamType::Custom },
	{ "filterscripts", ParamType::StringList },
	{ "plugins", ParamType::StringList },
	{ "announce", ParamType::Int },
	{ "query", ParamType::Int },
	{ "hostname", ParamType::String },
	{ "language", ParamType::String },
	{ "mapname", ParamType::String },
	{ "gamemodetext", ParamType::String },
	{ "weather", ParamType::Int },
	{ "worldtime", ParamType::Custom },
	{ "gravity", ParamType::Float },
	{ "weburl", ParamType::String },
	{ "maxplayers", ParamType::Int },
	{ "password", ParamType::String },
	{ "sleep", ParamType::Int },
	{ "lanmode", ParamType::Int },
	{ "bind", ParamType::String },
	{ "port", ParamType::Int },
	{ "conncookies", ParamType::Int },
	{ "cookielogging", ParamType::Int },
	{ "connseedtime", ParamType::Int },
	{ "minconnectiontime", ParamType::Int },
	{ "messageslimit", ParamType::Int },
	{ "messageholelimit", ParamType::Int },
	{ "ackslimit", ParamType::Int },
	{ "playertimeout", ParamType::Int },
	{ "mtu", ParamType::Int },
	{ "output", ParamType::Int },
	{ "timestamp", ParamType::Int },
	{ "logtimeformat", ParamType::String },
	{ "logqueries", ParamType::Int },
	{ "chatlogging", ParamType::Int },
	{ "db_logging", ParamType::Int },
	{ "db_log_queries", ParamType::Int },
	{ "onfoot_rate", ParamType::Int },
	{ "incar_rate", ParamType::Int },
	{ "weapon_rate", ParamType::Int },
	{ "stream_distance", ParamType::Float },
	{ "stream_rate", ParamType::Int },
	{ "maxnpc", ParamType::Int },
	{ "lagcompmode", ParamType::Int }
};

const FlatHashMap<StringView, StringView> dictionary = {
	//{ "echo", ParamType::String },
	//{ "rcon_password", ParamType::String },
	//{ "rcon", ParamType::Int },
	{ "gamemode", "entry_file" },
	{ "filterscripts", "side_scripts" },
	{ "plugins", "legacy_plugins" },
	{ "announce", "announce" },
	//{ "query", ParamType::Int },
	{ "hostname", "server_name" },
	//{ "language", ParamType::String },
	//{ "mapname", ParamType::String },
	//{ "gamemodetext", ParamType::String },
	{ "weather", "weather" },
	//{ "worldtime", ParamType::String }, TODO custom process
	{ "gravity", "gravity" },
	//{ "weburl", ParamType::String },
	{ "maxplayers", "max_players" },
	//{ "password", ParamType::String },
	{ "sleep", "sleep" },
	{ "lanmode", "lan_mode" },
	{ "bind", "bind" },
	{ "port", "port" },
	//{ "conncookies", ParamType::Int },
	//{ "cookielogging", ParamType::Int },
	//{ "connseedtime", "cookie_reseed_time" },
	//{ "minconnectiontime", ParamType::Int },
	//{ "messageslimit", ParamType::Int },
	//{ "messageholelimit", ParamType::Int },
	//{ "ackslimit", ParamType::Int },
	{ "playertimeout", "player_timeout" },
	//{ "mtu", ParamType::Int },
	//{ "output", ParamType::Int },
	//{ "timestamp", ParamType::Int },
	//{ "logtimeformat", ParamType::String },
	//{ "logqueries", ParamType::Int },
	//{ "chatlogging", ParamType::Int },
	//{ "db_logging", ParamType::Int },
	//{ "db_log_queries", ParamType::Int },
	{ "onfoot_rate", "on_foot_rate" },
	{ "incar_rate", "in_car_rate" },
	{ "weapon_rate", "weapon_rate" },
	{ "stream_distance", "stream_distance" },
	{ "stream_rate", "stream_rate" },
	{ "maxnpc", "max_bots" },
	{ "lagcompmode", "lag_compensation" }
};

struct LegacyConfigComponent final : public IConfigProviderComponent {
	PROVIDE_UUID(0x24ef6216838f9ffc);

	StringView componentName() override {
		return "LegacyConfig";
	}

	bool processCustom(IEarlyConfig& config, String name, String right) {
		if (name.find("gamemode") == 0) {
			auto it = dictionary.find("gamemode");
			if (it != dictionary.end()) {
				int idx = right.find_first_of(' ');
				if (idx != -1) {
					right = right.substr(0, idx);
				}
				config.setString(it->second, right);
				return true;
			}
		}

		return false;
	}

	bool processDefault(IEarlyConfig& config, ParamType type, String name, String right) {
		auto dictIt = dictionary.find(name);
		if (dictIt != dictionary.end()) {
			switch (type) {
			case ParamType::Int: {
				const int value = std::stoi(right);
				config.setInt(dictIt->second, value);
				return true;
			}
			case ParamType::Float: {
				const float value = std::stof(right);
				config.setFloat(dictIt->second, value);
				return true;
			}
			case ParamType::String: {
				config.setString(dictIt->second, right);
				return true;
			}
			case ParamType::StringList: {
				String listStr = right;
				DynamicArray<String> storage;
				DynamicArray<StringView> list;
				for (int i = listStr.find_first_of(' '); i != -1;) {
					int next = listStr.find_first_of(' ', i + 1);

					if (next != -1) {
						storage.emplace_back(listStr.substr(i + 1, next - i - 1));
					}
					else {
						storage.emplace_back(listStr.substr(i + 1));
					}

					i = next;
				}
				for (int i = 0; i < storage.size(); ++i) {
					list.emplace_back(storage[i]);
				}
				config.setStrings(dictIt->second, list);
				return true;
			}
			default:
				return false;
			}
		}
		return false;
	}

	bool configure(IEarlyConfig& config) override {
		if (config.getString("bot_exe").empty()) {
			config.setString("bot_exe", "samp-npc");
		}

		// someone can parse samp.ban properly, I didn't care to
		std::ifstream bans("samp.ban");
		if (bans.good()) {
			for (String line; std::getline(bans, line);) {
				size_t first = line.find_first_of(' ');
				if (first != -1) {
					PeerAddress address;
					address.ipv6 = false;
					if (PeerAddress::FromString(address, line.substr(0, first))) {
						config.addBan(BanEntry(address, "LEGACY", "LEGACY"));
					}
				}
			}
		}

		std::ifstream cfg("server.cfg");
		if (cfg.good()) {
			for (String line; std::getline(cfg, line);) {
				String name = line;
				int idx = name.find_first_of(' ');
				if (idx != -1) {
					name = line.substr(0, idx);
				}

				auto typeIt = types.find(name);
				if (typeIt != types.end()) {
					// Process default dictionary items
					if (typeIt->second == ParamType::Custom) {
						if (!processCustom(config, name, line.substr(idx + 1))) {
							// warning
						}
					}
					else {
						processDefault(config, typeIt->second, name, line.substr(idx + 1));
					}
				}
				else if (!processCustom(config, name, line.substr(idx + 1))) {
					// warning
				}
			}
		}
		// todo read server.cfg, process it
		return true;
	}

} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}
