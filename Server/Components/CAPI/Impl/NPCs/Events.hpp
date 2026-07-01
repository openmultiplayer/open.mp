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
struct NPCEvents : public NPCEventHandler, public Singleton<NPCEvents<PRIORITY>>
{
	void onNPCFinishMove(INPC& npc) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCFinishMove", EventReturnHandler::None, &npc);
	}

	void onNPCCreate(INPC& npc) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCCreate", EventReturnHandler::None, &npc);
	}

	void onNPCDestroy(INPC& npc) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCDestroy", EventReturnHandler::None, &npc);
	}

	void onNPCWeaponStateChange(INPC& npc, PlayerWeaponState newState, PlayerWeaponState oldState) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCWeaponStateChange", EventReturnHandler::None, &npc, int(newState), int(oldState));
	}

	bool onNPCTakeDamage(INPC& npc, IPlayer& damager, float damage, uint8_t weapon, BodyPart bodyPart) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCTakeDamage", EventReturnHandler::StopAtFalse, &npc, &damager, damage, int(weapon), int(bodyPart));
	}

	bool onNPCGiveDamage(INPC& npc, IPlayer& damaged, float damage, uint8_t weapon, BodyPart bodyPart) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCGiveDamage", EventReturnHandler::StopAtFalse, &npc, &damaged, damage, int(weapon), int(bodyPart));
	}

	void onNPCDeath(INPC& npc, IPlayer* killer, int reason) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCDeath", EventReturnHandler::None, &npc, killer, reason);
	}

	void onNPCSpawn(INPC& npc) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCSpawn", EventReturnHandler::None, &npc);
	}

	void onNPCRespawn(INPC& npc) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCRespawn", EventReturnHandler::None, &npc);
	}

	void onNPCPlaybackStart(INPC& npc, int recordId) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCPlaybackStart", EventReturnHandler::None, &npc, recordId);
	}

	void onNPCPlaybackEnd(INPC& npc, int recordId) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCPlaybackEnd", EventReturnHandler::None, &npc, recordId);
	}

	bool onNPCShotMissed(INPC& npc, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCShotMissed", EventReturnHandler::StopAtFalse, &npc,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onNPCShotPlayer(INPC& npc, IPlayer& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCShotPlayer", EventReturnHandler::StopAtFalse, &npc, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onNPCShotNPC(INPC& npc, INPC& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCShotNPC", EventReturnHandler::StopAtFalse, &npc, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onNPCShotVehicle(INPC& npc, IVehicle& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCShotVehicle", EventReturnHandler::StopAtFalse, &npc, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onNPCShotObject(INPC& npc, IObject& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCShotObject", EventReturnHandler::StopAtFalse, &npc, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	bool onNPCShotPlayerObject(INPC& npc, IPlayerObject& target, const PlayerBulletData& bulletData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCShotPlayerObject", EventReturnHandler::StopAtFalse, &npc, &target,
			int(bulletData.weapon), bulletData.offset.x, bulletData.offset.y, bulletData.offset.z);
	}

	void onNPCFinishNodePoint(INPC& npc, int nodeId, uint16_t pointId) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCFinishNodePoint", EventReturnHandler::None, &npc, nodeId, int(pointId));
	}

	void onNPCFinishNode(INPC& npc, int nodeId) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCFinishNode", EventReturnHandler::None, &npc, nodeId);
	}

	bool onNPCChangeNode(INPC& npc, int newNodeId, int oldNodeId) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onNPCChangeNode", EventReturnHandler::StopAtFalse, &npc, newNodeId, oldNodeId);
	}

	void onNPCFinishMovePath(INPC& npc, int pathId) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCFinishMovePath", EventReturnHandler::None, &npc, pathId);
	}

	void onNPCFinishMovePathPoint(INPC& npc, int pathId, int pointId) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onNPCFinishMovePathPoint", EventReturnHandler::None, &npc, pathId, pointId);
	}
};
