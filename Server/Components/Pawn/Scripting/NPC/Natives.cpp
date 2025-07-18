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
#include "../../format.hpp"

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
	PawnManager::Get()->npcs->destroy(npc);
	return true;
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

SCRIPT_API(NPC_SetPos, bool(INPC& npc, Vector3 position))
{
	npc.setPosition(position, true);
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

SCRIPT_API(NPC_Move, bool(INPC& npc, Vector3 targetPos, int moveType, float moveSpeed))
{
	return npc.move(targetPos, NPCMoveType(moveType), moveSpeed);
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

SCRIPT_API(NPC_SetSkin, bool(INPC& npc, int model))
{
	npc.setSkin(model);
	return true;
}

SCRIPT_API(NPC_IsStreamedIn, bool(INPC& npc, IPlayer& player))
{
	return npc.isStreamedInForPlayer(player);
}

SCRIPT_API(NPC_IsAnyStreamedIn, bool(INPC& npc))
{
	auto streamedIn = npc.streamedForPlayers();
	return streamedIn.size() > 0;
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

SCRIPT_API(NPC_ApplyAnimation, bool(INPC& npc, const std::string& animlib, const std::string& animname, float delta, bool loop, bool lockX, bool lockY, bool freeze, uint32_t time, int sync))
{
	const AnimationData animationData(delta, loop, lockX, lockY, freeze, time, animlib, animname);
	npc.getPlayer()->applyAnimation(animationData, PlayerAnimationSyncType_SyncOthers);
	return true;
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

SCRIPT_API(NPC_SetKeys, bool(INPC& npc, uint16_t upAndDown, uint16_t leftAndDown, uint16_t keys))
{
	npc.setKeys(upAndDown, leftAndDown, keys);
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
