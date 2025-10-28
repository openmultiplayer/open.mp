/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"
#include <Server/Components/NPCs/npcs.hpp>

OMP_CAPI(NPC_Create, objectPtr(StringCharPtr name, int* id))
{
	COMPONENT_CHECK_RET(npcs, nullptr);
	if (name)
	{
		auto npc = npcs->create(name);
		if (npc)
		{
			*id = npc->getID();
			return npc;
		}
	}
	return nullptr;
}

OMP_CAPI(NPC_Destroy, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npcs->destroy(*npc_);
	return true;
}

OMP_CAPI(NPC_FromID, objectPtr(int npcid))
{
	COMPONENT_CHECK_RET(npcs, nullptr);
	return npcs->get(npcid);
}

OMP_CAPI(NPC_GetID, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, INVALID_PLAYER_ID);
	return npc_->getID();
}

OMP_CAPI(NPC_IsValid, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npcs->get(npc_->getID()) != nullptr;
}

OMP_CAPI(NPC_GetPlayer, objectPtr(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, nullptr);
	return npc_->getPlayer();
}

OMP_CAPI(NPC_Spawn, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->spawn();
	return true;
}

OMP_CAPI(NPC_Respawn, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->respawn();
	return true;
}

OMP_CAPI(NPC_SetPos, bool(objectPtr npc, float x, float y, float z))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setPosition(Vector3(x, y, z), true);
	return true;
}

OMP_CAPI(NPC_GetPos, bool(objectPtr npc, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	Vector3 position = npc_->getPosition();
	*x = position.x;
	*y = position.y;
	*z = position.z;
	return true;
}

OMP_CAPI(NPC_SetRot, bool(objectPtr npc, float rx, float ry, float rz))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setRotation(Vector3(rx, ry, rz), true);
	return true;
}

OMP_CAPI(NPC_GetRot, bool(objectPtr npc, float* rx, float* ry, float* rz))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	Vector3 rotation = npc_->getRotation().ToEuler();
	*rx = rotation.x;
	*ry = rotation.y;
	*rz = rotation.z;
	return true;
}

OMP_CAPI(NPC_SetFacingAngle, bool(objectPtr npc, float angle))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto rotation = npc_->getRotation().ToEuler();
	rotation.z = angle;
	npc_->setRotation(rotation, true);
	return true;
}

OMP_CAPI(NPC_GetFacingAngle, bool(objectPtr npc, float* angle))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto rotation = npc_->getRotation().ToEuler();
	*angle = rotation.z;
	return true;
}

OMP_CAPI(NPC_SetVirtualWorld, bool(objectPtr npc, int virtualWorld))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setVirtualWorld(virtualWorld);
	return true;
}

OMP_CAPI(NPC_GetVirtualWorld, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getVirtualWorld();
}

OMP_CAPI(NPC_SetInterior, bool(objectPtr npc, int interior))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setInterior(interior);
	return true;
}

OMP_CAPI(NPC_GetInterior, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getInterior();
}

OMP_CAPI(NPC_Move, bool(objectPtr npc, float x, float y, float z, int moveType, float moveSpeed, float stopRange))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->move(Vector3(x, y, z), NPCMoveType(moveType), moveSpeed, stopRange);
}

OMP_CAPI(NPC_MoveToPlayer, bool(objectPtr npc, objectPtr player, int moveType, float moveSpeed, float stopRange, int posCheckUpdateDelay, bool autoRestart))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->moveToPlayer(*player_, NPCMoveType(moveType), moveSpeed, stopRange, Milliseconds(posCheckUpdateDelay), autoRestart);
}

OMP_CAPI(NPC_StopMove, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->stopMove();
	return true;
}

OMP_CAPI(NPC_IsMoving, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isMoving();
}

OMP_CAPI(NPC_SetSkin, bool(objectPtr npc, int model))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setSkin(model);
	return true;
}

OMP_CAPI(NPC_IsStreamedIn, bool(objectPtr npc, objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isStreamedInForPlayer(*player_);
}

OMP_CAPI(NPC_IsAnyStreamedIn, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto streamedIn = npc_->streamedForPlayers();
	return streamedIn.size() > 1;
}

OMP_CAPI(NPC_GetAll, int(int* npcsArr, int maxNPCs))
{
	COMPONENT_CHECK_RET(npcs, 0);
	if (!npcsArr)
	{
		return 0;
	}

	int index = 0;
	for (INPC* npc : *npcs)
	{
		if (index >= maxNPCs)
		{
			break;
		}
		npcsArr[index] = npc->getID();
		index++;
	}
	return index;
}

OMP_CAPI(NPC_SetHealth, bool(objectPtr npc, float health))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setHealth(health);
	return true;
}

OMP_CAPI(NPC_GetHealth, float(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0.0f);
	return npc_->getHealth();
}

OMP_CAPI(NPC_SetArmour, bool(objectPtr npc, float armour))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setArmour(armour);
	return true;
}

OMP_CAPI(NPC_GetArmour, float(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0.0f);
	return npc_->getArmour();
}

OMP_CAPI(NPC_IsDead, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isDead();
}

OMP_CAPI(NPC_SetInvulnerable, bool(objectPtr npc, bool toggle))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setInvulnerable(toggle);
	return true;
}

OMP_CAPI(NPC_IsInvulnerable, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isInvulnerable();
}

OMP_CAPI(NPC_SetWeapon, bool(objectPtr npc, uint8_t weapon))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setWeapon(weapon);
	return true;
}

OMP_CAPI(NPC_GetWeapon, uint8_t(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getWeapon();
}

OMP_CAPI(NPC_SetAmmo, bool(objectPtr npc, int ammo))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setAmmo(ammo);
	return true;
}

OMP_CAPI(NPC_GetAmmo, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getAmmo();
}

OMP_CAPI(NPC_SetAmmoInClip, bool(objectPtr npc, int ammo))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setAmmoInClip(ammo);
	return true;
}

OMP_CAPI(NPC_GetAmmoInClip, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getAmmoInClip();
}

OMP_CAPI(NPC_EnableReloading, bool(objectPtr npc, bool enable))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->enableReloading(enable);
	return true;
}

OMP_CAPI(NPC_IsReloadEnabled, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isReloadEnabled();
}

OMP_CAPI(NPC_IsReloading, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isReloading();
}

OMP_CAPI(NPC_EnableInfiniteAmmo, bool(objectPtr npc, bool enable))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->enableInfiniteAmmo(enable);
	return true;
}

OMP_CAPI(NPC_IsInfiniteAmmoEnabled, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isInfiniteAmmoEnabled();
}

OMP_CAPI(NPC_GetWeaponState, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return int(npc_->getWeaponState());
}

OMP_CAPI(NPC_SetKeys, bool(objectPtr npc, uint16_t upAndDown, uint16_t leftAndRight, uint16_t keys))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setKeys(upAndDown, leftAndRight, keys);
	return true;
}

OMP_CAPI(NPC_GetKeys, bool(objectPtr npc, uint16_t* upAndDown, uint16_t* leftAndRight, uint16_t* keys))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	uint16_t ud, lr, k;
	npc_->getKeys(ud, lr, k);
	if (upAndDown)
	{
		*upAndDown = ud;
	}
	if (leftAndRight)
	{
		*leftAndRight = lr;
	}
	if (keys)
	{
		*keys = k;
	}
	return true;
}

OMP_CAPI(NPC_SetWeaponSkillLevel, bool(objectPtr npc, uint8_t skill, int level))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setWeaponSkillLevel(PlayerWeaponSkill(skill), level);
	return true;
}

OMP_CAPI(NPC_GetWeaponSkillLevel, int(objectPtr npc, int skill))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getWeaponSkillLevel(PlayerWeaponSkill(skill));
}

OMP_CAPI(NPC_MeleeAttack, bool(objectPtr npc, int time, bool secondaryAttack))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->meleeAttack(time, secondaryAttack);
	return true;
}

OMP_CAPI(NPC_StopMeleeAttack, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->stopMeleeAttack();
	return true;
}

OMP_CAPI(NPC_IsMeleeAttacking, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isMeleeAttacking();
}

OMP_CAPI(NPC_SetFightingStyle, bool(objectPtr npc, int style))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setFightingStyle(PlayerFightingStyle(style));
	return true;
}

OMP_CAPI(NPC_GetFightingStyle, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return int(npc_->getFightingStyle());
}

OMP_CAPI(NPC_Shoot, bool(objectPtr npc, uint8_t weapon, int hitId, int hitType, float endX, float endY, float endZ, float offsetX, float offsetY, float offsetZ, bool isHit, uint8_t checkInBetweenFlags))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->shoot(hitId, PlayerBulletHitType(hitType), weapon, Vector3(endX, endY, endZ), Vector3(offsetX, offsetY, offsetZ), isHit, EntityCheckType(checkInBetweenFlags));
	return true;
}

OMP_CAPI(NPC_IsShooting, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isShooting();
}

OMP_CAPI(NPC_AimAt, bool(objectPtr npc, float x, float y, float z, bool shoot, int shootDelay, bool updateAngle, float offsetFromX, float offsetFromY, float offsetFromZ, uint8_t checkInBetweenFlags))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->aimAt(Vector3(x, y, z), shoot, shootDelay, updateAngle, Vector3(offsetFromX, offsetFromY, offsetFromZ), EntityCheckType(checkInBetweenFlags));
	return true;
}

OMP_CAPI(NPC_AimAtPlayer, bool(objectPtr npc, objectPtr atPlayer, bool shoot, int shootDelay, bool updateAngle, float offsetX, float offsetY, float offsetZ, float offsetFromX, float offsetFromY, float offsetFromZ, uint8_t checkInBetweenFlags))
{
	POOL_ENTITY_RET(players, IPlayer, atPlayer, atPlayer_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->aimAtPlayer(*atPlayer_, shoot, shootDelay, updateAngle, Vector3(offsetX, offsetY, offsetZ), Vector3(offsetFromX, offsetFromY, offsetFromZ), EntityCheckType(checkInBetweenFlags));
	return true;
}

OMP_CAPI(NPC_StopAim, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->stopAim();
	return true;
}

OMP_CAPI(NPC_IsAiming, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isAiming();
}

OMP_CAPI(NPC_IsAimingAtPlayer, bool(objectPtr npc, objectPtr atPlayer))
{
	POOL_ENTITY_RET(players, IPlayer, atPlayer, atPlayer_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isAimingAtPlayer(*atPlayer_);
}

OMP_CAPI(NPC_SetWeaponAccuracy, bool(objectPtr npc, int weapon, float accuracy))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setWeaponAccuracy(weapon, accuracy);
	return true;
}

OMP_CAPI(NPC_GetWeaponAccuracy, float(objectPtr npc, int weapon))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0.0f);
	return npc_->getWeaponAccuracy(weapon);
}

OMP_CAPI(NPC_SetWeaponReloadTime, bool(objectPtr npc, int weapon, int time))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setWeaponReloadTime(weapon, time);
	return true;
}

OMP_CAPI(NPC_GetWeaponReloadTime, int(objectPtr npc, int weapon))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getWeaponReloadTime(weapon);
}

OMP_CAPI(NPC_GetWeaponActualReloadTime, int(objectPtr npc, int weapon))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getWeaponActualReloadTime(weapon);
}

OMP_CAPI(NPC_SetWeaponShootTime, bool(objectPtr npc, int weapon, int time))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setWeaponShootTime(weapon, time);
	return true;
}

OMP_CAPI(NPC_GetWeaponShootTime, int(objectPtr npc, int weapon))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getWeaponShootTime(weapon);
}

OMP_CAPI(NPC_SetWeaponClipSize, bool(objectPtr npc, int weapon, int size))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setWeaponClipSize(weapon, size);
	return true;
}

OMP_CAPI(NPC_GetWeaponClipSize, int(objectPtr npc, int weapon))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getWeaponClipSize(weapon);
}

OMP_CAPI(NPC_GetWeaponActualClipSize, int(objectPtr npc, int weapon))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getWeaponActualClipSize(weapon);
}

OMP_CAPI(NPC_EnterVehicle, bool(objectPtr npc, objectPtr vehicle, int seatId, int moveType))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->enterVehicle(*vehicle_, seatId, NPCMoveType(moveType));
	return true;
}

OMP_CAPI(NPC_ExitVehicle, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->exitVehicle();
	return true;
}

OMP_CAPI(NPC_PutInVehicle, bool(objectPtr npc, objectPtr vehicle, int seatId))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->putInVehicle(*vehicle_, seatId);
	return true;
}

OMP_CAPI(NPC_RemoveFromVehicle, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->removeFromVehicle();
	return true;
}

OMP_CAPI(NPC_GetVehicle, objectPtr(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, nullptr);
	return npc_->getVehicle();
}

OMP_CAPI(NPC_GetVehicleID, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, INVALID_VEHICLE_ID);
	auto vehicle = npc_->getVehicle();
	if (vehicle)
	{
		return vehicle->getID();
	}
	return INVALID_VEHICLE_ID;
}

OMP_CAPI(NPC_GetEnteringVehicle, objectPtr(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, nullptr);
	return npc_->getEnteringVehicle();
}

OMP_CAPI(NPC_GetEnteringVehicleID, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, INVALID_VEHICLE_ID);
	auto vehicle = npc_->getEnteringVehicle();
	if (vehicle)
	{
		return vehicle->getID();
	}
	return INVALID_VEHICLE_ID;
}

OMP_CAPI(NPC_GetVehicleSeat, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, SEAT_NONE);
	return npc_->getVehicleSeat();
}

OMP_CAPI(NPC_GetEnteringVehicleSeat, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, SEAT_NONE);
	return npc_->getEnteringVehicleSeat();
}

OMP_CAPI(NPC_IsEnteringVehicle, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->getEnteringVehicle() && npc_->getVehicleSeat() != SEAT_NONE;
}

OMP_CAPI(NPC_UseVehicleSiren, bool(objectPtr npc, bool use))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->useVehicleSiren(use);
	return true;
}

OMP_CAPI(NPC_IsVehicleSirenUsed, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isVehicleSirenUsed();
}

OMP_CAPI(NPC_SetVehicleHealth, bool(objectPtr npc, float health))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setVehicleHealth(health);
	return true;
}

OMP_CAPI(NPC_GetVehicleHealth, float(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0.0f);
	return npc_->getVehicleHealth();
}

OMP_CAPI(NPC_SetVehicleHydraThrusters, bool(objectPtr npc, int direction))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setVehicleHydraThrusters(direction);
	return true;
}

OMP_CAPI(NPC_GetVehicleHydraThrusters, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getVehicleHydraThrusters();
}

OMP_CAPI(NPC_SetVehicleGearState, bool(objectPtr npc, int gearState))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setVehicleGearState(gearState);
	return true;
}

OMP_CAPI(NPC_GetVehicleGearState, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getVehicleGearState();
}

OMP_CAPI(NPC_SetVehicleTrainSpeed, bool(objectPtr npc, float speed))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setVehicleTrainSpeed(speed);
	return true;
}

OMP_CAPI(NPC_GetVehicleTrainSpeed, float(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0.0f);
	return npc_->getVehicleTrainSpeed();
}

OMP_CAPI(NPC_CreatePath, int())
{
	COMPONENT_CHECK_RET(npcs, -1);
	return npcs->createPath();
}

OMP_CAPI(NPC_DestroyPath, bool(int pathId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->destroyPath(pathId);
}

OMP_CAPI(NPC_DestroyAllPath, bool())
{
	COMPONENT_CHECK_RET(npcs, false);
	npcs->destroyAllPaths();
	return true;
}

OMP_CAPI(NPC_GetPathCount, int())
{
	COMPONENT_CHECK_RET(npcs, -1);
	return npcs->getPathCount();
}

OMP_CAPI(NPC_AddPointToPath, bool(int pathId, float x, float y, float z, float stopRange))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->addPointToPath(pathId, Vector3(x, y, z), stopRange);
}

OMP_CAPI(NPC_RemovePointFromPath, bool(int pathId, int pointIndex))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->removePointFromPath(pathId, pointIndex);
}

OMP_CAPI(NPC_ClearPath, bool(int pathId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->clearPath(pathId);
}

OMP_CAPI(NPC_GetPathPointCount, int(int pathId))
{
	COMPONENT_CHECK_RET(npcs, -1);
	return npcs->getPathPointCount(pathId);
}

OMP_CAPI(NPC_GetPathPoint, bool(int pathId, int pointIndex, float* x, float* y, float* z, float* stopRange))
{
	COMPONENT_CHECK_RET(npcs, false);
	Vector3 position;
	float sr;
	if (npcs->getPathPoint(pathId, pointIndex, position, sr))
	{
		*x = position.x;
		*y = position.y;
		*z = position.z;
		*stopRange = sr;
		return true;
	}
	return false;
}

OMP_CAPI(NPC_GetCurrentPathPointIndex, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, -1);
	return npc_->getCurrentPathPointIndex();
}

OMP_CAPI(NPC_IsValidPath, bool(int pathId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->isValidPath(pathId);
}

OMP_CAPI(NPC_HasPathPointInRange, bool(int pathId, float x, float y, float z, float radius))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->hasPathPointInRange(pathId, Vector3(x, y, z), radius);
}

OMP_CAPI(NPC_MoveByPath, bool(objectPtr npc, int pathId, int moveType, float moveSpeed, bool reverse))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->moveByPath(pathId, NPCMoveType(moveType), moveSpeed, reverse);
}

OMP_CAPI(NPC_ResetAnimation, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->resetAnimation();
	return true;
}

OMP_CAPI(NPC_SetAnimation, bool(objectPtr npc, int animationId, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setAnimation(animationId, delta, loop, lockX, lockY, freeze, time);
	return true;
}

OMP_CAPI(NPC_GetAnimation, bool(objectPtr npc, int* animationId, float* delta, bool* loop, bool* lockX, bool* lockY, bool* freeze, int* time))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	int aId;
	float d;
	bool l, lx, ly, f;
	int t;
	npc_->getAnimation(aId, d, l, lx, ly, f, t);
	*animationId = aId;
	*delta = d;
	*loop = l;
	*lockX = lx;
	*lockY = ly;
	*freeze = f;
	*time = t;
	return true;
}

OMP_CAPI(NPC_ApplyAnimation, bool(objectPtr npc, StringCharPtr animlib, StringCharPtr animname, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	if (!animlib || !animname)
	{
		return false;
	}
	const AnimationData animationData(delta, loop, lockX, lockY, freeze, time, animlib, animname);
	npc_->applyAnimation(animationData);
	return true;
}

OMP_CAPI(NPC_ClearAnimations, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->clearAnimations();
	return true;
}

OMP_CAPI(NPC_SetSpecialAction, bool(objectPtr npc, int action))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->setSpecialAction(PlayerSpecialAction(action));
	return true;
}

OMP_CAPI(NPC_GetSpecialAction, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, 0);
	return npc_->getSpecialAction();
}

OMP_CAPI(NPC_StartPlayback, bool(objectPtr npc, StringCharPtr recordName, bool autoUnload, float startPosX, float startPosY, float startPosZ, float startRotX, float startRotY, float startRotZ))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	if (!recordName)
	{
		return false;
	}
	return npc_->startPlayback(recordName, autoUnload, Vector3(startPosX, startPosY, startPosZ), Vector3(startRotX, startRotY, startRotZ));
}

OMP_CAPI(NPC_StartPlaybackEx, bool(objectPtr npc, int recordId, bool autoUnload, float startPosX, float startPosY, float startPosZ, float startRotX, float startRotY, float startRotZ))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->startPlayback(recordId, autoUnload, Vector3(startPosX, startPosY, startPosZ), Vector3(startRotX, startRotY, startRotZ));
}

OMP_CAPI(NPC_StopPlayback, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->stopPlayback();
	return true;
}

OMP_CAPI(NPC_PausePlayback, bool(objectPtr npc, bool paused))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->pausePlayback(paused);
	return true;
}

OMP_CAPI(NPC_IsPlayingPlayback, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isPlayingPlayback();
}

OMP_CAPI(NPC_IsPlaybackPaused, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isPlaybackPaused();
}

OMP_CAPI(NPC_LoadRecord, int(StringCharPtr filePath))
{
	COMPONENT_CHECK_RET(npcs, -1);
	if (filePath)
	{
		return npcs->loadRecord(filePath);
	}
	return -1;
}

OMP_CAPI(NPC_UnloadRecord, bool(int recordId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->unloadRecord(recordId);
}

OMP_CAPI(NPC_IsValidRecord, bool(int recordId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->isValidRecord(recordId);
}

OMP_CAPI(NPC_GetRecordCount, int())
{
	COMPONENT_CHECK_RET(npcs, 0);
	return static_cast<int>(npcs->getRecordCount());
}

OMP_CAPI(NPC_UnloadAllRecords, bool())
{
	COMPONENT_CHECK_RET(npcs, false);
	npcs->unloadAllRecords();
	return true;
}

OMP_CAPI(NPC_OpenNode, bool(int nodeId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->openNode(nodeId);
}

OMP_CAPI(NPC_CloseNode, bool(int nodeId))
{
	COMPONENT_CHECK_RET(npcs, false);
	npcs->closeNode(nodeId);
	return true;
}

OMP_CAPI(NPC_IsNodeOpen, bool(int nodeId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->isNodeOpen(nodeId);
}

OMP_CAPI(NPC_GetNodeType, int(int nodeId))
{
	COMPONENT_CHECK_RET(npcs, 0);
	return npcs->getNodeType(nodeId);
}

OMP_CAPI(NPC_SetNodePoint, bool(int nodeId, int pointId))
{
	COMPONENT_CHECK_RET(npcs, false);
	return npcs->setNodePoint(nodeId, static_cast<uint16_t>(pointId));
}

OMP_CAPI(NPC_GetNodePointPosition, bool(int nodeId, float* x, float* y, float* z))
{
	COMPONENT_CHECK_RET(npcs, false);
	Vector3 position;
	if (npcs->getNodePointPosition(nodeId, position))
	{
		*x = position.x;
		*y = position.y;
		*z = position.z;
		return true;
	}
	return false;
}

OMP_CAPI(NPC_GetNodePointCount, int(int nodeId))
{
	COMPONENT_CHECK_RET(npcs, 0);
	return npcs->getNodePointCount(nodeId);
}

OMP_CAPI(NPC_GetNodeInfo, bool(int nodeId, uint32_t* vehicleNodes, uint32_t* pedNodes, uint32_t* naviNodes))
{
	COMPONENT_CHECK_RET(npcs, false);
	uint32_t vehNodes, pedNodes32, naviNodes32;
	if (npcs->getNodeInfo(nodeId, vehNodes, pedNodes32, naviNodes32))
	{
		*vehicleNodes = vehNodes;
		*pedNodes = pedNodes32;
		*naviNodes = naviNodes32;
		return true;
	}
	return false;
}

OMP_CAPI(NPC_PlayNode, bool(objectPtr npc, int nodeId, int moveType, float moveSpeed, float radius, bool setAngle))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->playNode(nodeId, static_cast<NPCMoveType>(moveType), moveSpeed, radius, setAngle);
}

OMP_CAPI(NPC_StopPlayingNode, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->stopPlayingNode();
	return true;
}

OMP_CAPI(NPC_PausePlayingNode, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->pausePlayingNode();
	return true;
}

OMP_CAPI(NPC_ResumePlayingNode, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->resumePlayingNode();
	return true;
}

OMP_CAPI(NPC_IsPlayingNodePaused, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isPlayingNodePaused();
}

OMP_CAPI(NPC_IsPlayingNode, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->isPlayingNode();
}

OMP_CAPI(NPC_ChangeNode, int(objectPtr npc, int nodeId, int linkId))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, -1);
	return static_cast<int>(npc_->changeNode(nodeId, static_cast<uint16_t>(linkId)));
}

OMP_CAPI(NPC_UpdateNodePoint, bool(objectPtr npc, int pointId))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	return npc_->updateNodePoint(static_cast<uint16_t>(pointId));
}

OMP_CAPI(NPC_SetSurfingOffset, bool(objectPtr npc, float x, float y, float z))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto data = npc_->getSurfingData();
	data.offset = Vector3(x, y, z);
	npc_->setSurfingData(data);
	return true;
}

OMP_CAPI(NPC_GetSurfingOffset, bool(objectPtr npc, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto data = npc_->getSurfingData();
	*x = data.offset.x;
	*y = data.offset.y;
	*z = data.offset.z;
	return true;
}

OMP_CAPI(NPC_SetSurfingVehicle, bool(objectPtr npc, objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto data = npc_->getSurfingData();
	data.ID = vehicle_->getID();
	data.type = PlayerSurfingData::Type::Vehicle;
	npc_->setSurfingData(data);
	return true;
}

OMP_CAPI(NPC_GetSurfingVehicle, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, INVALID_VEHICLE_ID);
	auto data = npc_->getSurfingData();
	auto vehicles = ComponentManager::Get()->vehicles;
	if (data.type != PlayerSurfingData::Type::Vehicle || vehicles == nullptr)
	{
		return INVALID_VEHICLE_ID;
	}
	return data.ID;
}

OMP_CAPI(NPC_SetSurfingObject, bool(objectPtr npc, objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto data = npc_->getSurfingData();
	data.ID = object_->getID();
	data.type = PlayerSurfingData::Type::Object;
	npc_->setSurfingData(data);
	return true;
}

OMP_CAPI(NPC_GetSurfingObject, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, INVALID_OBJECT_ID);
	auto data = npc_->getSurfingData();
	auto objects = ComponentManager::Get()->objects;
	if (data.type != PlayerSurfingData::Type::Object || objects == nullptr)
	{
		return INVALID_OBJECT_ID;
	}
	return data.ID;
}

OMP_CAPI(NPC_SetSurfingPlayerObject, bool(objectPtr npc, objectPtr player, int objectId))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	auto playerObjects = queryExtension<IPlayerObjectData>(*player_);
	if (playerObjects)
	{
		auto object = playerObjects->get(objectId);
		if (object)
		{
			auto data = npc_->getSurfingData();
			data.ID = object->getID();
			data.type = PlayerSurfingData::Type::PlayerObject;
			npc_->setSurfingData(data);
			return true;
		}
	}
	return false;
}

OMP_CAPI(NPC_GetSurfingPlayerObject, int(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, INVALID_OBJECT_ID);
	auto data = npc_->getSurfingData();
	auto objects = ComponentManager::Get()->objects;
	if (data.type != PlayerSurfingData::Type::PlayerObject || objects == nullptr)
	{
		return INVALID_OBJECT_ID;
	}
	return data.ID;
}

OMP_CAPI(NPC_ResetSurfingData, bool(objectPtr npc))
{
	POOL_ENTITY_RET(npcs, INPC, npc, npc_, false);
	npc_->resetSurfingData();
	return true;
}
