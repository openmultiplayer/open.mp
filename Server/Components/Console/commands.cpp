/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "cmd_handler.hpp"

FlatHashMap<String, CommandHandlerFuncType> ConsoleCmdHandler::Commands;

ADD_CONSOLE_CMD(gamemodetext, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        console.sendMessage(sender, String("game.mode = \"") + core->getConfig().getString("game.mode").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::ModeText, params);
});

ADD_CONSOLE_CMD(hostname, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        console.sendMessage(sender, String("name = \"") + core->getConfig().getString("name").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::ServerName, params);
});

ADD_CONSOLE_CMD(mapname, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        console.sendMessage(sender, String("game.map = \"") + core->getConfig().getString("game.map").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::MapName, params);
});

ADD_CONSOLE_CMD(weburl, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        console.sendMessage(sender, String("website = \"") + core->getConfig().getString("website").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::URL, params);
});

ADD_CONSOLE_CMD(language, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        console.sendMessage(sender, String("language = \"") + core->getConfig().getString("language").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::Language, params);
    console.sendMessage(sender, "Setting server language to: \"" + params + "\"");
});

ADD_CONSOLE_CMD(cmdlist, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    console.sendMessage(sender, "Console commands:");
    for (auto& kv : ConsoleCmdHandler::Commands) {
        console.sendMessage(sender, kv.first);
    }
});

struct VarlistEnumCallback : OptionEnumeratorCallback {
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

        switch (type) {
        case ConfigOptionType_Int:
            console.sendMessage(sender, name + " = " + std::to_string(*config.getInt(name)) + " (int)");
            break;
        case ConfigOptionType_Float:
            console.sendMessage(sender, name + " = " + std::to_string(*config.getFloat(name)) + " (float)");
            break;
        case ConfigOptionType_String:
            console.sendMessage(sender, name + " = \"" + String(config.getString(name)) + "\" (string)");
            break;
        case ConfigOptionType_Strings: {
            size_t count = config.getStringsCount(name);

            if (count) {
                DynamicArray<StringView> output(count);
                config.getStrings(name, Span<StringView>(output.data(), output.size()));

                String strings_list = "";

                for (auto& string : output) {
                    strings_list += String(string) + ' ';
                }

                if (strings_list.back() == ' ') {
                    strings_list.pop_back();
                }

                console.sendMessage(sender, name + " = \"" + strings_list + "\" (strings)");
            } else {
                console.sendMessage(sender, name + " = \"\" (strings)");
            }
        } break;
        default:
            break;
        }
        return true;
    }
};

ADD_CONSOLE_CMD(varlist, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    console.sendMessage(sender, "Console variables:");
    VarlistEnumCallback cb(console, core->getConfig(), sender);
    core->getConfig().enumOptions(cb);
});

ADD_CONSOLE_CMD(password, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        console.sendMessage(sender, String("password = \"") + core->getConfig().getString("password").data() + "\"");
        return;
    } else if (params == "0") {
        console.sendMessage(sender, "Server password has been removed.");
        core->setData(SettableCoreDataType::Password, "");
        return;
    }
    core->setData(SettableCoreDataType::Password, params);
    console.sendMessage(sender, "Setting server password to: \"" + params + "\"");
});

ADD_CONSOLE_CMD(say, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        return;
    }
    core->getPlayers().sendClientMessageToAll(Colour(37, 135, 206), String("* Admin: ") + params);
});

ADD_CONSOLE_CMD(players, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (!core->getPlayers().entries().size()) {
        return;
    }

    console.sendMessage(sender, "ID\tName\tPing\tIP");

    for (IPlayer* player : core->getPlayers().entries()) {
        if (player) {
            String ip = "";
            PeerNetworkData data = player->getNetworkData();
            if (!data.networkID.address.ipv6) {
                PeerAddress::AddressString addressString;
                if (PeerAddress::ToString(data.networkID.address, addressString)) {
                    ip = String(addressString.data(), addressString.length());
                }
            }
            String result = String(std::to_string(player->getID()) + "\t" + player->getName().data() + "\t" + std::to_string(player->getPing()) + "\t" + ip);
            console.sendMessage(sender, result);
        }
    }
});

ADD_CONSOLE_CMD(kick, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int playerId;
    if (sscanf(params.data(), "%i", &playerId) == EOF) {
        return;
    } else if (!core->getPlayers().get(playerId)) {
        return;
    }
    IPlayer* player = core->getPlayers().get(playerId);
    String ip = "";
    PeerNetworkData data = player->getNetworkData();
    if (!data.networkID.address.ipv6) {
        PeerAddress::AddressString addressString;
        if (PeerAddress::ToString(data.networkID.address, addressString)) {
            ip = String(addressString.data(), addressString.length());
        }
    }
    console.sendMessage(sender, player->getName().data() + String("<# " + std::to_string(player->getID()) + " - ") + ip + "> has been kicked.");
    player->kick();
});

ADD_CONSOLE_CMD(ban, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int playerId;
    if (sscanf(params.data(), "%i", &playerId) == EOF) {
        return;
    } else if (!core->getPlayers().get(playerId)) {
        return;
    }
    IPlayer* player = core->getPlayers().get(playerId);
    String ip = "";
    PeerNetworkData data = player->getNetworkData();
    if (!data.networkID.address.ipv6) {
        PeerAddress::AddressString addressString;
        if (PeerAddress::ToString(data.networkID.address, addressString)) {
            ip = String(addressString.data(), addressString.length());
        }
    }
    console.sendMessage(sender, player->getName().data() + String("<# " + std::to_string(player->getID()) + " - ") + ip + "> has been banned.");
    player->ban("CONSOLE BAN");
});

ADD_CONSOLE_CMD(banip, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        return;
    }

    const BanEntry banEntry(String(params.data()));
    core->getConfig().addBan(banEntry);
    for (INetwork* network : core->getNetworks()) {
        network->ban(banEntry);
    }
    core->getConfig().writeBans();
    console.sendMessage(sender, String("IP ") + params.data() + String(" has been banned."));
});

ADD_CONSOLE_CMD(reloadbans, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    core->getConfig().reloadBans();
    console.sendMessage(sender, "Banlist reloded.");
});

ADD_CONSOLE_CMD(unbanip, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        return;
    }

    BanEntry unban(String(params.data()));
    for (INetwork* network : core->getNetworks()) {
        network->unban(unban);
    }

    core->getConfig().removeBan(unban);
});

ADD_CONSOLE_CMD(gravity, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    float gravity = 0.008f;
    if (sscanf(params.data(), "%f", &gravity) == EOF) {
        console.sendMessage(sender, String("game.gravity = " + std::to_string(*core->getConfig().getFloat("game.gravity"))));
        return;
    }
    core->setGravity(gravity);
});

ADD_CONSOLE_CMD(weather, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int weather = 0;
    if (sscanf(params.data(), "%i", &weather) == EOF) {
        console.sendMessage(sender, String("game.weather = " + std::to_string(*core->getConfig().getInt("game.weather"))));
        return;
    }
    core->setWeather(weather);
});

ADD_CONSOLE_CMD(rcon_password, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params.empty()) {
        console.sendMessage(sender, String("rcon.password = \"") + core->getConfig().getString("rcon.password").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::AdminPassword, params);
});

ADD_CONSOLE_CMD(echo, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    console.sendMessage(sender, params);
});

ADD_CONSOLE_CMD(messageslimit, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int value = 0;
    if (sscanf(params.data(), "%i", &value) == EOF) {
        console.sendMessage(sender, String("network.messages_limit = \"") + std::to_string(*core->getConfig().getInt("network.messages_limit")) + "\"");
        return;
    }
    static_cast<IEarlyConfig&>(core->getConfig()).setInt("network.messages_limit", value);
    core->updateNetworks();
});

ADD_CONSOLE_CMD(messageholelimit, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int value = 0;
    if (sscanf(params.data(), "%i", &value) == EOF) {
        console.sendMessage(sender, String("network.message_hole_limit = \"") + std::to_string(*core->getConfig().getInt("network.message_hole_limit")) + "\"");
        return;
    }
    static_cast<IEarlyConfig&>(core->getConfig()).setInt("network.message_hole_limit", value);
    core->updateNetworks();
});

ADD_CONSOLE_CMD(ackslimit, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int value = 0;
    if (sscanf(params.data(), "%i", &value) == EOF) {
        console.sendMessage(sender, String("network.acks_limit = \"") + std::to_string(*core->getConfig().getInt("network.acks_limit")) + "\"");
        return;
    }
    static_cast<IEarlyConfig&>(core->getConfig()).setInt("network.acks_limit", value);
    core->updateNetworks();
});

ADD_CONSOLE_CMD(playertimeout, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int value = 0;
    if (sscanf(params.data(), "%i", &value) == EOF) {
        console.sendMessage(sender, String("network.player_timeout = \"") + std::to_string(*core->getConfig().getInt("network.player_timeout")) + "\"");
        return;
    }
    static_cast<IEarlyConfig&>(core->getConfig()).setInt("network.player_timeout", value);
    core->updateNetworks();
});

ADD_CONSOLE_CMD(rcon, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    if (params == "1") {
        console.sendMessage(sender, "Remote console enabled.");
        static_cast<IEarlyConfig&>(core->getConfig()).setBool("rcon.enable", true);
        core->updateNetworks();
    } else if (params == "0") {
        console.sendMessage(sender, "Remote console disabled.");
        static_cast<IEarlyConfig&>(core->getConfig()).setBool("rcon.enable", false);
        core->updateNetworks();
    } else {
        console.sendMessage(sender, "Unknown parameter. Use rcon 0 to disable remote console or rcon 1 to enable it.");
    }
});

ADD_CONSOLE_CMD(sleep, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    float value = 0.0f;
    if (sscanf(params.data(), "%f", &value) == EOF) {
        console.sendMessage(sender, String("sleep = \"") + std::to_string(*core->getConfig().getFloat("sleep")) + "\"");
        return;
    }
    static_cast<IEarlyConfig&>(core->getConfig()).setFloat("sleep", value);
    core->setThreadSleep(Microseconds(static_cast<long long>(value * 1000.0f)));
});

ADD_CONSOLE_CMD(dynticks, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int value = 0;
    if (sscanf(params.data(), "%i", &value) == EOF) {
        console.sendMessage(sender, String("dynticks = \"") + std::to_string(*core->getConfig().getBool("use_dyn_ticks")) + "\"");
        return;
    }
    static_cast<IEarlyConfig&>(core->getConfig()).setBool("use_dyn_ticks", value);
    core->useDynTicks(bool(value));
});

ADD_CONSOLE_CMD(tickrate, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int value = 0;
    if (sscanf(params.data(), "%i", &value) == EOF) {
        console.sendMessage(sender, String("tickrate = \"") + std::to_string(static_cast<int>(1000.0f / *core->getConfig().getFloat("sleep"))) + "\"");
        return;
    }
    float sleep = 1000.0f / value;
    static_cast<IEarlyConfig&>(core->getConfig()).setFloat("sleep", sleep);
    core->setThreadSleep(Microseconds(static_cast<long long>(sleep * 1000.0f)));
});

ADD_CONSOLE_CMD(worldtime, [](const String& params, const ConsoleCommandSenderData& sender, IConsoleComponent& console, ICore* core) {
    int time;
    if (sscanf(params.data(), "%i", &time) == EOF) {
        console.sendMessage(sender, String("worldtime = \"") + std::to_string(*core->getConfig().getInt("game.time")) + "\"");
        return;
    }
    core->setWorldTime(Hours(time));
});
