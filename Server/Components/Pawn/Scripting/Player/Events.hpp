/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "../../Manager/Manager.hpp"
#include "sdk.hpp"

class PlayerEvents : public PlayerSpawnEventHandler, public PlayerConnectEventHandler, public PlayerStreamEventHandler, public PlayerTextEventHandler, public PlayerShotEventHandler, public PlayerChangeEventHandler, public PlayerDamageEventHandler, public PlayerClickEventHandler, public PlayerCheckEventHandler, public PlayerUpdateEventHandler, public Singleton<PlayerEvents>
{
public:
	void onPlayerConnect(IPlayer& player) override
	{
		PawnManager::Get()->CallInSidesWhile1("OnPlayerConnect", player.getID());
		PawnManager::Get()->CallInEntry("OnPlayerConnect", DefaultReturnValue_True, player.getID());
	}

	void onPlayerSpawn(IPlayer& player) override
	{
		PawnManager::Get()->CallInSidesWhile1("OnPlayerSpawn", player.getID());
		PawnManager::Get()->CallInEntry("OnPlayerSpawn", DefaultReturnValue_True, player.getID());
	}

	bool onPlayerCommandText(IPlayer& player, StringView cmdtext) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerCommandText", player.getID(), cmdtext);
		if (!ret)
		{
			ret = PawnManager::Get()->CallInEntry("OnPlayerCommandText", DefaultReturnValue_False, player.getID(), cmdtext);
		}
		return !!ret;
	}

	void onPlayerKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerKeyStateChange", DefaultReturnValue_True, player.getID(), newKeys, oldKeys);
	}

	void onIncomingConnection(IPlayer& player, StringView ipAddress, unsigned short port) override
	{
		PawnManager::Get()->CallInSidesWhile0("OnIncomingConnection", player.getID(), ipAddress, port);
		PawnManager::Get()->CallInEntry("OnIncomingConnection", DefaultReturnValue_True, player.getID(), ipAddress, port);
	}

	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override
	{
		PawnManager::Get()->CallInSidesWhile1("OnPlayerDisconnect", player.getID(), int(reason));
		PawnManager::Get()->CallInEntry("OnPlayerDisconnect", DefaultReturnValue_True, player.getID(), int(reason));
	}

	bool onPlayerRequestSpawn(IPlayer& player) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1("OnPlayerRequestSpawn", player.getID());
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry("OnPlayerRequestSpawn", DefaultReturnValue_True, player.getID());
		}
		return !!ret;
	}

	void onPlayerStreamIn(IPlayer& player, IPlayer& forPlayer) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerStreamIn", DefaultReturnValue_True, player.getID(), forPlayer.getID());
	}

	void onPlayerStreamOut(IPlayer& player, IPlayer& forPlayer) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerStreamOut", DefaultReturnValue_True, player.getID(), forPlayer.getID());
	}

	bool onPlayerText(IPlayer& player, StringView message) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1("OnPlayerText", player.getID(), message);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry("OnPlayerText", DefaultReturnValue_True, player.getID(), message);
		}
		return !!ret;
	}

	bool onPlayerShotMissed(IPlayer& player, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnPlayerWeaponShot",
			player.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnPlayerWeaponShot",
				DefaultReturnValue_True,
				player.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onPlayerShotPlayer(IPlayer& player, IPlayer& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnPlayerWeaponShot",
			player.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnPlayerWeaponShot",
				DefaultReturnValue_True,
				player.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onPlayerShotVehicle(IPlayer& player, IVehicle& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnPlayerWeaponShot",
			player.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnPlayerWeaponShot",
				DefaultReturnValue_True,
				player.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onPlayerShotObject(IPlayer& player, IObject& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnPlayerWeaponShot",
			player.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnPlayerWeaponShot",
				DefaultReturnValue_True,
				player.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onPlayerShotPlayerObject(IPlayer& player, IPlayerObject& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnPlayerWeaponShot",
			player.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnPlayerWeaponShot",
				DefaultReturnValue_True,
				player.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	void onPlayerDeath(IPlayer& player, IPlayer* killer, int reason) override
	{
		PawnManager::Get()->CallInSidesWhile1("OnPlayerDeath", player.getID(), killer ? killer->getID() : INVALID_PLAYER_ID, reason);
		PawnManager::Get()->CallInEntry("OnPlayerDeath", DefaultReturnValue_True, player.getID(), killer ? killer->getID() : INVALID_PLAYER_ID, reason);
	}

	void onPlayerTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part) override
	{
		PawnManager::Get()->CallInSidesWhile0("OnPlayerTakeDamage", player.getID(), from ? from->getID() : INVALID_PLAYER_ID, amount, weapon, int(part));
		PawnManager::Get()->CallInEntry("OnPlayerTakeDamage", DefaultReturnValue_True, player.getID(), from ? from->getID() : INVALID_PLAYER_ID, amount, weapon, int(part));
	}

	void onPlayerGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part) override
	{
		PawnManager::Get()->CallInSidesWhile0("OnPlayerGiveDamage", player.getID(), to.getID(), amount, weapon, int(part));
		PawnManager::Get()->CallInEntry("OnPlayerGiveDamage", DefaultReturnValue_True, player.getID(), to.getID(), amount, weapon, int(part));
	}

	void onPlayerInteriorChange(IPlayer& player, unsigned newInterior, unsigned oldInterior) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerInteriorChange", DefaultReturnValue_True, player.getID(), newInterior, oldInterior);
	}

	void onPlayerStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerStateChange", DefaultReturnValue_True, player.getID(), int(newState), int(oldState));
	}

	void onPlayerClickMap(IPlayer& player, Vector3 pos) override
	{
		PawnManager::Get()->CallInEntry("OnPlayerClickMap", DefaultReturnValue_True, player.getID(), pos.x, pos.y, pos.z);
		PawnManager::Get()->CallInSidesWhile0("OnPlayerClickMap", player.getID(), pos.x, pos.y, pos.z);
	}

	void onPlayerClickPlayer(IPlayer& player, IPlayer& clicked, PlayerClickSource source) override
	{
		PawnManager::Get()->CallInSidesWhile0("OnPlayerClickPlayer", player.getID(), clicked.getID(), int(source));
		PawnManager::Get()->CallInEntry("OnPlayerClickPlayer", DefaultReturnValue_True, player.getID(), clicked.getID(), int(source));
	}

	void onClientCheckResponse(IPlayer& player, int actionType, int address, int results) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnClientCheckResponse", DefaultReturnValue_True, player.getID(), actionType, address, results);
	}

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1("OnPlayerUpdate", player.getID());
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry("OnPlayerUpdate", DefaultReturnValue_True, player.getID());
		}
		return !!ret;
	}
};
