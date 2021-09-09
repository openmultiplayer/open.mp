#include "sdk.hpp"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "../Types.hpp"

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
	return vector.length();
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
	return std::stof(string);
}

SCRIPT_API(GetPlayerPoolSize, int())
{
	int highestID = -1;
	for (IPlayer* player : PawnManager::Get()->players->entries()) {
		if (highestID < player->getID()) {
			highestID = player->getID();
		}
	}
	return highestID;
}

SCRIPT_API(GetVehiclePoolSize, int())
{
	IVehiclesComponent* vehicles = PawnManager::Get()->vehicles;
	if (vehicles) {
		int highestID = -1;
		for (IVehicle* vehicle : vehicles->entries()) {
			if (highestID < vehicle->getID()) {
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
	if (actors) {
		int highestID = -1;
		for (IActor* actor : actors->entries()) {
			if (highestID < actor->getID()) {
				highestID = actor->getID();
			}
		}
		return highestID;
	}
	return -1;
}

SCRIPT_API(print, void(const std::string& text)) {
	PawnManager::Get()->core->printLn("%s", text.c_str());
}

SCRIPT_API(AddCharModel, bool(int baseid, int newid, std::string const& dff, std::string const& textureLibrary))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(AddSimpleModel, bool(int virtualWorld, int baseid, int newid, std::string const& dff, std::string const& textureLibrary))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(AddSimpleModelTimed, bool(int virtualWorld, int baseid, int newid, std::string const& dff, std::string const& textureLibrary, int timeOn, int timeOff))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(AllowAdminTeleport, bool(bool allow))
{
	*PawnManager::Get()->config->getInt("rcon_allow_teleport") = allow;
	return true;
}

SCRIPT_API(AllowInteriorWeapons, bool(bool allow))
{
	*PawnManager::Get()->config->getInt("allow_interior_weapons") = allow;
	return true;
}

SCRIPT_API(Ban, bool(IPlayer& player))
{
	player.ban();
	return true;
}

SCRIPT_API(BanEx, bool(IPlayer& player, std::string const& reason))
{
	player.ban(reason);
	return true;
}

SCRIPT_API(BlockIpAddress, bool(std::string const& ipAddress, int timeMS))
{
	PeerAddress address;
	address.ipv6 = false;
	PeerAddress::FromString(address, ipAddress);
	BanEntry entry(address);
	for (INetwork* network : PawnManager::Get()->core->getNetworks()) {
		network->ban(entry, Milliseconds(timeMS));
	}
	return true;
}

SCRIPT_API(UnBlockIpAddress, bool(std::string const& ipAddress))
{
	PeerAddress address;
	address.ipv6 = false;
	PeerAddress::FromString(address, ipAddress);
	BanEntry entry(address);
	for (INetwork* network : PawnManager::Get()->core->getNetworks()) {
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
	*PawnManager::Get()->config->getInt("disable_interior_enter_exits") = true;
	return true;
}

SCRIPT_API(DisableNameTagLOS, bool())
{
	*PawnManager::Get()->config->getInt("disable_name_tag_los") = true;
	return true;
}

SCRIPT_API(EnableTirePopping, bool(bool enable))
{
	PawnManager::Get()->core->logLn(LogLevel::Warning, "EnableTirePopping() function is removed.");
	return true;
}

SCRIPT_API(EnableZoneNames, bool(bool enable))
{
	*PawnManager::Get()->config->getInt("enable_zone_names") = enable;
	return true;
}

SCRIPT_API(FindModelFileNameFromCRC, bool(int crc, std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(FindTextureFileNameFromCRC, bool(int crc, std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GameModeExit, bool())
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GameTextForAll, bool(std::string const& string, int time, int style))
{
	PawnManager::Get()->players->sendGameTextToAll(string, Milliseconds(time), style);
	return true;
}

SCRIPT_API(GameTextForPlayer, bool(IPlayer& player, std::string const& string, int time, int style))
{
	player.sendGameText(string, Milliseconds(time), style);
	return true;
}

int getConfigOptionAsInt(std::string const& cvar) {
	IConfig* config = PawnManager::Get()->config;
	auto res = config->getNameFromAlias(cvar);
	int* var = nullptr;
	if (!res.second.empty()) {
		if (res.first) {
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Deprecated console variable %s", cvar.c_str());
		}
		var = config->getInt(res.second);
	}
	else {
		var = config->getInt(cvar);
	}
	if (var) {
		return *var;
	}
	else {
		return 0;
	}
}

int getConfigOptionAsString(std::string const& cvar, std::string& buffer) {
	IConfig* config = PawnManager::Get()->config;
	auto res = config->getNameFromAlias(cvar);
	if (!res.second.empty()) {
		if (res.first) {
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Deprecated console variable %s", cvar.c_str());
		}
		buffer = config->getString(res.second);
	}
	else {
		buffer = config->getString(cvar);
	}
	return buffer.size();
}

SCRIPT_API(GetConsoleVarAsBool, bool(std::string const& cvar))
{
	return getConfigOptionAsInt(cvar);
}

SCRIPT_API(GetConsoleVarAsInt, int(std::string const& cvar))
{
	return getConfigOptionAsInt(cvar);
}

SCRIPT_API(GetConsoleVarAsString, int(std::string const& cvar, std::string& buffer))
{
	return getConfigOptionAsString(cvar, buffer);
}

SCRIPT_API(GetGravity, float())
{
	return *PawnManager::Get()->config->getFloat("gravity");
}

SCRIPT_API(GetNetworkStats, bool(std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetPlayerNetworkStats, bool(IPlayer& player, std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetPlayerVersion, bool(IPlayer& player, std::string& version))
{
	version = player.getClientVersionName();
	return true;
}

SCRIPT_API(GetServerTickRate, int())
{
	return PawnManager::Get()->core->tickRate();
}

SCRIPT_API(GetServerVarAsBool, bool(std::string const& cvar))
{
	return getConfigOptionAsInt(cvar);
}

SCRIPT_API(GetServerVarAsInt, int(std::string const& cvar))
{
	return getConfigOptionAsInt(cvar);
}

SCRIPT_API(GetServerVarAsString, bool(std::string const& cvar, std::string& buffer))
{
	return getConfigOptionAsString(cvar, buffer);
}

SCRIPT_API(GetWeaponName, bool(int weaponid, std::string& weapon))
{
	weapon = PawnManager::Get()->core->getWeaponName(PlayerWeapon(weaponid));
	return true;
}

SCRIPT_API(gpci, bool(IPlayer& player, std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(IsPlayerAdmin, bool(IPlayer& player))
{
	IPlayerConsoleData* data = player.queryData<IPlayerConsoleData>();
	if (data) {
		return data->hasConsoleAccess();
	}
	return false;
}

SCRIPT_API(Kick, bool(IPlayer& player))
{
	player.kick();
	return true;
}

SCRIPT_API(LimitGlobalChatRadius, bool(float chatRadius))
{
	*PawnManager::Get()->config->getInt("use_limit_global_chat_radius") = true;
	*PawnManager::Get()->config->getFloat("limit_global_chat_radius") = chatRadius;
	return true;
}

SCRIPT_API(LimitPlayerMarkerRadius, bool(float markerRadius))
{
	*PawnManager::Get()->config->getInt("limit_player_markers") = true;
	*PawnManager::Get()->config->getFloat("player_markers_draw_distance") = markerRadius;
	return true;
}

SCRIPT_API(NetStats_BytesReceived, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_BytesSent, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_ConnectionStatus, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_GetConnectedTime, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_GetIpPort, bool(IPlayer& player, std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_MessagesReceived, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_MessagesRecvPerSecond, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_MessagesSent, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(NetStats_PacketLossPercent, float(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(RedirectDownload, bool(IPlayer& player, std::string const& url))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SendDeathMessageToPlayer, bool(IPlayer& player, IPlayer* killer, IPlayer& killee, int weapon))
{
	player.sendDeathMessage(killee, killer, weapon);
	return true;
}

SCRIPT_API(SendPlayerMessageToAll, bool(IPlayer& sender, std::string const& message))
{
	PawnManager::Get()->players->sendChatMessageToAll(sender, message);
	return true;
}

SCRIPT_API(SendPlayerMessageToPlayer, bool(IPlayer& player, IPlayer& sender, std::string const& message))
{
	player.sendChatMessage(sender, message);
	return true;
}

SCRIPT_API(SendRconCommand, bool(std::string const& command))
{
	IConsoleComponent* console = PawnManager::Get()->console;
	if (console) {
		console->send(command);
	}
	return true;
}

SCRIPT_API(SetDeathDropAmount, bool(int amount))
{
	*PawnManager::Get()->config->getInt("death_drop_amount") = amount;
	return true;
}

SCRIPT_API(SetGameModeText, bool(std::string const& string))
{
	PawnManager::Get()->core->setData(SettableCoreDataType::ModeText, string);
	return true;
}

SCRIPT_API(SetGravity, bool(float gravity))
{
	PawnManager::Get()->core->setGravity(gravity);
	return true;
}

SCRIPT_API(SetNameTagDrawDistance, bool(float distance))
{
	*PawnManager::Get()->config->getFloat("name_tag_draw_distance") = distance;
	return true;
}

SCRIPT_API(SetTeamCount, bool(int count))
{
	return false;
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

SCRIPT_API(SHA256_PassHash, bool(std::string const& password, std::string const& salt, std::string& output))
{
	PawnManager::Get()->core->logLn(LogLevel::Warning, "Using unsafe hashing function SHA256_PassHash");

	StaticArray<char, 64 + 1> hash;
	bool res = PawnManager::Get()->core->sha256(password, salt, hash);
	if (res) {
		output = hash.data();
		return true;
	}
	output = "";
	return false;
}

SCRIPT_API(ShowNameTags, bool(bool show))
{
	*PawnManager::Get()->config->getInt("show_name_tags") = show;
	return true;
}

SCRIPT_API(ShowPlayerMarkers, bool(int mode))
{
	*PawnManager::Get()->config->getInt("show_player_markers") = mode;
	return true;
}

SCRIPT_API(UsePlayerPedAnims, bool())
{
	*PawnManager::Get()->config->getInt("use_player_ped_anims") = true;
	return true;
}
