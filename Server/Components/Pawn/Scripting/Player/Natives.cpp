#include "sdk.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(SendClientMessage, bool(IPlayer& player, uint32_t colour, std::string const& msg))
{
	player.sendClientMessage(Colour::FromRGBA(colour), msg);
	return true;
}

SCRIPT_API(SetPlayerCameraPos, bool(IPlayer& player, const Vector3& vec))
{	
	player.setCameraPosition(vec);
	return true;
}

SCRIPT_API(SetPlayerDrunkLevel, bool(IPlayer& player, int level))
{
	player.setDrunkLevel(level);
	return true;
}

SCRIPT_API(SetPlayerInterior, bool(IPlayer& player, int interiorid))
{
	player.setInterior(interiorid);
	return true;
}

SCRIPT_API(SetPlayerWantedLevel, bool(IPlayer& player, int level))
{
	player.setWantedLevel(level);
	return true;
}

SCRIPT_API(SetPlayerWeather, bool(IPlayer& player, int weatherid))
{
	player.setWeather(weatherid);
	return true;
}

SCRIPT_API(SetPlayerSkin, bool(IPlayer& player, int skinid))
{
	player.setSkin(skinid);
	return true;
}

SCRIPT_API(SetPlayerShopName, bool(IPlayer& player, std::string const& name))
{
	player.setShopName(name);
	return true;
}

SCRIPT_API(GivePlayerMoney, bool(IPlayer& player, int amount))
{
	player.giveMoney(amount);
	return true;
}

SCRIPT_API(SetPlayerCameraLookAt, bool(IPlayer& player, const Vector3& vec, int cutType))
{
	player.setCameraLookAt(vec, cutType);
	return true;
}

SCRIPT_API(SendCommand, bool(IPlayer& player, std::string const& msg))
{
	player.sendCommand(msg);
	return true;
}

SCRIPT_API(SetCameraBehindPlayer, bool(IPlayer& player))
{
	player.setCameraBehind();
	return true;
}

SCRIPT_API(CreateExplosion, bool(IPlayer& player, const Vector3& vec, int type, float radius))
{
	player.createExplosion(vec, type, radius);
	return true;
}

SCRIPT_API(PlayAudioStreamForPlayer, bool(IPlayer& player, std::string const& url, const Vector3& pos, bool usePos, float distance))
{
	player.playAudio(url, usePos, pos, distance);
	return true;
}

SCRIPT_API(StopAudioStreamForPlayer, bool(IPlayer& player))
{
	player.stopAudio();
	return true;
}

SCRIPT_API(SendDeathMessage, bool(IPlayer& player, IPlayer* killer, int KillerID, int reason))
{
	PawnManager::Get()->players->sendDeathMessageToAll(player, killer, reason);
	return true;
}

SCRIPT_API(ToggleWidescreen, bool(IPlayer& player, bool enable))
{
	player.setWidescreen(enable);
	return true;
}

SCRIPT_API(SetPlayerHealth, bool(IPlayer& player, float health))
{
	player.setHealth(health);
	return true;
}

SCRIPT_API(GetPlayerHealth, bool(IPlayer& player, float* health))
{
	*health = player.getHealth();
	return true;
}

SCRIPT_API(SetPlayerArmour, bool(IPlayer& player, float armour))
{
	player.setArmour(armour);
	return true;
}

SCRIPT_API(GetPlayerArmour, bool(IPlayer& player, float* armour))
{
	*armour = player.getArmour();
	return true;
}

SCRIPT_API(SetPlayerTeam, bool(IPlayer& player, int teamid))
{
	player.setTeam(teamid);
	return true;
}

SCRIPT_API(GetPlayerTeam, int(IPlayer& player))
{
	return player.getTeam();
}

SCRIPT_API(SetPlayerScore, bool(IPlayer& player, int score))
{
	player.setScore(score);
	return true;
}

SCRIPT_API(GetPlayerScore, int(IPlayer& player))
{
	return player.getScore();
}

SCRIPT_API(GetPlayerSkin, int(IPlayer& player))
{
	return player.getSkin();
}

SCRIPT_API(SetPlayerColor, bool(IPlayer& player, uint32_t colour))
{
	player.setColour(Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(GetPlayerColor, int(IPlayer& player))
{
	return player.getColour().RGBA();
}

SCRIPT_API(GetPlayerDrunkLevel, int(IPlayer& player))
{
	return player.getDrunkLevel();
}

SCRIPT_API(GivePlayerWeapon, bool(IPlayer& player, uint8_t weaponid, uint8_t ammo))
{
	WeaponSlotData data;
	data.id = weaponid;
	data.ammo = ammo;
	player.giveWeapon(data);
	return true;
}

SCRIPT_API(GetPlayerMoney, int(IPlayer& player)) {
	return player.getMoney();
}

SCRIPT_API(ResetPlayerMoney, bool(IPlayer& player))
{
	player.resetMoney();
	return true;
}

SCRIPT_API(SetPlayerName, bool(IPlayer& player, const std::string& name))
{
	player.setName(name);
	return true;
}

SCRIPT_API(GetPlayerState, int(IPlayer& player))
{
	return player.getState();
}

SCRIPT_API(GetPlayerPing, int(IPlayer& player))
{
	return player.getPing();
}

SCRIPT_API(GetPlayerWeapon, int(IPlayer& player))
{
	return player.getArmedWeapon();
}

SCRIPT_API(SetPlayerTime, bool(IPlayer& player, int hour, int minute))
{
	player.setTime(std::chrono::hours(hour), std::chrono::minutes(minute));
	return true;
}

SCRIPT_API(GetPlayerTime, bool(IPlayer& player, int* hour, int* minute))
{
	std::pair<std::chrono::hours, std::chrono::minutes> data = player.getTime();
	*hour = data.first.count();
	*minute = data.second.count();
	return true;
}

SCRIPT_API(TogglePlayerClock, bool(IPlayer& player, bool toggle))
{
	player.toggleClock(toggle);
	return true;
}

SCRIPT_API(ForceClassSelection, bool(IPlayer& player))
{
	player.forceClassSelection();
	return true;
}

SCRIPT_API(GetPlayerWantedLevel, int(IPlayer& player))
{
	return player.getWantedLevel();
}

SCRIPT_API(SetPlayerFightingStyle, bool(IPlayer& player, int style))
{
	player.setFightingStyle(PlayerFightingStyle(style));
	return true;
}

SCRIPT_API(GetPlayerFightingStyle, int(IPlayer& player))
{
	return player.getFightingStyle();
}

SCRIPT_API(SetPlayerVelocity, bool(IPlayer& player, const Vector3 & velocity))
{
	player.setVelocity(velocity);
	return true;
}

SCRIPT_API(GetPlayerVelocity, bool(IPlayer& player, Vector3& velocity))
{
	velocity = player.getVelocity();
	return true;
}

SCRIPT_API(GetPlayerCameraPos, bool(IPlayer& player, Vector3& pos))
{
	pos = player.getCameraPosition();
	return true;
}

SCRIPT_API(GetPlayerDistanceFromPoint, float(IPlayer& player, const Vector3 & pos))
{
	Vector3 playerCoords = player.getPosition();
	return glm::distance(playerCoords, pos);
}

SCRIPT_API(GetPlayerInterior, int(IPlayer& player))
{
	return player.getInterior();
}

SCRIPT_API(GetPlayerPos, bool(IPlayer& player, Vector3& pos))
{
	pos = player.getPosition();
	return true;
}

SCRIPT_API(GetPlayerVirtualWorld, int(IPlayer& player))
{
	return player.getVirtualWorld();
}

SCRIPT_API(IsPlayerNPC, bool(IPlayer& player))
{
	return player.isBot();
}

SCRIPT_API(IsPlayerStreamedIn, bool(IPlayer& player, IPlayer& other))
{
	return player.isStreamedInForPlayer(other);
}

SCRIPT_API(PlayerPlaySound, bool(IPlayer& player, uint32_t sound, const Vector3& pos))
{
	player.playSound(sound, pos);
	return true;
}

SCRIPT_API(PlayerSpectatePlayer, bool(IPlayer& player, IPlayer& target, int mode))
{
	player.spectatePlayer(target, PlayerSpectateMode(mode));
	return true;
}

SCRIPT_API(PlayerSpectateVehicle, bool(IPlayer& player, IVehicle& target, int mode))
{
	player.spectateVehicle(target, PlayerSpectateMode(mode));
	return true;
}

SCRIPT_API(SetPlayerVirtualWorld, bool(IPlayer& player, int vw))
{
	player.setVirtualWorld(vw);
	return true;
}

SCRIPT_API(SetPlayerWorldBounds, bool(IPlayer& player, float xMax, float xMin, float yMax, float yMin))
{
	Vector4 coords = { xMax, xMin, yMax, yMin };
	player.setWorldBounds(coords);
	return true;
}
