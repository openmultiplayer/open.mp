/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "../../format.hpp"

inline float getAngleOfLine(float x, float y)
{
	float angle = atan2(y, x) * (180.0f / M_PI) + 270.0f;
	if (angle >= 360.0f)
	{
		angle -= 360.0f;
	}
	else if (angle < 0.0f)
	{
		angle += 360.0f;
	}
	return angle;
}

SCRIPT_API(NPC_Create, int(const String& name))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		auto npc = component->create(name.c_str());
		if (npc)
		{
			return npc->getID();
		}
	}
	return INVALID_PLAYER_ID;
}

SCRIPT_API(NPC_Destroy, bool(INPC& npc))
{
	if (PawnManager::Get()->npcs)
	{
		PawnManager::Get()->npcs->destroy(npc);
		return true;
	}
	return false;
}

SCRIPT_API(NPC_IsValid, bool(INPC* npc))
{
	return npc != nullptr;
}

SCRIPT_API(NPC_Spawn, bool(INPC& npc))
{
	npc.spawn();
	return true;
}

SCRIPT_API(NPC_Respawn, bool(INPC& npc))
{
	npc.respawn();
	return true;
}

SCRIPT_API(NPC_SetPos, bool(INPC& npc, Vector3 position))
{
	npc.setPosition(position, true);
	return true;
}

SCRIPT_API(NPC_SetVehiclePos, bool(INPC& npc, Vector3 position))
{
	npc.setVehiclePosition(position, true);
	return true;
}

SCRIPT_API(NPC_GetPos, bool(INPC& npc, Vector3& position))
{
	position = npc.getPosition();
	return true;
}

SCRIPT_API(NPC_SetRot, bool(INPC& npc, Vector3 rotation))
{
	npc.setRotation(rotation, true);
	return true;
}

SCRIPT_API(NPC_SetVehicleRot, bool(INPC& npc, Vector3 rotation))
{
	npc.setVehicleRotation(rotation, true);
	return true;
}

SCRIPT_API(NPC_GetRot, bool(INPC& npc, Vector3& rotation))
{
	rotation = npc.getRotation().ToEuler();
	return true;
}

SCRIPT_API(NPC_SetFacingAngle, bool(INPC& npc, float angle))
{
	auto rotation = npc.getRotation().ToEuler();
	rotation.z = angle;
	npc.setRotation(rotation, true);
	return true;
}

SCRIPT_API(NPC_GetFacingAngle, bool(INPC& npc, float& angle))
{
	auto rotation = npc.getRotation().ToEuler();
	angle = rotation.z;
	return true;
}

SCRIPT_API(NPC_SetVirtualWorld, bool(INPC& npc, int virtualWorld))
{
	npc.setVirtualWorld(virtualWorld);
	return true;
}

SCRIPT_API(NPC_GetVirtualWorld, int(INPC& npc))
{
	return npc.getVirtualWorld();
}

SCRIPT_API(NPC_Move, bool(INPC& npc, Vector3 targetPos, int moveType, float moveSpeed, float stopRange))
{
	return npc.move(targetPos, NPCMoveType(moveType), moveSpeed, stopRange);
}

SCRIPT_API(NPC_MoveToPlayer, bool(INPC& npc, IPlayer& player, int moveType, float moveSpeed, float stopRange, int posCheckUpdateDelay, bool autoRestart))
{
	return npc.moveToPlayer(player, NPCMoveType(moveType), moveSpeed, stopRange, Milliseconds(posCheckUpdateDelay), autoRestart);
}

SCRIPT_API(NPC_StopMove, bool(INPC& npc))
{
	npc.stopMove();
	return true;
}

SCRIPT_API(NPC_IsMoving, bool(INPC& npc))
{
	return npc.isMoving();
}

SCRIPT_API(NPC_IsMovingToPlayer, bool(INPC& npc, IPlayer& player))
{
	return npc.isMovingToPlayer(player);
}

SCRIPT_API(NPC_SetSkin, bool(INPC& npc, int model))
{
	npc.setSkin(model);
	return true;
}

SCRIPT_API(NPC_GetSkin, int(INPC& npc))
{
	auto player = npc.getPlayer();
	if (player)
	{
		return player->getSkin();
	}
	return -1;
}

SCRIPT_API(NPC_GetCustomSkin, bool(INPC& npc))
{
	auto player = npc.getPlayer();
	if (player)
	{
		IPlayerCustomModelsData* data = queryExtension<IPlayerCustomModelsData>(player);
		if (!data)
		{
			return -1;
		}
		return data->getCustomSkin();
	}
	return -1;
}

SCRIPT_API(NPC_IsStreamedIn, bool(INPC& npc, IPlayer& player))
{
	return npc.isStreamedInForPlayer(player);
}

SCRIPT_API(NPC_IsAnyStreamedIn, bool(INPC& npc))
{
	auto streamedIn = npc.streamedForPlayers();
	return streamedIn.size() > 1;
}

SCRIPT_API(NPC_GetAll, int(DynamicArray<int>& outputNPCs))
{
	int index = -1;
	auto npcs = PawnManager::Get()->npcs;
	if (npcs)
	{
		if (outputNPCs.size() < npcs->count())
		{
			PawnManager::Get()->core->printLn(
				"There are %zu NPCs in your server but array size used in `NPC_GetAll` is %zu; Use a bigger size in your script.",
				npcs->count(),
				outputNPCs.size());
		}

		for (INPC* npc : *npcs)
		{
			index++;
			if (index >= outputNPCs.size())
			{
				break;
			}
			outputNPCs[index] = npc->getID();
		}
	}
	return index + 1;
}

SCRIPT_API(NPC_SetInterior, bool(INPC& npc, int interior))
{
	npc.setInterior(interior);
	return true;
}

SCRIPT_API(NPC_GetInterior, int(INPC& npc))
{
	return npc.getInterior();
}

SCRIPT_API(NPC_SetHealth, bool(INPC& npc, float health))
{
	npc.setHealth(health);
	return true;
}

SCRIPT_API(NPC_GetHealth, float(INPC& npc))
{
	return npc.getHealth();
}

SCRIPT_API(NPC_SetArmour, bool(INPC& npc, float armour))
{
	npc.setArmour(armour);
	return true;
}

SCRIPT_API(NPC_GetArmour, float(INPC& npc))
{
	return npc.getArmour();
}

SCRIPT_API(NPC_IsDead, bool(INPC& npc))
{
	return npc.isDead();
}

SCRIPT_API(NPC_SetWeapon, bool(INPC& npc, uint8_t weapon))
{
	npc.setWeapon(weapon);
	return true;
}

SCRIPT_API(NPC_GetWeapon, uint8_t(INPC& npc))
{
	return npc.getWeapon();
}

SCRIPT_API(NPC_SetAmmo, bool(INPC& npc, int ammo))
{
	npc.setAmmo(ammo);
	return true;
}

SCRIPT_API(NPC_GetAmmo, int(INPC& npc))
{
	return npc.getAmmo();
}

SCRIPT_API(NPC_SetKeys, bool(INPC& npc, uint16_t upAndDown, uint16_t leftAndRight, uint16_t keys))
{
	npc.setKeys(upAndDown, leftAndRight, keys);
	return true;
}

SCRIPT_API(NPC_GetKeys, bool(INPC& npc, uint16_t& upAndDown, uint16_t& leftAndDown, uint16_t& keys))
{
	npc.getKeys(upAndDown, leftAndDown, keys);
	return true;
}

SCRIPT_API(NPC_SetWeaponSkillLevel, bool(INPC& npc, uint8_t skill, int level))
{
	npc.setWeaponSkillLevel(PlayerWeaponSkill(skill), level);
	return true;
}

SCRIPT_API(NPC_GetWeaponSkillLevel, int(INPC& npc, int skill))
{
	return npc.getWeaponSkillLevel(PlayerWeaponSkill(skill));
}

SCRIPT_API(NPC_MeleeAttack, bool(INPC& npc, int time, bool secondaryAttack))
{
	npc.meleeAttack(time, secondaryAttack);
	return true;
}

SCRIPT_API(NPC_StopMeleeAttack, bool(INPC& npc))
{
	npc.stopMeleeAttack();
	return true;
}

SCRIPT_API(NPC_IsMeleeAttacking, bool(INPC& npc))
{
	return npc.isMeleeAttacking();
}

SCRIPT_API(NPC_SetFightingStyle, bool(INPC& npc, int style))
{
	npc.setFightingStyle(PlayerFightingStyle(style));
	return true;
}

SCRIPT_API(NPC_GetFightingStyle, int(INPC& npc))
{
	return int(npc.getFightingStyle());
}

SCRIPT_API(NPC_EnableReloading, bool(INPC& npc, bool enable))
{
	npc.enableReloading(enable);
	return true;
}

SCRIPT_API(NPC_IsReloadEnabled, bool(INPC& npc))
{
	return npc.isReloadEnabled();
}

SCRIPT_API(NPC_IsReloading, bool(INPC& npc))
{
	return npc.isReloading();
}

SCRIPT_API(NPC_EnableInfiniteAmmo, bool(INPC& npc, bool enable))
{
	npc.enableInfiniteAmmo(enable);
	return true;
}

SCRIPT_API(NPC_IsInfiniteAmmoEnabled, bool(INPC& npc))
{
	return npc.isInfiniteAmmoEnabled();
}

SCRIPT_API(NPC_GetWeaponState, int(INPC& npc))
{
	return int(npc.getWeaponState());
}

SCRIPT_API(NPC_SetAmmoInClip, bool(INPC& npc, int ammo))
{
	npc.setAmmoInClip(ammo);
	return true;
}

SCRIPT_API(NPC_GetAmmoInClip, int(INPC& npc))
{
	return npc.getAmmoInClip();
}

SCRIPT_API(NPC_Shoot, bool(INPC& npc, uint8_t weapon, int hitId, int hitType, Vector3 endPoint, Vector3 offset, bool isHit, uint8_t checkInBetweenFlags))
{
	npc.shoot(hitId, PlayerBulletHitType(hitType), weapon, endPoint, offset, isHit, EntityCheckType(checkInBetweenFlags));
	return true;
}

SCRIPT_API(NPC_IsShooting, bool(INPC& npc))
{
	return npc.isShooting();
}

SCRIPT_API(NPC_AimAt, bool(INPC& npc, Vector3 point, bool shoot, int shootDelay, bool updateAngle, Vector3 offsetFrom, uint8_t checkInBetweenFlags))
{
	npc.aimAt(point, shoot, shootDelay, updateAngle, offsetFrom, EntityCheckType(checkInBetweenFlags));
	return true;
}

SCRIPT_API(NPC_AimAtPlayer, bool(INPC& npc, IPlayer& atPlayer, bool shoot, int shootDelay, bool updateAngle, Vector3 offset, Vector3 offsetFrom, uint8_t checkInBetweenFlags))
{
	npc.aimAtPlayer(atPlayer, shoot, shootDelay, updateAngle, offset, offsetFrom, EntityCheckType(checkInBetweenFlags));
	return true;
}

SCRIPT_API(NPC_StopAim, bool(INPC& npc))
{
	npc.stopAim();
	return true;
}

SCRIPT_API(NPC_IsAiming, bool(INPC& npc))
{
	return npc.isAiming();
}

SCRIPT_API(NPC_IsAimingAtPlayer, bool(INPC& npc, IPlayer& atPlayer))
{
	return npc.isAimingAtPlayer(atPlayer);
}

SCRIPT_API(NPC_SetWeaponAccuracy, bool(INPC& npc, int weapon, float accuracy))
{
	npc.setWeaponAccuracy(weapon, accuracy);
	return true;
}

SCRIPT_API(NPC_GetWeaponAccuracy, float(INPC& npc, int weapon))
{
	return npc.getWeaponAccuracy(weapon);
}

SCRIPT_API(NPC_SetWeaponReloadTime, bool(INPC& npc, int weapon, int time))
{
	npc.setWeaponReloadTime(weapon, time);
	return true;
}

SCRIPT_API(NPC_GetWeaponReloadTime, int(INPC& npc, int weapon))
{
	return npc.getWeaponReloadTime(weapon);
}

SCRIPT_API(NPC_GetWeaponActualReloadTime, int(INPC& npc, int weapon))
{
	return npc.getWeaponActualReloadTime(weapon);
}

SCRIPT_API(NPC_SetWeaponShootTime, bool(INPC& npc, int weapon, int time))
{
	npc.setWeaponShootTime(weapon, time);
	return true;
}

SCRIPT_API(NPC_GetWeaponShootTime, int(INPC& npc, int weapon))
{
	return npc.getWeaponShootTime(weapon);
}

SCRIPT_API(NPC_SetWeaponClipSize, bool(INPC& npc, int weapon, int size))
{
	npc.setWeaponClipSize(weapon, size);
	return true;
}

SCRIPT_API(NPC_GetWeaponClipSize, int(INPC& npc, int weapon))
{
	return npc.getWeaponClipSize(weapon);
}

SCRIPT_API(NPC_GetWeaponActualClipSize, int(INPC& npc, int weapon))
{
	return npc.getWeaponActualClipSize(weapon);
}

SCRIPT_API(NPC_EnterVehicle, bool(INPC& npc, IVehicle& vehicle, int seatId, int moveType))
{
	npc.enterVehicle(vehicle, seatId, NPCMoveType(moveType));
	return true;
}

SCRIPT_API(NPC_ExitVehicle, bool(INPC& npc))
{
	npc.exitVehicle();
	return true;
}

SCRIPT_API(NPC_PutInVehicle, bool(INPC& npc, IVehicle& vehicle, int seatId))
{
	npc.putInVehicle(vehicle, seatId);
	return true;
}

SCRIPT_API(NPC_RemoveFromVehicle, bool(INPC& npc))
{
	npc.removeFromVehicle();
	return true;
}

SCRIPT_API(NPC_CreatePath, int())
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->createPath();
	}
	return false;
}

SCRIPT_API(NPC_DestroyPath, bool(int pathId))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->destroyPath(pathId);
	}
	return false;
}

SCRIPT_API(NPC_DestroyAllPath, bool())
{
	if (PawnManager::Get()->npcs)
	{
		PawnManager::Get()->npcs->destroyAllPaths();
		return true;
	}
	return false;
}

SCRIPT_API(NPC_GetPathCount, int())
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->getPathCount();
	}
	return -1;
}

SCRIPT_API(NPC_AddPointToPath, bool(int pathId, Vector3 position, float stopRange))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->addPointToPath(pathId, position, stopRange);
	}
	return false;
}

SCRIPT_API(NPC_RemovePointFromPath, bool(int pathId, int pointIndex))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->removePointFromPath(pathId, pointIndex);
	}
	return false;
}

SCRIPT_API(NPC_ClearPath, bool(int pathId))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->clearPath(pathId);
	}
	return false;
}

SCRIPT_API(NPC_GetPathPointCount, int(int pathId))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->getPathPointCount(pathId);
	}
	return -1;
}

SCRIPT_API(NPC_GetPathPoint, bool(int pathId, int pointIndex, Vector3& position, float& stopRange))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->getPathPoint(pathId, pointIndex, position, stopRange);
	}
	return false;
}

SCRIPT_API(NPC_GetCurrentPathPointIndex, int(INPC& npc))
{
	return npc.getCurrentPathPointIndex();
}

SCRIPT_API(NPC_IsValidPath, bool(int pathId))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->isValidPath(pathId);
	}
	return false;
}

SCRIPT_API(NPC_HasPathPointInRange, bool(int pathId, Vector3 position, float radius))
{
	if (PawnManager::Get()->npcs)
	{
		return PawnManager::Get()->npcs->hasPathPointInRange(pathId, position, radius);
	}
	return false;
}

SCRIPT_API(NPC_MoveByPath, bool(INPC& npc, int pathId, int moveType, float moveSpeed, bool reverse))
{
	return npc.moveByPath(pathId, NPCMoveType(moveType), moveSpeed, reverse);
}

SCRIPT_API_FAILRET(NPC_GetVehicle, INVALID_VEHICLE_ID, int(INPC& npc))
{
	auto vehicle = npc.getVehicle();
	if (vehicle)
	{
		return vehicle->getID();
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API_FAILRET(NPC_GetVehicleID, INVALID_VEHICLE_ID, int(INPC& npc))
{
	auto vehicle = npc.getVehicle();
	if (vehicle)
	{
		return vehicle->getID();
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API_FAILRET(NPC_GetEnteringVehicle, INVALID_VEHICLE_ID, int(INPC& npc))
{
	auto vehicle = npc.getEnteringVehicle();
	if (vehicle)
	{
		return vehicle->getID();
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API_FAILRET(NPC_GetEnteringVehicleID, INVALID_VEHICLE_ID, int(INPC& npc))
{
	auto vehicle = npc.getEnteringVehicle();
	if (vehicle)
	{
		return vehicle->getID();
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API(NPC_GetVehicleSeat, int(INPC& npc))
{
	return npc.getVehicleSeat();
}

SCRIPT_API(NPC_GetEnteringVehicleSeat, int(INPC& npc))
{
	return npc.getEnteringVehicleSeat();
}

SCRIPT_API(NPC_IsEnteringVehicle, bool(INPC& npc))
{
	return npc.getEnteringVehicle() && npc.getVehicleSeat() != SEAT_NONE;
}

SCRIPT_API(NPC_UseVehicleSiren, bool(INPC& npc, bool use))
{
	npc.useVehicleSiren(use);
	return true;
}

SCRIPT_API(NPC_IsVehicleSirenUsed, bool(INPC& npc))
{
	return npc.isVehicleSirenUsed();
}

SCRIPT_API(NPC_SetVehicleHealth, bool(INPC& npc, float health))
{
	npc.setVehicleHealth(health);
	return true;
}

SCRIPT_API(NPC_GetVehicleHealth, float(INPC& npc))
{
	return npc.getVehicleHealth();
}

SCRIPT_API(NPC_SetVehicleHydraThrusters, bool(INPC& npc, int direction))
{
	npc.setVehicleHydraThrusters(direction);
	return true;
}

SCRIPT_API(NPC_GetVehicleHydraThrusters, int(INPC& npc))
{
	return npc.getVehicleHydraThrusters();
}

SCRIPT_API(NPC_SetVehicleGearState, bool(INPC& npc, int gearState))
{
	npc.setVehicleGearState(gearState);
	return true;
}

SCRIPT_API(NPC_GetVehicleGearState, int(INPC& npc))
{
	return npc.getVehicleGearState();
}

SCRIPT_API(NPC_SetVehicleTrainSpeed, bool(INPC& npc, float speed))
{
	npc.setVehicleTrainSpeed(speed);
	return true;
}

SCRIPT_API(NPC_GetVehicleTrainSpeed, float(INPC& npc))
{
	return npc.getVehicleTrainSpeed();
}

SCRIPT_API(NPC_ResetAnimation, bool(INPC& npc))
{
	npc.resetAnimation();
	return true;
}

SCRIPT_API(NPC_SetAnimation, bool(INPC& npc, int animationId, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time))
{
	npc.setAnimation(animationId, delta, loop, lockX, lockY, freeze, time);
	return true;
}

SCRIPT_API(NPC_GetAnimation, bool(INPC& npc, int& animationId, float& delta, bool& loop, bool& lockX, bool& lockY, bool& freeze, int& time))
{
	npc.getAnimation(animationId, delta, loop, lockX, lockY, freeze, time);
	return true;
}

SCRIPT_API(NPC_ApplyAnimation, bool(INPC& npc, const std::string& animlib, const std::string& animname, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time))
{
	const AnimationData animationData(delta, loop, lockX, lockY, freeze, time, animlib, animname);
	npc.applyAnimation(animationData);
	return true;
}

SCRIPT_API(NPC_ClearAnimations, bool(INPC& npc))
{
	npc.clearAnimations();
	return true;
}

SCRIPT_API(NPC_SetSpecialAction, bool(INPC& npc, int action))
{
	npc.setSpecialAction(PlayerSpecialAction(action));
	return true;
}

SCRIPT_API(NPC_GetSpecialAction, int(INPC& npc))
{
	return npc.getSpecialAction();
}

SCRIPT_API(NPC_StartPlayback, bool(INPC& npc, const std::string& recordName, bool autoUnload, Vector3 startPos, Vector3 startRot))
{
	return npc.startPlayback(recordName, autoUnload, startPos, startRot);
}

SCRIPT_API(NPC_StartPlaybackEx, bool(INPC& npc, int recordId, bool autoUnload, Vector3 startPos, Vector3 startRot))
{
	return npc.startPlayback(recordId, autoUnload, startPos, startRot);
}

SCRIPT_API(NPC_StopPlayback, bool(INPC& npc))
{
	npc.stopPlayback();
	return true;
}

SCRIPT_API(NPC_PausePlayback, bool(INPC& npc, bool paused))
{
	npc.pausePlayback(paused);
	return true;
}

SCRIPT_API(NPC_IsPlayingPlayback, bool(INPC& npc))
{
	return npc.isPlayingPlayback();
}

SCRIPT_API(NPC_IsPlaybackPaused, bool(INPC& npc))
{
	return npc.isPlaybackPaused();
}

SCRIPT_API(NPC_LoadRecord, int(const std::string& filePath))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->loadRecord(filePath);
	}
	return -1;
}

SCRIPT_API(NPC_UnloadRecord, bool(int recordId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->unloadRecord(recordId);
	}
	return false;
}

SCRIPT_API(NPC_IsValidRecord, bool(int recordId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->isValidRecord(recordId);
	}
	return false;
}

SCRIPT_API(NPC_GetRecordCount, int())
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return static_cast<int>(component->getRecordCount());
	}
	return 0;
}

SCRIPT_API(NPC_UnloadAllRecords, bool())
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		component->unloadAllRecords();
		return true;
	}
	return false;
}

SCRIPT_API(NPC_OpenNode, bool(int nodeId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->openNode(nodeId);
	}
	return false;
}

SCRIPT_API(NPC_CloseNode, bool(int nodeId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		component->closeNode(nodeId);
		return true;
	}
	return false;
}

SCRIPT_API(NPC_IsNodeOpen, bool(int nodeId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->isNodeOpen(nodeId);
	}
	return false;
}

SCRIPT_API(NPC_GetNodeType, int(int nodeId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->getNodeType(nodeId);
	}
	return 0;
}

SCRIPT_API(NPC_SetNodePoint, bool(int nodeId, int pointId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->setNodePoint(nodeId, static_cast<uint16_t>(pointId));
	}
	return false;
}

SCRIPT_API(NPC_GetNodePointPosition, bool(int nodeId, float& x, float& y, float& z))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		Vector3 position;
		if (component->getNodePointPosition(nodeId, position))
		{
			x = position.x;
			y = position.y;
			z = position.z;
			return true;
		}
	}
	return false;
}

SCRIPT_API(NPC_GetNodePointCount, int(int nodeId))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		return component->getNodePointCount(nodeId);
	}
	return 0;
}

SCRIPT_API(NPC_GetNodeInfo, bool(int nodeId, int& vehicleNodes, int& pedNodes, int& naviNodes))
{
	auto component = PawnManager::Get()->npcs;
	if (component)
	{
		uint32_t vehNodes, pedNodes32, naviNodes32;
		if (component->getNodeInfo(nodeId, vehNodes, pedNodes32, naviNodes32))
		{
			vehicleNodes = static_cast<int>(vehNodes);
			pedNodes = static_cast<int>(pedNodes32);
			naviNodes = static_cast<int>(naviNodes32);
			return true;
		}
	}
	return false;
}

SCRIPT_API(NPC_PlayNode, bool(INPC& npc, int nodeId, int moveType, float moveSpeed, float radius, bool setAngle))
{
	return npc.playNode(nodeId, static_cast<NPCMoveType>(moveType), moveSpeed, radius, setAngle);
}

SCRIPT_API(NPC_StopPlayingNode, bool(INPC& npc))
{
	npc.stopPlayingNode();
	return true;
}

SCRIPT_API(NPC_PausePlayingNode, bool(INPC& npc))
{
	npc.pausePlayingNode();
	return true;
}

SCRIPT_API(NPC_ResumePlayingNode, bool(INPC& npc))
{
	npc.resumePlayingNode();
	return true;
}

SCRIPT_API(NPC_IsPlayingNodePaused, bool(INPC& npc))
{
	return npc.isPlayingNodePaused();
}

SCRIPT_API(NPC_IsPlayingNode, bool(INPC& npc))
{
	return npc.isPlayingNode();
}

SCRIPT_API(NPC_ChangeNode, int(INPC& npc, int nodeId, int linkId))
{
	return static_cast<int>(npc.changeNode(nodeId, static_cast<uint16_t>(linkId)));
}

SCRIPT_API(NPC_UpdateNodePoint, bool(INPC& npc, int pointId))
{
	return npc.updateNodePoint(static_cast<uint16_t>(pointId));
}

SCRIPT_API(NPC_SetInvulnerable, bool(INPC& npc, bool toggle))
{
	npc.setInvulnerable(toggle);
	return true;
}

SCRIPT_API(NPC_IsInvulnerable, bool(INPC& npc))
{
	return npc.isInvulnerable();
}

SCRIPT_API(NPC_SetSurfingOffsets, bool(INPC& npc, Vector3 offset))
{
	auto data = npc.getSurfingData();
	data.offset = offset;
	npc.setSurfingData(data);
	return true;
}

SCRIPT_API(NPC_GetSurfingOffsets, bool(INPC& npc, Vector3& offset))
{
	auto data = npc.getSurfingData();
	offset = data.offset;
	return true;
}

SCRIPT_API(NPC_SetSurfingVehicle, bool(INPC& npc, IVehicle& vehicle))
{
	auto data = npc.getSurfingData();
	data.ID = vehicle.getID();
	data.type = PlayerSurfingData::Type::Vehicle;
	npc.setSurfingData(data);
	return true;
}

SCRIPT_API_FAILRET(NPC_GetSurfingVehicle, INVALID_VEHICLE_ID, int(INPC& npc))
{
	auto data = npc.getSurfingData();
	if (data.type != PlayerSurfingData::Type::Vehicle || PawnManager::Get()->vehicles == nullptr)
	{
		return INVALID_VEHICLE_ID;
	}

	return data.ID;
}

SCRIPT_API(NPC_SetSurfingObject, bool(INPC& npc, IObject& object))
{
	auto data = npc.getSurfingData();
	data.ID = object.getID();
	data.type = PlayerSurfingData::Type::Object;
	npc.setSurfingData(data);
	return true;
}

SCRIPT_API_FAILRET(NPC_GetSurfingObject, INVALID_OBJECT_ID, int(INPC& npc))
{
	auto data = npc.getSurfingData();
	if (data.type != PlayerSurfingData::Type::Object || PawnManager::Get()->objects == nullptr)
	{
		return INVALID_OBJECT_ID;
	}

	return data.ID;
}

SCRIPT_API(NPC_SetSurfingPlayerObject, bool(INPC& npc, int objectId))
{
	auto playerObjects = queryExtension<IPlayerObjectData>(npc.getPlayer());
	if (playerObjects)
	{
		auto object = playerObjects->get(objectId);
		if (object)
		{
			auto data = npc.getSurfingData();
			data.ID = object->getID();
			data.type = PlayerSurfingData::Type::PlayerObject;
			npc.setSurfingData(data);
			return true;
		}
	}

	return false;
}

SCRIPT_API_FAILRET(NPC_GetSurfingPlayerObject, INVALID_OBJECT_ID, int(INPC& npc))
{
	auto data = npc.getSurfingData();
	auto playerObjects = queryExtension<IPlayerObjectData>(npc.getPlayer());
	if (data.type != PlayerSurfingData::Type::PlayerObject || PawnManager::Get()->objects == nullptr || playerObjects == nullptr)
	{
		return INVALID_OBJECT_ID;
	}

	return data.ID;
}

SCRIPT_API(NPC_ResetSurfingData, bool(INPC& npc))
{
	npc.resetSurfingData();
	return true;
}

SCRIPT_API(NPC_IsSpawned, bool(INPC& npc))
{
	auto player = npc.getPlayer();
	if (player)
	{
		auto state = player->getState();
		if (state == PlayerState_OnFoot || state == PlayerState_Driver || state == PlayerState_Passenger || state == PlayerState_Spawned)
		{
			return true;
		}
	}
	return false;
}

SCRIPT_API(NPC_Kill, bool(INPC& npc, IPlayer* killer, int reason))
{
	npc.kill(killer, reason);
	return true;
}

SCRIPT_API(NPC_SetVelocity, bool(INPC& npc, Vector3 velocity))
{
	npc.setVelocity(velocity, true);
	return true;
}

SCRIPT_API(NPC_GetVelocity, bool(INPC& npc, Vector3& velocity))
{
	velocity = npc.getVelocity();
	return true;
}

SCRIPT_API(NPC_GetPlayerAimingAt, int(INPC& npc))
{
	auto player = npc.getPlayerAimingAt();
	return player ? player->getID() : INVALID_PLAYER_ID;
}

SCRIPT_API(NPC_GetPlayerMovingTo, int(INPC& npc))
{
	auto player = npc.getPlayerMovingTo();
	return player ? player->getID() : INVALID_PLAYER_ID;
}

SCRIPT_API(NPC_SetWeaponState, bool(INPC& npc, int weaponState))
{
	npc.setWeaponState(PlayerWeaponState(weaponState));
	return true;
}

SCRIPT_API(NPC_GetPosMovingTo, bool(INPC& npc, Vector3& position))
{
	position = npc.getPositionMovingTo();
	return true;
}

SCRIPT_API(NPC_SetAngleToPos, bool(INPC& npc, Vector3 position))
{
	auto vec = position - npc.getPosition();
	auto angle = getAngleOfLine(vec.x, vec.z);
	openmp_scripting::NPC_SetFacingAngle(npc, angle);
	return true;
}

SCRIPT_API(NPC_SetAngleToPlayer, bool(INPC& npc, IPlayer& player))
{
	openmp_scripting::NPC_SetAngleToPos(npc, player.getPosition());
	return true;
}
