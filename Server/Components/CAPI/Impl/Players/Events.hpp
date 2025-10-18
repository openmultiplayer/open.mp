/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#pragma once
#include "../ComponentManager.hpp"
#include "sdk.hpp"

template <EventPriorityType PRIORITY>
class PlayerEvents : public PlayerSpawnEventHandler, public PlayerConnectEventHandler, public PlayerStreamEventHandler, public PlayerTextEventHandler, public PlayerShotEventHandler, public PlayerChangeEventHandler, public PlayerDamageEventHandler, public PlayerClickEventHandler, public PlayerCheckEventHandler, public PlayerUpdateEventHandler, public Singleton<PlayerEvents<PRIORITY>>
{
public:
	void onPlayerConnect(IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerConnect", EventReturnHandler::None, &player);
	}

	void onPlayerSpawn(IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerSpawn", EventReturnHandler::None, &player);
	}

	bool onPlayerCommandText(IPlayer& player, StringView cmdtext) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerCommandText", EventReturnHandler::StopAtTrue, &player, CREATE_CAPI_STRING_VIEW(cmdtext));
	}

	void onPlayerKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerKeyStateChange", EventReturnHandler::None, &player, int(newKeys), int(oldKeys));
	}

	void onIncomingConnection(IPlayer& player, StringView ipAddress, unsigned short port) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onIncomingConnection", EventReturnHandler::None, &player, CREATE_CAPI_STRING_VIEW(ipAddress), int(port));
	}

	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerDisconnect", EventReturnHandler::None, &player, int(reason));
	}

	bool onPlayerRequestSpawn(IPlayer& player) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerRequestSpawn", EventReturnHandler::StopAtFalse, &player);
	}

	void onPlayerStreamIn(IPlayer& player, IPlayer& forPlayer) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerStreamIn", EventReturnHandler::None, &player, &forPlayer);
	}

	void onPlayerStreamOut(IPlayer& player, IPlayer& forPlayer) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerStreamOut", EventReturnHandler::None, &player, &forPlayer);
	}

	bool onPlayerText(IPlayer& player, StringView message) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerText", EventReturnHandler::StopAtFalse, &player, CREATE_CAPI_STRING_VIEW(message));
	}

	bool onPlayerShotMissed(IPlayer& player, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerShotMissed", EventReturnHandler::StopAtFalse, &player,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onPlayerShotPlayer(IPlayer& player, IPlayer& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerShotPlayer", EventReturnHandler::StopAtFalse, &player, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onPlayerShotVehicle(IPlayer& player, IVehicle& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerShotVehicle", EventReturnHandler::StopAtFalse, &player, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onPlayerShotObject(IPlayer& player, IObject& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerShotObject", EventReturnHandler::StopAtFalse, &player, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onPlayerShotPlayerObject(IPlayer& player, IPlayerObject& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerShotPlayerObject", EventReturnHandler::StopAtFalse, &player, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	void onPlayerDeath(IPlayer& player, IPlayer* killer, int reason) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerDeath", EventReturnHandler::None, &player, killer, reason);
	}

	void onPlayerTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerTakeDamage", EventReturnHandler::None, &player, from, amount, int(weapon), int(part));
	}

	void onPlayerGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerGiveDamage", EventReturnHandler::None, &player, &to, amount, int(weapon), int(part));
	}

	void onPlayerInteriorChange(IPlayer& player, unsigned newInterior, unsigned oldInterior) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerInteriorChange", EventReturnHandler::None, &player, int(newInterior), int(oldInterior));
	}

	void onPlayerStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerStateChange", EventReturnHandler::None, &player, int(newState), int(oldState));
	}

	void onPlayerClickMap(IPlayer& player, Vector3 pos) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerClickMap", EventReturnHandler::None, &player, pos.x, pos.y, pos.z);
	}

	void onPlayerClickPlayer(IPlayer& player, IPlayer& clicked, PlayerClickSource source) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerClickPlayer", EventReturnHandler::None, &player, &clicked, int(source));
	}

	void onClientCheckResponse(IPlayer& player, int actionType, int address, int results) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onClientCheckResponse", EventReturnHandler::None, &player, actionType, address, results);
	}

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerUpdate", EventReturnHandler::StopAtFalse, &player);
	}
};
