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
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(AllowInteriorWeapons, bool(bool allow))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(Ban, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(BanEx, bool(IPlayer& player, std::string const& reason))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(BlockIpAddress, bool(std::string const& ipAddress, int timeMS))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(UnBlockIpAddress, bool(std::string const& ipAddress))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(ConnectNPC, bool(std::string const& name, std::string const& script))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(DisableInteriorEnterExits, bool())
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(DisableNameTagLOS, bool())
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(EnableTirePopping, bool(bool enable))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(EnableZoneNames, bool(bool enable))
{
	throw pawn_natives::NotImplemented();
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

SCRIPT_API(OnGameModeExit, bool())
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GameTextForAll, bool(std::string const& string, int time, int style))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GameTextForPlayer, bool(IPlayer& player, std::string const& string, int time, int style))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetConsoleVarAsBool, bool(std::string const& cvar))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetConsoleVarAsInt, int(std::string const& cvar))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetConsoleVarAsString, bool(std::string const& cvar, std::string& buffer))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetGravity, float())
{
	throw pawn_natives::NotImplemented();
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
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetServerTickRate, int())
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetServerVarAsBool, bool(std::string const& cvar))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetServerVarAsInt, int(std::string const& cvar))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetServerVarAsString, bool(std::string const& cvar, std::string& buffer))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetWeaponName, bool(int weaponid, std::string& weapon))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(gpci, bool(IPlayer& player, std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(IsPlayerAdmin, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(Kick, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(KillTimer, bool(int timerid))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(LimitGlobalChatRadius, bool(float chatRadius))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(LimitPlayerMarkerRadius, bool(float markerRadius))
{
	throw pawn_natives::NotImplemented();
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

SCRIPT_API(SendClientMessageToAll, bool(int colour, std::string const& message))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SendDeathMessageToPlayer, bool(IPlayer& player, IPlayer* killer, IPlayer& killee, int weapon))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SendPlayerMessageToAll, bool(int senderid, std::string const& message))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SendPlayerMessageToPlayer, bool(IPlayer& player, int senderid, std::string const& message))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SendRconCommand, bool(std::string const& command))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetDeathDropAmount, bool(int amount))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetGameModeText, bool(std::string const& string))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetGravity, bool(float gravity))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetNameTagDrawDistance, bool(float distance))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetTeamCount, bool(int count))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetWeather, bool(int weatherid))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetWorldTime, bool(int hour))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SHA256_PassHash, bool(std::string const& password, std::string const& salt, std::string& output))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(ShowNameTags, bool(bool show))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(ShowPlayerMarkers, bool(int mode))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(UsePlayerPedAnims, bool())
{
	throw pawn_natives::NotImplemented();
}
