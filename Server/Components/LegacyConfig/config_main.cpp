/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/network_impl.hpp>
#include <Server/Components/Console/console.hpp>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sdk.hpp>
#include <thread>
#include <charconv>

using namespace Impl;

enum class ParamType {
    Int,
    Float,
    String,
    StringList,
    Custom,
    Obsolete
};

const FlatHashMap<StringView, ParamType> types = {
    { "echo", ParamType::Custom },
    { "rcon_password", ParamType::String },
    { "rcon", ParamType::Int },
    { "gamemode", ParamType::Custom },
    { "filterscripts", ParamType::Custom },
    { "plugins", ParamType::StringList },
    { "announce", ParamType::Int },
    { "query", ParamType::Int },
    { "hostname", ParamType::String },
    { "language", ParamType::String },
    { "mapname", ParamType::String },
    { "gamemodetext", ParamType::String },
    { "weather", ParamType::Int },
    { "worldtime", ParamType::Obsolete },
    { "gravity", ParamType::Float },
    { "weburl", ParamType::String },
    { "maxplayers", ParamType::Int },
    { "password", ParamType::Custom },
    { "sleep", ParamType::Int },
    { "lanmode", ParamType::Int },
    { "bind", ParamType::String },
    { "port", ParamType::Int },
    { "conncookies", ParamType::Obsolete },
    { "cookielogging", ParamType::Int },
    { "connseedtime", ParamType::Int },
    { "minconnectiontime", ParamType::Int },
    { "messageslimit", ParamType::Int },
    { "messageholelimit", ParamType::Int },
    { "ackslimit", ParamType::Int },
    { "playertimeout", ParamType::Int },
    { "mtu", ParamType::Int },
    { "output", ParamType::Obsolete },
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
    { "rcon", "enable_rcon" },
    { "rcon_password", "rcon_password" },
    { "gamemode", "pawn.main_scripts" },
    { "filterscripts", "pawn.side_scripts" },
    { "plugins", "pawn.legacy_plugins" },
    { "announce", "announce" },
    { "query", "enable_query" },
    { "hostname", "server_name" },
    { "language", "language" },
    { "mapname", "map_name" },
    { "gamemodetext", "mode_name" },
    { "weather", "weather" },
    { "gravity", "gravity" },
    { "weburl", "website" },
    { "maxplayers", "max_players" },
    { "password", "password" },
    { "sleep", "sleep" },
    { "lanmode", "lan_mode" },
    { "bind", "bind" },
    { "port", "port" },
    { "cookielogging", "logging_cookies" },
    { "connseedtime", "cookie_reseed_time" },
    { "minconnectiontime", "min_connection_time" },
    { "messageslimit", "messages_limit" },
    { "messageholelimit", "message_hole_limit" },
    { "ackslimit", "acks_limit" },
    { "playertimeout", "player_timeout" },
    { "mtu", "network_mtu" },
    { "timestamp", "logging_timestamp" },
    { "logtimeformat", "logging_timestamp_format" },
    { "logqueries", "logging_queries" },
    { "chatlogging", "logging_chat" },
    { "db_logging", "logging_sqlite" },
    { "db_log_queries", "logging_sqlite_queries" },
    { "onfoot_rate", "on_foot_rate" },
    { "incar_rate", "in_car_rate" },
    { "weapon_rate", "weapon_rate" },
    { "stream_distance", "stream_distance" },
    { "stream_rate", "stream_rate" },
    { "maxnpc", "max_bots" },
    { "lagcompmode", "lag_compensation" }
};

class LegacyConfigComponent final : public IComponent, public ConsoleEventHandler {
public:
    PROVIDE_UID(0x24ef6216838f9ffc);

private:
    ICore* core;
    IConsoleComponent* console;
    DynamicArray<String> gamemodes_;

    bool processCustom(ILogger& logger, IEarlyConfig& config, String name, String right)
    {
        if (name.find("gamemode") == 0) {
            auto it = dictionary.find("gamemode");
            if (it != dictionary.end()) {
                int gmidx = 0;
                auto conv = std::from_chars(name.data() + 8, name.data() + name.size(), gmidx, 10);
                if (conv.ec == std::errc::invalid_argument || conv.ec == std::errc::result_out_of_range || gmidx < 0) {
                    gmidx = 0;
                }
                while (gamemodes_.size() <= gmidx) {
                    gamemodes_.push_back("");
                }
                // Don't strip spaces here.  That was to find the count, but that is now done later.
                gamemodes_[gmidx] = right;
                return true;
            }
        }

        if (name.find("filterscripts") == 0) {
            auto it = dictionary.find("filterscripts");
            if (it != dictionary.end()) {
                String listStr = right;
                DynamicArray<String> storage;
                size_t i = 0;
                for (;;) {
                    size_t next = listStr.find_first_of(' ', i);

                    std::filesystem::path path("filterscripts");
                    if (next != String::npos) {
                        path /= listStr.substr(i, next - i);
                    } else {
                        path /= listStr.substr(i);
                        storage.emplace_back(path.string());
                        break;
                    }
                    storage.emplace_back(path.string());

                    i = next + 1;
                }
                DynamicArray<StringView> list;
                for (int i = 0; i < storage.size(); ++i) {
                    list.emplace_back(storage[i]);
                }
                config.setStrings(it->second, list);
                return true;
            }
        }

        if (name.find("echo") == 0) {
            logger.printLn("%s", right.c_str());
            return true;
        }

        if (name.find("password") == 0) {
            String password = "\0";
            if (right.size() > 0 && right[0] != '0') {
                password = right;
            }
            auto it = dictionary.find("password");
            config.setString(it->second, password);
            return true;
        }

        return false;
    }

    bool processDefault(ILogger& logger, IEarlyConfig& config, ParamType type, String name, String right)
    {
        auto dictIt = dictionary.find(name);
        if (dictIt != dictionary.end()) {
            switch (type) {
            case ParamType::Int: {
                try {
                    const int value = std::stoi(right);
                    config.setInt(dictIt->second, value);
                } catch (std::out_of_range e) {
                    logger.logLn(LogLevel::Error, "Invalid '%s' value passed. '%s' is out of integer bounds (0-%d).", name.c_str(), right.c_str(), INT_MAX);
                } catch (std::invalid_argument e) {
                    logger.logLn(LogLevel::Error, "Invalid '%s' value passed. '%s' is not an integer.", name.c_str(), right.c_str());
                }
                return true;
            }
            case ParamType::Float: {
                try {
                    const float value = std::stof(right);
                    config.setFloat(dictIt->second, value);
                } catch (std::invalid_argument e) {
                    logger.logLn(LogLevel::Error, "Invalid '%s' value passed. '%s' is not a floating number.", name.c_str(), right.c_str());
                }
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
                size_t i = 0;
                for (;;) {
                    size_t next = listStr.find_first_of(' ', i);

                    if (next != String::npos) {
                        storage.emplace_back(listStr.substr(i, next - i));
                    } else {
                        storage.emplace_back(listStr.substr(i));
                        break;
                    }

                    i = next + 1;
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

    bool loadLegacyConfigFile(ILogger& logger, IEarlyConfig& config, const std::string& filename)
    {
        std::ifstream cfg(filename);
        if (cfg.good()) {
            gamemodes_.clear();
            for (String line; std::getline(cfg, line);) {
                size_t idx;
                // Ignore // comments
                idx = line.find("//");
                if (idx != String::npos) {
                    line = line.substr(0, idx);
                }
                // Ignore # comments
                idx = line.find_first_of('#');
                if (idx != String::npos) {
                    line = line.substr(0, idx);
                }

                // Remove all spaces from the beginning of the string
                while (line.size() && isspace(line.front())) {
                    line.erase(line.begin());
                }
                // Remove all spaces from the end of the string
                while (line.size() && isspace(line.back())) {
                    line.pop_back();
                }

                // Skip empty lines
                if (line.size() == 0) {
                    continue;
                }

                // Get the setting name
                String name = line;
                idx = name.find_first_of(' ');
                if (idx != String::npos) {
                    name = line.substr(0, idx);
                } else {
                    // No value; skip line
                    continue;
                }

                auto typeIt = types.find(name);
                if (typeIt != types.end()) {
                    // Process default dictionary items
                    if (typeIt->second == ParamType::Custom) {
                        if (!processCustom(logger, config, name, line.substr(idx + 1))) {
                            logger.logLn(LogLevel::Warning, "Parsing unknown legacy option %s", name.c_str());
                        }
                    } else if (typeIt->second == ParamType::Obsolete) {
                        logger.logLn(LogLevel::Warning, "Parsing obsolete legacy option %s", name.c_str());
                    } else if (!processDefault(logger, config, typeIt->second, name, line.substr(idx + 1))) {
                        logger.logLn(LogLevel::Warning, "Parsing unknown legacy option %s", name.c_str());
                    }
                } else if (!processCustom(logger, config, name, line.substr(idx + 1))) {
                    logger.logLn(LogLevel::Warning, "Parsing unknown legacy option %s", name.c_str());
                }
            }
            size_t gmcount = 0;
            DynamicArray<StringView> list;
            for (int i = 0; i < gamemodes_.size(); ++i) {
                if (gamemodes_[i] != "") {
                    ++gmcount;
                    list.emplace_back(gamemodes_[i]);
                }
            }
            if (gmcount != 0) {
                config.setStrings("pawn.main_scripts", list);
            }
            return true;
        }
        return false;
    }

public:
    StringView componentName() const override
    {
        return "LegacyConfig";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void onLoad(ICore* c) override { core = c; }
    void onInit(IComponentList* components) override
    {
        console = components->queryComponent<IConsoleComponent>();

        if (console) {
            console->getEventDispatcher().addEventHandler(this);
        }
    }

    void onFree(IComponent* component) override
    {
        if (console == component) {
            console = nullptr;
        }
    }

    ~LegacyConfigComponent()
    {
        if (console) {
            console->getEventDispatcher().removeEventHandler(this);
        }
    }

    void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) override
    {
        // Don't provide defaults for generating the config file
        if (!defaults) {
            if (config.getString("bot_exe").empty()) {
                config.setString("bot_exe", "samp-npc");
            }

            // someone can parse samp.ban properly, I didn't care to
            std::ifstream bans("samp.ban");
            if (bans.good()) {
                for (String line; std::getline(bans, line);) {
                    size_t first = line.find_first_of(' ');
                    if (first != String::npos) {
                        config.addBan(BanEntry(line.substr(0, first), "", line.substr(first + 1)));
                    }
                }
            }

            for (auto& kv : dictionary) {
                config.addAlias(kv.first, kv.second, true);
            }

            loadLegacyConfigFile(logger, config, "server.cfg");
        }
    }

    void free() override
    {
        delete this;
    }

    bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override
    {
        if (command == "exec") {
            if (parameters.empty()) {
                console->sendMessage(sender, "Usage: exec <filename>");
            } else {
                auto file_name = String(parameters);

                if (file_name.find_last_of(".cfg") != file_name.size() - 1) {
                    file_name += ".cfg";
                }

                if (!loadLegacyConfigFile(*core, static_cast<IEarlyConfig&>(core->getConfig()), file_name)) {
                    console->sendMessage(sender, "Unable to exec file '" + file_name + "'.");
                } else {
                    console->sendMessage(sender, "Loaded configuration file '" + file_name + "'.");
                    core->updateNetworks();
                }
            }
            return true;
        }
        return false;
    }

    void reset() override
    {
        // Nothing to reset here.
    }
};

COMPONENT_ENTRY_POINT()
{
    return new LegacyConfigComponent();
}
