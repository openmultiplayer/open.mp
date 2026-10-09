/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"
#include <sstream>
#include <iomanip>

OMP_CAPI(Core_TickCount, uint32_t())
{
	auto tick = ComponentManager::Get()->core->getTickCount();
	return tick;
}

OMP_CAPI(Core_MaxPlayers, int())
{
	int max = *ComponentManager::Get()->core->getConfig().getInt("max_players");
	return max;
}

OMP_CAPI(Core_Log, bool(StringCharPtr text))
{
	ComponentManager::Get()->core->printLn("%s", text);
	return true;
}

OMP_CAPI(Core_IsAdminTeleportAllowed, bool())
{
	bool allowed = *ComponentManager::Get()->core->getConfig().getBool("rcon.allow_teleport");
	return allowed;
}

OMP_CAPI(Core_AllowAdminTeleport, bool(bool allow))
{
	*ComponentManager::Get()->core->getConfig().getBool("rcon.allow_teleport") = allow;
	return true;
}

OMP_CAPI(Core_AreAllAnimationsEnabled, bool())
{
	bool allowed = *ComponentManager::Get()->core->getConfig().getBool("game.use_all_animations");
	return allowed;
}

OMP_CAPI(Core_EnableAllAnimations, bool(bool allow))
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_all_animations") = allow;
	return true;
}

OMP_CAPI(Core_IsAnimationLibraryValid, bool(StringCharPtr name))
{
	auto valid = animationLibraryValid(name, true);
	return valid;
}

OMP_CAPI(Core_AreInteriorWeaponsAllowed, bool())
{
	bool allowed = *ComponentManager::Get()->core->getConfig().getBool("game.allow_interior_weapons");
	return allowed;
}

OMP_CAPI(Core_AllowInteriorWeapons, bool(bool allow))
{
	if (allow)
	{
		*ComponentManager::Get()->core->getConfig().getBool("game.allow_interior_weapons") = true;
	}
	else
	{
		IPlayerPool* players = ComponentManager::Get()->players;
		for (IPlayer* player : players->entries())
		{
			if (player->getInterior() && player->areWeaponsAllowed())
			{
				// Because they are allowed weapons currently this will send a full client reset.
				player->resetWeapons();
			}
		}
		// By the time the player reports having no weapons, this is set and so we remember the old
		// ones still.
		*ComponentManager::Get()->core->getConfig().getBool("game.allow_interior_weapons") = false;
	}
	return true;
}

OMP_CAPI(Core_BlockIpAddress, bool(StringCharPtr ipAddress, int timeMS))
{
	if (strlen(ipAddress) < 1)
	{
		return false;
	}
	BanEntry entry(ipAddress);
	for (INetwork* network : ComponentManager::Get()->core->getNetworks())
	{
		network->ban(entry, Milliseconds(timeMS));
	}
	return true;
}

OMP_CAPI(Core_UnBlockIpAddress, bool(StringCharPtr ipAddress))
{
	BanEntry entry(ipAddress);
	for (INetwork* network : ComponentManager::Get()->core->getNetworks())
	{
		network->unban(entry);
	}
	return true;
}

OMP_CAPI(NPC_Connect, bool(StringCharPtr name, StringCharPtr script))
{
	ComponentManager::Get()->core->connectBot(name, script);
	return true;
}

OMP_CAPI(Core_DisableEntryExitMarkers, bool())
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_entry_exit_markers") = false;
	return true;
}

OMP_CAPI(Core_DisableNameTagsLOS, bool())
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_nametag_los") = false;
	return true;
}

OMP_CAPI(Core_EnableZoneNames, bool(bool enable))
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_zone_names") = enable;
	return true;
}

OMP_CAPI(Core_ShowGameTextForAll, bool(StringCharPtr msg, int time, int style))
{
	if (strlen(msg) < 1)
	{
		return false;
	}
	ComponentManager::Get()->players->sendGameTextToAll(msg, Milliseconds(time), style);
	return true;
}

OMP_CAPI(Core_HideGameTextForAll, bool(int style))
{
	ComponentManager::Get()->players->hideGameTextForAll(style);
	return true;
}

OMP_CAPI(Core_NetworkStats, int(OutputStringBufferPtr output))
{
	std::stringstream stream;
	NetworkStats stats;

	for (INetwork* network : ComponentManager::Get()->core->getNetworks())
	{
		if (network->getNetworkType() == ENetworkType::ENetworkType_RakNetLegacy)
		{
			stats = network->getStatistics();
		}
	}

	stream
		<< "Server Ticks: " << ComponentManager::Get()->core->tickRate() << std::endl
		<< "Messages in Send buffer: " << stats.messageSendBuffer << std::endl
		<< "Messages sent: " << stats.messagesSent << std::endl
		<< "Bytes sent: " << stats.totalBytesSent << std::endl
		<< "Acks sent: " << stats.acknowlegementsSent << std::endl
		<< "Acks in send buffer: " << stats.acknowlegementsPending << std::endl
		<< "Messages waiting for ack: " << stats.messagesOnResendQueue << std::endl
		<< "Messages resent: " << stats.messageResends << std::endl
		<< "Bytes resent: " << stats.messagesTotalBytesResent << std::endl
		<< "Packetloss: " << std::setprecision(1) << std::fixed << stats.packetloss << "%" << std::endl
		<< "Messages received: " << stats.messagesReceived << std::endl
		<< "Bytes received: " << stats.bytesReceived << std::endl
		<< "Acks received: " << stats.acknowlegementsReceived << std::endl
		<< "Duplicate acks received: " << stats.duplicateAcknowlegementsReceived << std::endl
		<< "Inst. KBits per second: " << std::setprecision(1) << (stats.bitsPerSecond / 1000.0) << std::endl
		<< "KBits per second sent: " << std::setprecision(1) << (stats.bpsSent / 1000.0) << std::endl
		<< "KBits per second received: " << std::setprecision(1) << (stats.bpsReceived / 1000.0) << std::endl;

	int len = stream.str().size();
	COPY_STRING_TO_CAPI_STRING_BUFFER(output, stream.str().c_str(), len);
	return len;
}

OMP_CAPI(Player_GetNetworkStats, int(objectPtr player, OutputStringBufferPtr output))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	std::stringstream stream;
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);

	stream
		<< "Network Active: " << int(stats.isActive) << std::endl
		<< "Network State: " << stats.connectMode << std::endl
		<< "Messages in Send buffer: " << stats.messageSendBuffer << std::endl
		<< "Messages sent: " << stats.messagesSent << std::endl
		<< "Bytes sent: " << stats.totalBytesSent << std::endl
		<< "Acks sent: " << stats.acknowlegementsSent << std::endl
		<< "Acks in send buffer: " << stats.acknowlegementsPending << std::endl
		<< "Messages waiting for ack: " << stats.messagesOnResendQueue << std::endl
		<< "Messages resent: " << stats.messageResends << std::endl
		<< "Bytes resent: " << stats.messagesTotalBytesResent << std::endl
		<< "Packetloss: " << std::setprecision(1) << std::fixed << stats.packetloss << "%" << std::endl
		<< "Messages received: " << stats.messagesReceived << std::endl
		<< "Bytes received: " << stats.bytesReceived << std::endl
		<< "Acks received: " << stats.acknowlegementsReceived << std::endl
		<< "Duplicate acks received: " << stats.duplicateAcknowlegementsReceived << std::endl
		<< "Inst. KBits per second: " << std::setprecision(1) << (stats.bitsPerSecond / 1000.0) << std::endl
		<< "KBits per second sent: " << std::setprecision(1) << (stats.bpsSent / 1000.0) << std::endl
		<< "KBits per second received: " << std::setprecision(1) << (stats.bpsReceived / 1000.0) << std::endl;

	int len = stream.str().size();
	COPY_STRING_TO_CAPI_STRING_BUFFER(output, stream.str().c_str(), len);
	return len;
}

OMP_CAPI(Core_ServerTickRate, int())
{
	int tick = ComponentManager::Get()->core->tickRate();
	return tick;
}

OMP_CAPI(Core_GetWeaponName, bool(int weaponid, OutputStringViewPtr output))
{
	SET_CAPI_STRING_VIEW(output, ComponentManager::Get()->core->getWeaponName(PlayerWeapon(weaponid)));
	return true;
}

OMP_CAPI(Core_SetChatRadius, bool(float globalChatRadius))
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_chat_radius") = true;
	*ComponentManager::Get()->core->getConfig().getFloat("game.chat_radius") = globalChatRadius;
	return true;
}

OMP_CAPI(Core_SetMarkerRadius, bool(float playerMarkerRadius))
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_player_marker_draw_radius") = true;
	*ComponentManager::Get()->core->getConfig().getFloat("game.player_marker_draw_radius") = playerMarkerRadius;
	return true;
}

OMP_CAPI(Player_NetStatsBytesReceived, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	int bytes = stats.bytesReceived;
	return bytes;
}

OMP_CAPI(Player_NetStatsBytesSent, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	int bytes = stats.totalBytesSent;
	return bytes;
}

OMP_CAPI(Player_NetStatsConnectionStatus, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	int status = stats.connectMode;
	return status;
}

OMP_CAPI(Player_NetStatsGetConnectedTime, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	int ms = stats.connectionElapsedTime;
	return ms;
}

OMP_CAPI(Player_NetStatsGetIpPort, bool(objectPtr player, OutputStringBufferPtr output))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PeerNetworkData data = player_->getNetworkData();
	PeerAddress::AddressString addressString;
	if (PeerAddress::ToString(data.networkID.address, addressString))
	{
		Impl::String ip_port((StringView(addressString)));
		ip_port += ":";
		ip_port += std::to_string(data.networkID.port);
		int len = ip_port.length();
		COPY_STRING_TO_CAPI_STRING_BUFFER(output, ip_port.c_str(), len);
		return len;
	}
	return 0;
}

OMP_CAPI(Player_NetStatsMessagesReceived, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	int received = stats.messagesReceived;
	return received;
}

OMP_CAPI(Player_NetStatsMessagesRecvPerSecond, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	int received = stats.messagesReceivedPerSecond;
	return received;
}

OMP_CAPI(Player_NetStatsMessagesSent, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	int sent = stats.messagesSent;
	return sent;
}

OMP_CAPI(Player_NetStatsPacketLossPercent, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	NetworkStats stats = player_->getNetworkData().network->getStatistics(player_);
	auto packetLoss = stats.packetloss;
	return packetLoss;
}

OMP_CAPI(Core_SendRconCommand, bool(StringCharPtr command))
{
	IConsoleComponent* console = ComponentManager::Get()->console;
	if (console)
	{
		console->send(command);
		return true;
	}
	return false;
}

OMP_CAPI(Core_SetDeathDropAmount, bool(int amount))
{
	*ComponentManager::Get()->core->getConfig().getInt("game.death_drop_amount") = amount;
	return true;
}

OMP_CAPI(Core_GameMode_SetText, bool(StringCharPtr string))
{
	ComponentManager::Get()->core->setData(SettableCoreDataType::ModeText, string);
	return true;
}

OMP_CAPI(Core_SetGravity, bool(float gravity))
{
	ComponentManager::Get()->core->setGravity(gravity);
	return true;
}

OMP_CAPI(Core_GetGravity, float())
{
	float gravity = ComponentManager::Get()->core->getGravity();
	return gravity;
}

OMP_CAPI(Core_SetNameTagsDrawDistance, bool(float distance))
{
	*ComponentManager::Get()->core->getConfig().getFloat("game.nametag_draw_radius") = distance;
	return true;
}

OMP_CAPI(Core_SetWeather, bool(int weatherid))
{
	ComponentManager::Get()->core->setWeather(weatherid);
	return true;
}

OMP_CAPI(Core_SetWorldTime, bool(int hour))
{
	ComponentManager::Get()->core->setWorldTime(Hours(hour));
	return true;
}

OMP_CAPI(Core_ShowNameTags, bool(bool show))
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_nametags") = show;
	return true;
}

OMP_CAPI(Core_ShowPlayerMarkers, bool(int mode))
{
	*ComponentManager::Get()->core->getConfig().getInt("game.player_marker_mode") = mode;
	return true;
}

OMP_CAPI(Core_UsePedAnims, bool())
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_player_ped_anims") = true;
	return true;
}

OMP_CAPI(Core_GetWeather, int())
{
	int weather = *ComponentManager::Get()->core->getConfig().getInt("game.weather");
	return weather;
}

OMP_CAPI(Core_GetWorldTime, int())
{
	int hour = *ComponentManager::Get()->core->getConfig().getInt("game.time");
	return hour;
}

OMP_CAPI(Core_ToggleChatTextReplacement, bool(bool enable))
{
	*ComponentManager::Get()->core->getConfig().getBool("chat_input_filter") = enable;
	return true;
}

OMP_CAPI(Core_IsChatTextReplacementToggled, bool())
{
	bool toggled = *ComponentManager::Get()->core->getConfig().getBool("chat_input_filter");
	return toggled;
}

OMP_CAPI(Core_IsNickNameValid, bool(StringCharPtr name))
{
	auto valid = ComponentManager::Get()->players->isNameValid(name);
	return valid;
}

OMP_CAPI(Core_AllowNickNameCharacter, bool(int character, bool allow))
{
	ComponentManager::Get()->players->allowNickNameCharacter(character, allow);
	return true;
}

OMP_CAPI(Core_IsNickNameCharacterAllowed, bool(int character))
{
	bool allowed = ComponentManager::Get()->players->isNickNameCharacterAllowed(character);
	return allowed;
}

OMP_CAPI(Core_ClearBanList, bool())
{
	ICore* core = ComponentManager::Get()->core;
	if (!core)
	{
		return false;
	}

	core->getConfig().clearBans();
	return true;
}

OMP_CAPI(Core_IsIpAddressBanned, bool(StringCharPtr ip))
{
	ICore* core = ComponentManager::Get()->core;
	if (!core)
	{
		return false;
	}
	BanEntry entry(ip);
	bool isBanned = core->getConfig().isBanned(entry);
	return isBanned;
}

OMP_CAPI(Core_GetWeaponSlot, int(uint8_t weapon))
{
	int slot = WeaponSlotData { weapon }.slot();
	return slot;
}

OMP_CAPI(Core_AddRule, bool(StringCharPtr name, StringCharPtr value))
{
	ICore* core = ComponentManager::Get()->core;
	if (!core)
	{
		return false;
	}

	for (INetwork* network : core->getNetworks())
	{
		INetworkQueryExtension* query = queryExtension<INetworkQueryExtension>(network);

		if (query)
		{
			query->addRule(name, value);
			return true;
		}
	}
	return false;
}

OMP_CAPI(Core_IsValidRule, bool(StringCharPtr name))
{
	ICore* core = ComponentManager::Get()->core;
	if (!core)
	{
		return false;
	}

	for (INetwork* network : core->getNetworks())
	{
		INetworkQueryExtension* query = queryExtension<INetworkQueryExtension>(network);

		if (query)
		{
			bool valid = query->isValidRule(name);
			return valid;
		}
	}
	return false;
}

OMP_CAPI(Core_RemoveRule, bool(StringCharPtr name))
{
	ICore* core = ComponentManager::Get()->core;
	if (!core)
	{
		return false;
	}

	for (INetwork* network : core->getNetworks())
	{
		INetworkQueryExtension* query = queryExtension<INetworkQueryExtension>(network);

		if (query)
		{
			query->removeRule(name);
			return true;
		}
	}
	return false;
}
