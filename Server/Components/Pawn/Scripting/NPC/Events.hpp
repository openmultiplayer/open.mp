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

struct NPCEvents : public NPCEventHandler, public Singleton<NPCEvents>
{
	void onNPCFinishMove(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCFinishMove", DefaultReturnValue_True, npc.getID());
	}

	void onNPCCreate(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCCreate", DefaultReturnValue_True, npc.getID());
	}

	void onNPCDestroy(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCDestroy", DefaultReturnValue_True, npc.getID());
	}

	void onNPCWeaponStateChange(INPC& npc, PlayerWeaponState newState, PlayerWeaponState oldState) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCWeaponStateChange", DefaultReturnValue_True, npc.getID(), int(newState), int(oldState));
	}

	bool onNPCTakeDamage(INPC& npc, IPlayer& damager, float damage, uint8_t weapon, BodyPart bodyPart) override
	{
		auto result = !!PawnManager::Get()->CallAllInEntryFirst("OnNPCTakeDamage", DefaultReturnValue_True, npc.getID(), damager.getID(), damage, weapon, int(bodyPart));
		return result;
	}

	bool onNPCGiveDamage(INPC& npc, IPlayer& damager, float damage, uint8_t weapon, BodyPart bodyPart) override
	{
		auto result = !!PawnManager::Get()->CallAllInEntryFirst("OnNPCGiveDamage", DefaultReturnValue_True, npc.getID(), damager.getID(), damage, weapon, int(bodyPart));
		return result;
	}

	void onNPCDeath(INPC& npc, IPlayer* killer, int reason) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCDeath", DefaultReturnValue_True, npc.getID(), killer ? killer->getID() : INVALID_PLAYER_ID, reason);
	}

	void onNPCSpawn(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCSpawn", DefaultReturnValue_True, npc.getID());
	}

	void onNPCRespawn(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCRespawn", DefaultReturnValue_True, npc.getID());
	}

	void onNPCPlaybackStart(INPC& npc, int recordId) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCPlaybackStart", DefaultReturnValue_True, npc.getID(), recordId);
	}

	void onNPCPlaybackEnd(INPC& npc, int recordId) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCPlaybackEnd", DefaultReturnValue_True, npc.getID(), recordId);
	}

	bool onNPCShotMissed(INPC& npc, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnNPCWeaponShot",
			npc.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnNPCWeaponShot",
				DefaultReturnValue_True,
				npc.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onNPCShotPlayer(INPC& npc, IPlayer& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnNPCWeaponShot",
			npc.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnNPCWeaponShot",
				DefaultReturnValue_True,
				npc.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onNPCShotNPC(INPC& npc, INPC& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnNPCWeaponShot",
			npc.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnNPCWeaponShot",
				DefaultReturnValue_True,
				npc.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onNPCShotVehicle(INPC& npc, IVehicle& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnNPCWeaponShot",
			npc.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnNPCWeaponShot",
				DefaultReturnValue_True,
				npc.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onNPCShotObject(INPC& npc, IObject& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnNPCWeaponShot",
			npc.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnNPCWeaponShot",
				DefaultReturnValue_True,
				npc.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	bool onNPCShotPlayerObject(INPC& npc, IPlayerObject& target, const PlayerBulletData& bulletData) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnNPCWeaponShot",
			npc.getID(),
			bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
			bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		if (ret)
		{
			ret = PawnManager::Get()->CallInEntry(
				"OnNPCWeaponShot",
				DefaultReturnValue_True,
				npc.getID(),
				bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
				bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
		}
		return !!ret;
	}

	void onNPCFinishNodePoint(INPC& npc, int nodeId, uint16_t pointId) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCFinishNodePoint", DefaultReturnValue_True, npc.getID(), nodeId, int(pointId));
	}

	void onNPCFinishNode(INPC& npc, int nodeId) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCFinishNode", DefaultReturnValue_True, npc.getID(), nodeId);
	}

	bool onNPCChangeNode(INPC& npc, int newNodeId, int oldNodeId) override
	{
		cell ret = PawnManager::Get()->CallAllInEntryFirst("OnNPCChangeNode", DefaultReturnValue_True, npc.getID(), newNodeId, oldNodeId);
		return !!ret;
	}

	void onNPCFinishMovePath(INPC& npc, int pathId) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCFinishMovePath", DefaultReturnValue_True, npc.getID(), pathId);
	}

	void onNPCFinishMovePathPoint(INPC& npc, int pathId, int pointId) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCFinishMovePathPoint", DefaultReturnValue_True, npc.getID(), pathId, pointId);
	}
};
