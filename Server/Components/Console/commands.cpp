#include "cmd_handler.hpp"

std::unordered_map<std::string, CommandHandlerFuncType> ConsoleCmdHandler::Commands;

ADD_CONSOLE_CMD(gamemodetext, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        console->sendMessage(sender, String("gamemodetext = \"") + core->getConfig().getString("mode_name").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::ModeText, params);
});

ADD_CONSOLE_CMD(hostname, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        console->sendMessage(sender, String("hostname = \"") + core->getConfig().getString("server_name").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::ServerName, params);
});

ADD_CONSOLE_CMD(mapname, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        console->sendMessage(sender, String("mapname = \"") + core->getConfig().getString("map_name").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::MapName, params);
});

ADD_CONSOLE_CMD(weburl, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        console->sendMessage(sender, String("weburl = \"") + core->getConfig().getString("website").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::URL, params);
});

ADD_CONSOLE_CMD(password, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        console->sendMessage(sender, String("password = \"") + core->getConfig().getString("password").data() + "\"");
        return;
    } else if (params == "0") {
        console->sendMessage(sender, "Server password has been removed.");
        core->setData(SettableCoreDataType::Password, "");
        return;
    }
    core->setData(SettableCoreDataType::Password, params);
});

ADD_CONSOLE_CMD(say, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        return;
    }
    core->getPlayers().sendClientMessageToAll(Colour(37, 135, 206), String("* Admin: ") + params);
});

ADD_CONSOLE_CMD(players, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (!core->getPlayers().entries().size()) {
        return;
    }

    console->sendMessage(sender, "ID\tName\tPing\tIP");

    for (IPlayer* player : core->getPlayers().entries()) {
        if (player) {
            std::string ip = "";
            PeerNetworkData data = player->getNetworkData();
            if (!data.networkID.address.ipv6) {
                PeerAddress::AddressString addressString;
                if (PeerAddress::ToString(data.networkID.address, addressString)) {
                    ip = String(addressString.data(), addressString.length());
                }
            }
            String result = String(std::to_string(player->getID()) + "\t" + player->getName().data() + "\t" + std::to_string(player->getPing()) + "\t" + ip);
            console->sendMessage(sender, result);
        }
    }
});

ADD_CONSOLE_CMD(kick, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    int playerId;
    if (sscanf(params.data(), "%i", &playerId) == EOF) {
        return;
    } else if (!core->getPlayers().get(playerId)) {
        return;
    }
    IPlayer* player = core->getPlayers().get(playerId);
    std::string ip = "";
    PeerNetworkData data = player->getNetworkData();
    if (!data.networkID.address.ipv6) {
        PeerAddress::AddressString addressString;
        if (PeerAddress::ToString(data.networkID.address, addressString)) {
            ip = String(addressString.data(), addressString.length());
        }
    }
    console->sendMessage(sender, player->getName().data() + String("<# " + std::to_string(player->getID()) + " - ") + ip + "> has been kicked.");
    player->kick();
});

ADD_CONSOLE_CMD(ban, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    int playerId;
    if (sscanf(params.data(), "%i", &playerId) == EOF) {
        return;
    } else if (!core->getPlayers().get(playerId)) {
        return;
    }
    IPlayer* player = core->getPlayers().get(playerId);
    std::string ip = "";
    PeerNetworkData data = player->getNetworkData();
    if (!data.networkID.address.ipv6) {
        PeerAddress::AddressString addressString;
        if (PeerAddress::ToString(data.networkID.address, addressString)) {
            ip = String(addressString.data(), addressString.length());
        }
    }
    console->sendMessage(sender, player->getName().data() + String("<# " + std::to_string(player->getID()) + " - ") + ip + "> has been banned.");
    player->ban("CONSOLE BAN");
});

ADD_CONSOLE_CMD(banip, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        return;
    }

    const BanEntry banEntry(String(params.data()));
    core->getConfig().addBan(banEntry);
    for (INetwork* network : core->getNetworks()) {
        network->ban(banEntry);
    }
    core->getConfig().writeBans();
    console->sendMessage(sender, String("IP ") + params.data() + String(" has been banned."));
});

ADD_CONSOLE_CMD(unbanip, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        return;
    }

    BanEntry unban(String(params.data()));
    for (INetwork* network : core->getNetworks()) {
        network->unban(unban);
    }

    bool found = false;
    size_t index = 0;
    for (size_t j = core->getConfig().getBansCount(); index < j; index++) {
        const BanEntry& entry = core->getConfig().getBan(index);
        if (entry.address == unban.address) {
            found = true;
            break;
        }
    }

    if (found) {
        core->getConfig().removeBan(index);
        core->getConfig().writeBans();
    }
});

ADD_CONSOLE_CMD(gravity, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    float gravity = 0.008f;
    if (sscanf(params.data(), "%f", &gravity) == EOF) {
        console->sendMessage(sender, String("gravity = " + std::to_string(*core->getConfig().getFloat("gravity"))));
        return;
    }
    core->setGravity(gravity);
});

ADD_CONSOLE_CMD(weather, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    int weather = 0;
    if (sscanf(params.data(), "%i", &weather) == EOF) {
        console->sendMessage(sender, String("weather = " + std::to_string(*core->getConfig().getInt("weather"))));
        return;
    }
    core->setWeather(weather);
});

ADD_CONSOLE_CMD(rcon_password, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    if (params.empty()) {
        console->sendMessage(sender, String("rcon_password = \"") + core->getConfig().getString("rcon_password").data() + "\"");
        return;
    }
    core->setData(SettableCoreDataType::AdminPassword, params);
});

ADD_CONSOLE_CMD(echo, [](const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core) {
    core->logLn(LogLevel::Message, "%s", params.c_str());
});
