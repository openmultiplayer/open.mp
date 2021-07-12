#include "sdk.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(SendClientMessage, bool(IPlayer* player, uint32_t colour, std::string const& msg))
{
	player->sendClientMessage(colour, msg.c_str());
	return true;
}

SCRIPT_API(SetPlayerCameraPos, bool(IPlayer* player, const Vector3& vec))
{	
	player->setCameraPosition(vec);
	return true;
}

SCRIPT_API(SetPlayerDrunkLevel, bool(IPlayer* player, int level))
{
	player->setDrunkLevel(level);
	return true;
}

SCRIPT_API(SetPlayerInterior, bool(IPlayer* player, int interiorid))
{
	player->setInterior(interiorid);
	return true;
}

SCRIPT_API(SetPlayerWantedLevel, bool(IPlayer* player, int level))
{
	player->setWantedLevel(level);
	return true;
}

SCRIPT_API(SetPlayerWeather, bool(IPlayer* player, int weatherid))
{
	player->setWeather(weatherid);
	return true;
}

SCRIPT_API(SetPlayerSkin, bool(IPlayer* player, int skinid))
{
	player->setSkin(skinid);
	return true;
}

SCRIPT_API(SetPlayerShopName, bool(IPlayer* player, std::string const& name))
{
	player->setShopName(name.c_str());
	return true;
}

SCRIPT_API(GivePlayerMoney, bool(IPlayer* player, int amount))
{
	player->giveMoney(amount);
	return true;
}

SCRIPT_API(SetPlayerCameraLookAt, bool(IPlayer* player, const Vector3& vec, int cutType))
{
	player->setCameraLookAt(vec, cutType);
	return true;
}

SCRIPT_API(SendCommand, bool(IPlayer* player, std::string const& msg))
{
	player->sendCommand(msg.c_str());
	return true;
}

SCRIPT_API(SetCameraBehindPlayer, bool(IPlayer* player))
{
	player->setCameraBehind();
	return true;
}

SCRIPT_API(CreateExplosion, bool(IPlayer* player, const Vector3& vec, int type, float radius))
{
	player->createExplosion(vec, type, radius);
	return true;
}

SCRIPT_API(PlayAudioStreamForPlayer, bool(IPlayer* player, std::string const& url, const Vector3& pos, bool usePos, float distance))
{
	player->playAudio(url.c_str(), usePos, pos, distance);
	return true;
}

SCRIPT_API(StopAudioStreamForPlayer, bool(IPlayer* player))
{
	player->stopAudio();
	return true;
}

SCRIPT_API(SendDeathMessage, bool(IPlayer* player, int PlayerID, int KillerID, int reason))
{
	player->sendDeathMessage(PlayerID, KillerID, reason);
	return true;
}

