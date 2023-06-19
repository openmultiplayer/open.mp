/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "cmd_handler.hpp"

FlatHashMap<String, CommandHandlerFuncType> ConsoleCmdHandler::Commands;

ADD_CONSOLE_CMD(cmdlist, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		FlatHashSet<StringView> commands;

		console.sendMessage(sender, "Console commands:");
		for (auto& kv : ConsoleCmdHandler::Commands)
		{
			commands.emplace(kv.first);
		}

		console.defEventDispatcher().all(
			[&commands](ConsoleEventHandler* handler)
			{
				handler->onConsoleCommandListRequest(commands);
			});

		for (auto kv : commands)
		{
			console.sendMessage(sender, kv);
		}
	});

ADD_CONSOLE_CMD(password, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		if (params.empty())
		{
			console.sendMessage(sender, String("password = \"") + core->getConfig().getString("password").data() + "\"");
			return;
		}
		else if (params == "0")
		{
			console.sendMessage(sender, "Server password has been removed.");
			core->setData(SettableCoreDataType::Password, "");
			return;
		}
		core->setData(SettableCoreDataType::Password, params);
		console.sendMessage(sender, "Setting server password to: \"" + params + "\"");
	});

ADD_CONSOLE_CMD(say, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		if (params.empty())
		{
			return;
		}
		core->getPlayers().sendClientMessageToAll(Colour(37, 135, 206), String("* Admin: ") + params);
	});

ADD_CONSOLE_CMD(players, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		if (!core->getPlayers().entries().size())
		{
			return;
		}

		console.sendMessage(sender, "ID\tName\tPing\tIP");

		for (IPlayer* player : core->getPlayers().entries())
		{
			if (player)
			{
				String ip = "";
				PeerNetworkData data = player->getNetworkData();
				if (!data.networkID.address.ipv6)
				{
					PeerAddress::AddressString addressString;
					if (PeerAddress::ToString(data.networkID.address, addressString))
					{
						ip = String(addressString.data(), addressString.length());
					}
				}
				String result = String(std::to_string(player->getID()) + "\t" + player->getName().data() + "\t" + std::to_string(player->getPing()) + "\t" + ip);
				console.sendMessage(sender, result);
			}
		}
	});

ADD_CONSOLE_CMD(kick, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		int playerId;
		if (sscanf(params.data(), "%d", &playerId) == EOF)
		{
			return;
		}
		else if (!core->getPlayers().get(playerId))
		{
			return;
		}
		IPlayer* player = core->getPlayers().get(playerId);
		String ip = "";
		PeerNetworkData data = player->getNetworkData();
		if (!data.networkID.address.ipv6)
		{
			PeerAddress::AddressString addressString;
			if (PeerAddress::ToString(data.networkID.address, addressString))
			{
				ip = String(addressString.data(), addressString.length());
			}
		}
		console.sendMessage(sender, player->getName().data() + String("<# " + std::to_string(player->getID()) + " - ") + ip + "> has been kicked.");
		player->kick();
	});

ADD_CONSOLE_CMD(ban, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		int playerId;
		if (sscanf(params.data(), "%d", &playerId) == EOF)
		{
			return;
		}
		else if (!core->getPlayers().get(playerId))
		{
			return;
		}
		IPlayer* player = core->getPlayers().get(playerId);
		String ip = "";
		PeerNetworkData data = player->getNetworkData();
		if (!data.networkID.address.ipv6)
		{
			PeerAddress::AddressString addressString;
			if (PeerAddress::ToString(data.networkID.address, addressString))
			{
				ip = String(addressString.data(), addressString.length());
			}
		}
		console.sendMessage(sender, player->getName().data() + String("<# " + std::to_string(player->getID()) + " - ") + ip + "> has been banned.");
		player->ban("CONSOLE BAN");
	});

ADD_CONSOLE_CMD(banip, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		if (params.empty())
		{
			return;
		}

		const BanEntry banEntry(String(params.data()));
		core->getConfig().addBan(banEntry);
		for (INetwork* network : core->getNetworks())
		{
			network->ban(banEntry);
		}
		core->getConfig().writeBans();
		console.sendMessage(sender, String("IP ") + params.data() + String(" has been banned."));
	});

ADD_CONSOLE_CMD(reloadbans, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		core->getConfig().reloadBans();
		console.sendMessage(sender, "Banlist reloded.");
	});

ADD_CONSOLE_CMD(unbanip, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		if (params.empty())
		{
			return;
		}

		BanEntry unban(String(params.data()));
		for (INetwork* network : core->getNetworks())
		{
			network->unban(unban);
		}

		core->getConfig().removeBan(unban);
	});

ADD_CONSOLE_CMD(gravity, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		float gravity = 0.008f;
		if (sscanf(params.data(), "%f", &gravity) == EOF)
		{
			console.sendMessage(sender, String("game.gravity = " + std::to_string(*core->getConfig().getFloat("game.gravity"))));
			return;
		}
		core->setGravity(gravity);
	});

ADD_CONSOLE_CMD(weather, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		int weather = 0;
		if (sscanf(params.data(), "%d", &weather) == EOF)
		{
			console.sendMessage(sender, String("game.weather = " + std::to_string(*core->getConfig().getInt("game.weather"))));
			return;
		}
		core->setWeather(weather);
	});

ADD_CONSOLE_CMD(echo, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		console.sendMessage(sender, params);
	});

ADD_CONSOLE_CMD(sleep, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		float value = 0.0f;
		if (sscanf(params.data(), "%f", &value) == EOF)
		{
			console.sendMessage(sender, String("sleep = \"") + std::to_string(*core->getConfig().getFloat("sleep")) + "\"");
			return;
		}
		*core->getConfig().getFloat("sleep") = value;
		core->setThreadSleep(Microseconds(static_cast<long long>(value * 1000.0f)));
	});

ADD_CONSOLE_CMD(dynticks, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		int value = 0;
		if (sscanf(params.data(), "%d", &value) == EOF)
		{
			console.sendMessage(sender, String("dynticks = \"") + std::to_string(*core->getConfig().getBool("use_dyn_ticks")) + "\"");
			return;
		}
		*core->getConfig().getBool("use_dyn_ticks") = value;
		core->useDynTicks(bool(value));
	});

ADD_CONSOLE_CMD(tickrate, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		int value = 0;
		if (sscanf(params.data(), "%d", &value) == EOF)
		{
			console.sendMessage(sender, String("tickrate = \"") + std::to_string(static_cast<int>(1000.0f / *core->getConfig().getFloat("sleep"))) + "\"");
			return;
		}
		float sleep = 1000.0f / value;

		*core->getConfig().getFloat("sleep") = sleep;
		core->setThreadSleep(Microseconds(static_cast<long long>(sleep * 1000.0f)));
	});

ADD_CONSOLE_CMD(worldtime, [](const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core)
	{
		int time;
		if (sscanf(params.data(), "%d", &time) == EOF)
		{
			console.sendMessage(sender, String("worldtime = \"") + std::to_string(*core->getConfig().getInt("game.time")) + "\"");
			return;
		}
		core->setWorldTime(Hours(time));
	});
