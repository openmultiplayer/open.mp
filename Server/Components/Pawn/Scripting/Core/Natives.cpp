/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "sdk.hpp"
#include <iostream>
#define _USE_MATH_DEFINES
#include "../Types.hpp"
#include "../../format.hpp"
#include <Impl/network_impl.hpp>
#include <iomanip>
#include <math.h>
#include <sstream>
#include <anim.hpp>

SCRIPT_API(GetTickCount, int())
{
	return PawnManager::Get()->core->getTickCount();
}

SCRIPT_API(GetMaxPlayers, int())
{
	return *PawnManager::Get()->core->getConfig().getInt("max_players");
}

SCRIPT_API(VectorSize, float(Vector3 vector))
{
	return glm::length(vector);
}

SCRIPT_API(asin, float(float value))
{
	return std::asin(value) * 180 / M_PI;
}

SCRIPT_API(acos, float(float value))
{
	return std::acos(value) * 180 / M_PI;
}

SCRIPT_API(atan, float(float value))
{
	return std::atan(value) * 180 / M_PI;
}

SCRIPT_API(atan2, float(float y, float x))
{
	return std::atan2(y, x) * 180 / M_PI;
}

SCRIPT_API(floatstr, float(std::string const& string))
{
	try
	{
		return std::stof(string);
	}
	catch (std::exception e)
	{
		return 0.0f;
	}
}

SCRIPT_API(GetPlayerPoolSize, int())
{
	int highestID = -1;
	for (IPlayer* player : PawnManager::Get()->players->entries())
	{
		if (highestID < player->getID())
		{
			highestID = player->getID();
		}
	}
	return highestID;
}

SCRIPT_API(GetVehiclePoolSize, int())
{
	IVehiclesComponent* vehicles = PawnManager::Get()->vehicles;
	if (vehicles)
	{
		int highestID = -1;
		for (IVehicle* vehicle : *vehicles)
		{
			if (highestID < vehicle->getID())
			{
				highestID = vehicle->getID();
			}
		}
		return highestID;
	}
	return -1;
}

SCRIPT_API(GetActorPoolSize, int())
{
	IActorsComponent* actors = PawnManager::Get()->actors;
	if (actors)
	{
		int highestID = -1;
		for (IActor* actor : *actors)
		{
			if (highestID < actor->getID())
			{
				highestID = actor->getID();
			}
		}
		return highestID;
	}
	return -1;
}

SCRIPT_API(GetPlayers, int(DynamicArray<int>& outputPlayers))
{
	int index = -1;
	IPlayerPool* players = PawnManager::Get()->players;
	if (outputPlayers.size() < players->entries().size())
	{
		PawnManager::Get()->core->printLn(
			"There are %zu players in your server but array size used in `GetPlayers` is %zu; Use a bigger size in your script.",
			players->entries().size(),
			outputPlayers.size());
	}

	for (IPlayer* player : players->entries())
	{
		index++;
		if (index >= outputPlayers.size())
		{
			break;
		}
		outputPlayers[index] = player->getID();
	}
	return index + 1;
}

SCRIPT_API(GetActors, int(DynamicArray<int>& outputActors))
{
	int index = -1;
	IActorsComponent* actors = PawnManager::Get()->actors;
	if (actors)
	{
		if (outputActors.size() < actors->count())
		{
			PawnManager::Get()->core->printLn(
				"There are %zu actors in your server but array size used in `GetActors` is %zu; Use a bigger size in your script.",
				actors->count(),
				outputActors.size());
		}

		for (IActor* actor : *actors)
		{
			index++;
			if (index >= outputActors.size())
			{
				break;
			}
			outputActors[index] = actor->getID();
		}
	}
	return index + 1;
}

SCRIPT_API(GetVehicles, int(DynamicArray<int>& outputVehicles))
{
	int index = -1;
	IVehiclesComponent* vehicles = PawnManager::Get()->vehicles;
	if (vehicles)
	{
		if (outputVehicles.size() < vehicles->count())
		{
			PawnManager::Get()->core->printLn(
				"There are %zu vehicles in your server but array size used in `GetVehicles` is %zu; Use a bigger size in your script.",
				vehicles->count(),
				outputVehicles.size());
		}

		for (IVehicle* vehicle : *vehicles)
		{
			index++;
			if (index >= outputVehicles.size())
			{
				break;
			}
			outputVehicles[index] = vehicle->getID();
		}
	}
	return index + 1;
}

SCRIPT_API(print, bool(const std::string& text))
{
	PawnManager::Get()->core->printLn("%s", text.c_str());
	return false;
}

SCRIPT_API(IsAdminTeleportAllowed, bool())
{
	return *PawnManager::Get()->config->getBool("rcon.allow_teleport");
}

SCRIPT_API(AllowAdminTeleport, bool(bool allow))
{
	*PawnManager::Get()->config->getBool("rcon.allow_teleport") = allow;
	return true;
}

SCRIPT_API(AreAllAnimationsEnabled, bool())
{
	return *PawnManager::Get()->config->getBool("game.use_all_animations");
}

SCRIPT_API(EnableAllAnimations, bool(bool allow))
{
	*PawnManager::Get()->config->getBool("game.use_all_animations") = allow;
	return true;
}

SCRIPT_API(IsValidAnimationLibrary, bool(std::string const& name))
{
	cell* args = GetParams();
	return animationLibraryValid(name, args[0] == 1 * sizeof(cell) || args[2]);
}

SCRIPT_API(AreInteriorWeaponsAllowed, bool())
{
	return *PawnManager::Get()->config->getBool("game.allow_interior_weapons");
}

SCRIPT_API(AllowInteriorWeapons, bool(bool allow))
{
	if (allow)
	{
		*PawnManager::Get()->config->getBool("game.allow_interior_weapons") = true;
	}
	else
	{
		IPlayerPool* players = PawnManager::Get()->players;
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
		*PawnManager::Get()->config->getBool("game.allow_interior_weapons") = false;
	}
	return true;
}

SCRIPT_API(BlockIpAddress, bool(std::string const& ipAddress, int timeMS))
{
	if (ipAddress.empty())
	{
		return false;
	}
	BanEntry entry(ipAddress);
	for (INetwork* network : PawnManager::Get()->core->getNetworks())
	{
		network->ban(entry, Milliseconds(timeMS));
	}
	return true;
}

SCRIPT_API(UnBlockIpAddress, bool(std::string const& ipAddress))
{
	if (ipAddress.empty())
	{
		return false;
	}
	BanEntry entry(ipAddress);
	for (INetwork* network : PawnManager::Get()->core->getNetworks())
	{
		network->unban(entry);
	}
	return true;
}

SCRIPT_API(ConnectNPC, bool(std::string const& name, std::string const& script))
{
	PawnManager::Get()->core->connectBot(name, script);
	return true;
}

SCRIPT_API(DisableInteriorEnterExits, bool())
{
	*PawnManager::Get()->config->getBool("game.use_entry_exit_markers") = false;
	return true;
}

SCRIPT_API(DisableNameTagLOS, bool())
{
	*PawnManager::Get()->config->getBool("game.use_nametag_los") = false;
	return true;
}

SCRIPT_API(EnableTirePopping, bool(bool enable))
{
	PawnManager::Get()->core->logLn(LogLevel::Warning, "EnableTirePopping() function is removed.");
	return true;
}

SCRIPT_API(EnableZoneNames, bool(bool enable))
{
	*PawnManager::Get()->config->getBool("game.use_zone_names") = enable;
	return true;
}

SCRIPT_API(GameModeExit, bool())
{
	PawnManager::Get()->EndMainScript();
	return true;
}

SCRIPT_API(GameTextForAll, bool(cell const* format, int time, int style))
{
	AmxStringFormatter msg(format, GetAMX(), GetParams(), 3);
	if (msg.empty())
	{
		return false;
	}
	PawnManager::Get()->players->sendGameTextToAll(msg, Milliseconds(time), style);
	return true;
}

SCRIPT_API(HideGameTextForAll, bool(int style))
{
	PawnManager::Get()->players->hideGameTextForAll(style);
	return true;
}

SCRIPT_API(GameTextForAllf, bool(int time, int style, cell const* format))
{
	AmxStringFormatter msg(format, GetAMX(), GetParams(), 3);
	if (msg.empty())
	{
		return false;
	}
	PawnManager::Get()->players->sendGameTextToAll(msg, Milliseconds(time), style);
	return true;
}

int getConfigOptionAsInt(std::string const& cvar)
{
	IConfig* config = PawnManager::Get()->config;
	auto res = config->getNameFromAlias(cvar);
	bool* v0 = nullptr;
	int* v1 = nullptr;
	if (!res.second.empty())
	{
		if (res.first)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Deprecated console variable \"%s\", use \"%.*s\" instead.", cvar.c_str(), PRINT_VIEW(res.second));
		}
		if (!(v1 = config->getInt(res.second)))
		{
			v0 = config->getBool(res.second);
		}
	}
	else
	{
		if (!(v1 = config->getInt(cvar)))
		{
			v0 = config->getBool(cvar);
		}
	}
	if (v1)
	{
		return *v1;
	}
	else if (v0)
	{
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Boolean console variable \"%s\" retreived as integer.", cvar.c_str());
		return *v0;
	}
	else
	{
		return 0;
	}
}

bool getConfigOptionAsBool(std::string const& cvar)
{
	IConfig* config = PawnManager::Get()->config;
	auto res = config->getNameFromAlias(cvar);
	bool* v0 = nullptr;
	int* v1 = nullptr;
	if (!res.second.empty())
	{
		if (res.first)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Deprecated console variable \"%s\", use \"%.*s\" instead.", cvar.c_str(), PRINT_VIEW(res.second));
		}
		if (!(v0 = config->getBool(res.second)))
		{
			v1 = config->getInt(res.second);
		}
	}
	else
	{
		if (!(v0 = config->getBool(cvar)))
		{
			v1 = config->getInt(cvar);
		}
	}
	if (v0)
	{
		return *v0;
	}
	else if (v1)
	{
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Integer console variable \"%s\" retreived as boolean.", cvar.c_str());
		return *v1 != 0;
	}
	else
	{
		return false;
	}
}

float getConfigOptionAsFloat(std::string const& cvar)
{
	IConfig* config = PawnManager::Get()->config;
	auto res = config->getNameFromAlias(cvar);
	float* var = nullptr;
	if (!res.second.empty())
	{
		if (res.first)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Deprecated console variable \"%s\", use \"%.*s\" instead.", cvar.c_str(), PRINT_VIEW(res.second));
		}
		var = config->getFloat(res.second);
	}
	else
	{
		var = config->getFloat(cvar);
	}
	if (var)
	{
		return *var;
	}
	else
	{
		return 0.0f;
	}
}

int getConfigOptionAsString(std::string const& cvar, OutputOnlyString& buffer)
{
	// Special case, converting `gamemode0` to `pawn.main_scripts[0]`.  It is the only string to
	// array change.
	IConfig* config = PawnManager::Get()->config;
	bool gm = cvar.substr(0, 8) == "gamemode";
	auto res = config->getNameFromAlias(gm ? "gamemode" : cvar);
	if (!res.second.empty())
	{
		if (res.first)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Deprecated console variable \"%s\", use \"%.*s\" instead.", cvar.c_str(), PRINT_VIEW(res.second));
		}
		if (gm)
		{
			size_t i = std::stoi("0" + cvar.substr(8));
			DynamicArray<StringView> mainScripts(i + 1);
			size_t n = config->getStrings(res.second, Span<StringView>(mainScripts.data(), mainScripts.size()));
			if (i < n)
			{
				buffer = mainScripts[i];
			}
		}
		else
		{
			buffer = config->getString(res.second);
		}
	}
	else
	{
		buffer = config->getString(cvar);
	}
	return std::get<StringView>(buffer).length();
}

SCRIPT_API(GetConsoleVarAsBool, bool(std::string const& cvar))
{
	return getConfigOptionAsBool(cvar);
}

SCRIPT_API(GetConsoleVarAsInt, int(std::string const& cvar))
{
	return getConfigOptionAsInt(cvar);
}

SCRIPT_API(GetConsoleVarAsFloat, float(std::string const& cvar))
{
	return getConfigOptionAsFloat(cvar);
}

SCRIPT_API(GetConsoleVarAsString, int(std::string const& cvar, OutputOnlyString& buffer))
{
	return getConfigOptionAsString(cvar, buffer);
}

SCRIPT_API(GetNetworkStats, bool(OutputOnlyString& output))
{
	std::stringstream stream;
	NetworkStats stats;

	for (INetwork* network : PawnManager::Get()->core->getNetworks())
	{
		if (network->getNetworkType() == ENetworkType::ENetworkType_RakNetLegacy)
		{
			stats = network->getStatistics();
		}
	}

	stream
		<< "Server Ticks: " << PawnManager::Get()->core->tickRate() << std::endl
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

	// Scope-allocated string, copy it
	output = stream.str();
	return true;
}

SCRIPT_API(GetPlayerNetworkStats, bool(IPlayer& player, OutputOnlyString& output))
{
	std::stringstream stream;
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);

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

	// Scope-allocated string, copy it
	output = stream.str();
	return true;
}

SCRIPT_API(GetServerTickRate, int())
{
	return PawnManager::Get()->core->tickRate();
}

SCRIPT_API(GetServerVarAsBool, bool(std::string const& cvar))
{
	return getConfigOptionAsBool(cvar);
}

SCRIPT_API(GetServerVarAsInt, int(std::string const& cvar))
{
	return getConfigOptionAsInt(cvar);
}

SCRIPT_API(GetServerVarAsFloat, float(std::string const& cvar))
{
	return getConfigOptionAsFloat(cvar);
}

SCRIPT_API(GetServerVarAsString, int(std::string const& cvar, OutputOnlyString& buffer))
{
	return getConfigOptionAsString(cvar, buffer);
}

SCRIPT_API(GetWeaponName, int(int weaponid, OutputOnlyString& weapon))
{
	weapon = PawnManager::Get()->core->getWeaponName(PlayerWeapon(weaponid));
	return std::get<StringView>(weapon).length();
}

SCRIPT_API(LimitGlobalChatRadius, bool(float chatRadius))
{
	*PawnManager::Get()->config->getBool("game.use_chat_radius") = true;
	*PawnManager::Get()->config->getFloat("game.chat_radius") = chatRadius;
	return true;
}

SCRIPT_API(LimitPlayerMarkerRadius, bool(float markerRadius))
{
	*PawnManager::Get()->config->getBool("game.use_player_marker_draw_radius") = true;
	*PawnManager::Get()->config->getFloat("game.player_marker_draw_radius") = markerRadius;
	return true;
}

SCRIPT_API(NetStats_BytesReceived, int(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.bytesReceived;
}

SCRIPT_API(NetStats_BytesSent, int(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.totalBytesSent;
}

SCRIPT_API(NetStats_ConnectionStatus, int(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.connectMode;
}

SCRIPT_API(NetStats_GetConnectedTime, int(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.connectionElapsedTime;
}

SCRIPT_API_FAILRET(NetStats_GetIpPort, -1, int(IPlayer& player, OutputOnlyString& output))
{
	PeerNetworkData data = player.getNetworkData();
	PeerAddress::AddressString addressString;
	if (PeerAddress::ToString(data.networkID.address, addressString))
	{
		String ip_port((StringView(addressString)));
		ip_port += ":";
		ip_port += std::to_string(data.networkID.port);
		// Scope-allocated string, copy it
		output = ip_port;
		return std::get<String>(output).length();
	}
	return FailRet;
}

SCRIPT_API(NetStats_MessagesReceived, int(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.messagesReceived;
}

SCRIPT_API(NetStats_MessagesRecvPerSecond, int(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.messagesReceivedPerSecond;
}

SCRIPT_API(NetStats_MessagesSent, int(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.messagesSent;
}

SCRIPT_API(NetStats_PacketLossPercent, float(IPlayer& player))
{
	NetworkStats stats = player.getNetworkData().network->getStatistics(&player);
	return stats.packetloss;
}

SCRIPT_API(SendPlayerMessageToAll, bool(IPlayer& sender, cell const* format))
{
	AmxStringFormatter message(format, GetAMX(), GetParams(), 2);
	PawnManager::Get()->players->sendChatMessageToAll(sender, message);
	return true;
}

SCRIPT_API(SendPlayerMessageToAllf, bool(IPlayer& sender, cell const* format))
{
	AmxStringFormatter message(format, GetAMX(), GetParams(), 2);
	PawnManager::Get()->players->sendChatMessageToAll(sender, message);
	return true;
}

SCRIPT_API(SendRconCommand, bool(cell const* format))
{
	IConsoleComponent* console = PawnManager::Get()->console;
	if (console)
	{
		AmxStringFormatter command(format, GetAMX(), GetParams(), 1);
		if (command.empty())
		{
			return false;
		}
		console->send(command);
	}
	return true;
}

SCRIPT_API(SendRconCommandf, bool(cell const* format))
{
	IConsoleComponent* console = PawnManager::Get()->console;
	if (console)
	{
		AmxStringFormatter command(format, GetAMX(), GetParams(), 1);
		if (command.empty())
		{
			return false;
		}
		console->send(command);
	}
	return true;
}

SCRIPT_API(SetDeathDropAmount, bool(int amount))
{
	*PawnManager::Get()->config->getInt("game.death_drop_amount") = amount;
	return true;
}

SCRIPT_API(SetGameModeText, bool(cell const* format))
{
	AmxStringFormatter string(format, GetAMX(), GetParams(), 1);
	if (string.empty())
	{
		return false;
	}
	PawnManager::Get()->core->setData(SettableCoreDataType::ModeText, string);
	return true;
}

SCRIPT_API(SetGravity, bool(float gravity))
{
	PawnManager::Get()->core->setGravity(gravity);
	return true;
}

SCRIPT_API(GetGravity, float())
{
	return PawnManager::Get()->core->getGravity();
}

SCRIPT_API(SetNameTagDrawDistance, bool(float distance))
{
	*PawnManager::Get()->config->getFloat("game.nametag_draw_radius") = distance;
	return true;
}

SCRIPT_API(SetTeamCount, bool(int count))
{
	PawnManager::Get()->core->logLn(LogLevel::Warning, "SetTeamCount() function is removed.");
	return true;
}

SCRIPT_API(SetWeather, bool(int weatherid))
{
	PawnManager::Get()->core->setWeather(weatherid);
	return true;
}

SCRIPT_API(SetWorldTime, bool(int hour))
{
	PawnManager::Get()->core->setWorldTime(Hours(hour));
	return true;
}

SCRIPT_API(SHA256_PassHash, int(std::string const& password, std::string const& salt, OutputOnlyString& output))
{
	PawnManager::Get()->core->logLn(LogLevel::Warning, "Using unsafe hashing function SHA256_PassHash");

	// Scope-allocated string, copy it
	StaticArray<char, 64 + 1> hash;
	bool res = PawnManager::Get()->core->sha256(password, salt, hash);
	if (res)
	{
		output = String(hash.data());
		return hash.size();
	}
	output = String("");
	return 0;
}

SCRIPT_API(ShowNameTags, bool(bool show))
{
	*PawnManager::Get()->config->getBool("game.use_nametags") = show;
	return true;
}

SCRIPT_API(ShowPlayerMarkers, bool(int mode))
{
	*PawnManager::Get()->config->getInt("game.player_marker_mode") = mode;
	return true;
}

SCRIPT_API(UsePlayerPedAnims, bool())
{
	*PawnManager::Get()->config->getBool("game.use_player_ped_anims") = true;
	return true;
}

SCRIPT_API(GetWeather, int())
{
	return *PawnManager::Get()->config->getInt("game.weather");
}

SCRIPT_API(GetWorldTime, int())
{
	return *PawnManager::Get()->config->getInt("game.time");
}

SCRIPT_API(ToggleChatTextReplacement, bool(bool enable))
{
	*PawnManager::Get()->config->getBool("chat_input_filter") = enable;
	return true;
}

SCRIPT_API(ChatTextReplacementToggled, bool())
{
	return *PawnManager::Get()->config->getBool("chat_input_filter");
}

SCRIPT_API(IsValidNickName, bool(const std::string& name))
{
	return PawnManager::Get()->players->isNameValid(name);
}

SCRIPT_API(AllowNickNameCharacter, bool(int character, bool allow))
{
	PawnManager::Get()->players->allowNickNameCharacter(character, allow);
	return true;
}

SCRIPT_API(IsNickNameCharacterAllowed, bool(char character))
{
	return PawnManager::Get()->players->isNickNameCharacterAllowed(character);
}

SCRIPT_API(ClearBanList, bool())
{
	ICore* core = PawnManager::Get()->core;
	if (!core)
	{
		return false;
	}

	core->getConfig().clearBans();
	return true;
}

SCRIPT_API(IsBanned, bool(const std::string& ip))
{
	ICore* core = PawnManager::Get()->core;
	if (!core)
	{
		return false;
	}
	BanEntry entry(ip);
	return core->getConfig().isBanned(entry);
}

SCRIPT_API(GetWeaponSlot, int(uint8_t weapon))
{
	return WeaponSlotData { weapon }.slot();
}

bool addRule(AMX* amx, cell* params, const std::string& name, cell const* format)
{
	ICore* core = PawnManager::Get()->core;
	if (!core)
	{
		return false;
	}

	AmxStringFormatter value(format, amx, params, 2);
	for (INetwork* network : core->getNetworks())
	{
		INetworkQueryExtension* query = queryExtension<INetworkQueryExtension>(network);

		if (query)
		{
			return query->addRule(name, value);
		}
	}
	return false;
}

SCRIPT_API(AddServerRule, bool(const std::string& name, cell const* format))
{
	return addRule(GetAMX(), GetParams(), name, format);
}

SCRIPT_API(SetServerRule, bool(const std::string& name, cell const* format))
{
	return addRule(GetAMX(), GetParams(), name, format);
}

SCRIPT_API(IsValidServerRule, bool(const std::string& name))
{
	ICore* core = PawnManager::Get()->core;
	if (!core)
	{
		return false;
	}

	for (INetwork* network : core->getNetworks())
	{
		INetworkQueryExtension* query = queryExtension<INetworkQueryExtension>(network);

		if (query)
		{
			return query->isValidRule(name);
		}
	}
	return false;
}

SCRIPT_API(RemoveServerRule, bool(const std::string& name))
{
	ICore* core = PawnManager::Get()->core;
	if (!core)
	{
		return false;
	}

	for (INetwork* network : core->getNetworks())
	{
		INetworkQueryExtension* query = queryExtension<INetworkQueryExtension>(network);

		if (query)
		{
			return query->removeRule(name);
		}
	}
	return false;
}

SCRIPT_API(CountRunningTimers, int())
{
	ITimersComponent* timers = PawnManager::Get()->timers;
	return timers == nullptr ? 0 : timers->count();
}

SCRIPT_API(GetRunningTimers, int())
{
	return openmp_scripting::CountRunningTimers();
}
