/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once
#include <sdk.hpp>
#include "npcs_impl.hpp"
#include <Server/Components/Vehicles/vehicle_models.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

#define MAX_HIT_RADIUS 0.4f
#define MAX_HIT_RADIUS_VEHICLE 1.0f
#define MAX_DISTANCE_TO_ENTER_VEHICLE 30.0f
#define MIN_VEHICLE_GO_TO_DISTANCE 1.0f

static const float WeaponDamages[MAX_WEAPON_ID] = {
	5.0f, // fists (0)
	5.0f, // WEAPON_BRASSKNUCKLE (1)
	5.0f, // WEAPON_GOLFCLUB (2)
	5.0f, // WEAPON_NITESTICK (3)
	5.0f, // WEAPON_KNIFE (4)
	5.0f, // WEAPON_BAT (5)
	5.0f, // WEAPON_SHOVEL (6)
	5.0f, // WEAPON_POOLSTICK (7)
	5.0f, // WEAPON_KATANA (8)
	5.0f, // WEAPON_CHAINSAW (9)
	5.0f, // WEAPON_DILDO (10)
	5.0f, // WEAPON_DILDO2 (11)
	5.0f, // WEAPON_VIBRATOR (12)
	5.0f, // WEAPON_VIBRATOR2 (13)
	5.0f, // WEAPON_FLOWER (14)
	5.0f, // WEAPON_CANE (15)
	5.0f, // WEAPON_GRENADE (16)
	5.0f, // WEAPON_TEARGAS (17)
	5.0f, // WEAPON_MOLTOV (18)
	0.0f, // nothing (19)
	0.0f, // nothing (20)
	0.0f, // nothing (21)
	8.25f, // WEAPON_COLT45 (22)
	13.2f, // WEAPON_SILENCED (23)
	46.2f, // WEAPON_DEAGLE (24)
	30.0f, // WEAPON_SHOTGUN (25)
	30.0f, // WEAPON_SAWEDOFF (26)
	30.0f, // WEAPON_SHOTGSPA (27)
	6.6f, // WEAPON_UZI (28)
	8.25f, // WEAPON_MP5 (29)
	9.9f, // WEAPON_AK47 (30)
	9.9f, // WEAPON_M4 (31)
	6.6f, // WEAPON_TEC9 (32)
	24.8f, // WEAPON_RIFLE (33)
	41.3f, // WEAPON_SNIPER (34)
	5.0f, // WEAPON_ROCKETLAUNCHER (35)
	5.0f, // WEAPON_HEATSEEKER (36)
	5.0f, // WEAPON_FLAMETHROWER (37)
	46.2f, // WEAPON_MINIGUN (38)
	5.0f, // WEAPON_SATCHEL (39)
	5.0f, // WEAPON_BOMB (40)
	5.0f, // WEAPON_SPRAYCAN (41)
	5.0f, // WEAPON_FIREEXTINGUISHER (42)
	0.0f, // WEAPON_CAMERA (43)
	0.0f, // WEAPON_NIGHTVISION (44)
	0.0f, // WEAPON_INFRARED (45)
	0.0f, // WEAPON_PARACHUTE (46)
};

inline WeaponInfo* getCustomWeaponInfo(StaticArray<WeaponInfo, MAX_WEAPON_ID>& list, uint8_t weapon)
{
	if (weapon >= list.size())
	{
		return nullptr;
	}
	return &list[weapon];
}

inline bool canWeaponBeDoubleHanded(uint8_t weapon)
{
	switch (weapon)
	{
	case PlayerWeapon_Colt45:
	case PlayerWeapon_Sawedoff:
	case PlayerWeapon_UZI:
	case PlayerWeapon_TEC9:
		return true;
	}

	return false;
}

inline bool isWeaponDoubleHanded(uint8_t weapon, int skillLevel)
{
	return canWeaponBeDoubleHanded(weapon) && skillLevel > 999;
}

inline PlayerWeaponSkill getWeaponSkillID(uint8_t weapon)
{
	static PlayerWeaponSkill skills[] = {
		PlayerWeaponSkill_Pistol, // 22
		PlayerWeaponSkill_SilencedPistol, // 23
		PlayerWeaponSkill_DesertEagle, // 24
		PlayerWeaponSkill_Shotgun, // 25
		PlayerWeaponSkill_SawnOff, // 26
		PlayerWeaponSkill_SPAS12, // 27
		PlayerWeaponSkill_Uzi, // 28
		PlayerWeaponSkill_MP5, // 29
		PlayerWeaponSkill_AK47, // 30
		PlayerWeaponSkill_M4, // 31
		PlayerWeaponSkill_Uzi, // 32
		PlayerWeaponSkill_Sniper, // 33
		PlayerWeaponSkill_Sniper // 34
	};

	if (!WeaponSlotData(weapon).shootable())
	{
		return PlayerWeaponSkill_Invalid;
	}

	return skills[weapon - 22];
}

inline int getWeaponActualShootTime(StaticArray<WeaponInfo, MAX_WEAPON_ID>& list, uint8_t weapon)
{
	auto data = getCustomWeaponInfo(list, weapon);
	if (data)
	{
		return data->shootTime;
	}
	return 0;
}

inline Vector3 getNearestPointToRay(const Vector3& startPosition, const Vector3& endPosition, const Vector3& point)
{
	Vector3 vecDirection = (endPosition - startPosition) / glm::distance(startPosition, endPosition);
	return vecDirection * glm::distance(startPosition, point) + startPosition;
}

inline float getDistanceFromRayToPoint(const Vector3& startPosition, const Vector3& endPosition, const Vector3& point)
{
	return glm::distance(getNearestPointToRay(startPosition, endPosition, point), point);
}

inline IPlayer* getClosestPlayerInBetween(IPlayerPool* players, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, int playerId, int targetId)
{
	IPlayer* closestPlayer = nullptr;
	float rangeSq = range * range; // Pre-calculate square to avoid sqrt in distance comparison
	float closestDistanceSq = rangeSq;

	// Pre-calculate ray direction and length
	const Vector3 rayDir = hitTarget - hitOrigin;
	const float rayLengthSq = glm::dot(rayDir, rayDir);
	const float rayLength = std::sqrt(rayLengthSq);
	const Vector3 rayNormalized = rayLength > 0.0f ? rayDir / rayLength : Vector3(0.0f);

	// Loop through all the players with early exits
	for (auto player : players->entries())
	{
		// Validate the player
		if (!player)
		{
			continue;
		}

		auto currPlayerId = player->getID();
		if (playerId == currPlayerId || targetId == currPlayerId || player->isBot())
		{
			continue;
		}

		auto pos = player->getPosition();

		// Quick distance check first (cheaper than ray calculation)
		const Vector3 toPlayer = pos - hitOrigin;
		float playerDistanceSq = glm::dot(toPlayer, toPlayer);
		if (playerDistanceSq > rangeSq)
		{
			continue;
		}

		// Optimized ray-to-point distance check
		float t = glm::dot(toPlayer, rayNormalized);
		t = glm::clamp(t, 0.0f, rayLength);
		Vector3 projection = hitOrigin + rayNormalized * t;
		float rayDistanceSq = glm::dot(pos - projection, pos - projection);
		if (rayDistanceSq > MAX_HIT_RADIUS * MAX_HIT_RADIUS)
		{
			continue;
		}

		// Is the player closer than another player
		if (!closestPlayer || playerDistanceSq < closestDistanceSq)
		{
			closestDistanceSq = playerDistanceSq;
			closestPlayer = player;
		}
	}

	if (closestPlayer)
	{
		distance = std::sqrt(closestDistanceSq);
	}
	return closestPlayer;
}

inline INPC* getClosestNpcInBetween(NPCComponent* npcs, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, int playerId, int targetId)
{
	INPC* closestNpc = nullptr;
	float rangeSq = range * range;
	float closestDistanceSq = rangeSq;

	// Pre-calculate ray direction and length
	const Vector3 rayDir = hitTarget - hitOrigin;
	const float rayLengthSq = glm::dot(rayDir, rayDir);
	const float rayLength = std::sqrt(rayLengthSq);
	const Vector3 rayNormalized = rayLength > 0.0f ? rayDir / rayLength : Vector3(0.0f);

	// Loop through all the NPCs
	for (auto npc : npcs->entries())
	{
		if (!npc)
		{
			continue;
		}

		auto npcId = npc->getID();
		if (playerId == npcId || targetId == npcId)
		{
			continue;
		}

		auto pos = npc->getPosition();

		// Quick distance check first
		const Vector3 toNpc = pos - hitOrigin;
		float npcDistanceSq = glm::dot(toNpc, toNpc);
		if (npcDistanceSq > rangeSq)
		{
			continue;
		}

		// Optimized ray-to-point distance check
		float t = glm::dot(toNpc, rayNormalized);
		t = glm::clamp(t, 0.0f, rayLength);
		Vector3 projection = hitOrigin + rayNormalized * t;
		float rayDistanceSq = glm::dot(pos - projection, pos - projection);
		if (rayDistanceSq > MAX_HIT_RADIUS * MAX_HIT_RADIUS)
		{
			continue;
		}

		// Is the NPC closer than another NPC
		if (!closestNpc || npcDistanceSq < closestDistanceSq)
		{
			closestDistanceSq = npcDistanceSq;
			closestNpc = npc;
		}
	}

	if (closestNpc)
	{
		distance = std::sqrt(closestDistanceSq);
	}
	return closestNpc;
}

inline IActor* getClosestActorInBetween(IActorsComponent* actors, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance)
{
	IActor* closestActor = nullptr;

	// Loop through all the actors
	for (auto actor : *actors)
	{
		if (!actor)
		{
			continue;
		}

		// Is the actor on the ray
		if (getDistanceFromRayToPoint(hitOrigin, hitTarget, actor->getPosition()) > MAX_HIT_RADIUS)
		{
			continue;
		}

		auto pos = actor->getPosition();

		// Is the actor in the damage range
		float actorDistance = glm::distance(hitOrigin, pos);
		if (actorDistance > range)
		{
			continue;
		}

		// Is the actor closer than another actor
		if (!closestActor || actorDistance < distance)
		{
			distance = actorDistance;
			closestActor = actor;
		}
	}
	return closestActor;
}

inline IVehicle* getClosestVehicleInBetween(IVehiclesComponent* vehicles, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance)
{
	IVehicle* closestVehicle = nullptr;

	// Loop through all the vehicles
	for (auto vehicle : *vehicles)
	{
		if (!vehicle)
		{
			continue;
		}

		auto pos = vehicle->getPosition();

		// Is the vehicle on the ray
		if (getDistanceFromRayToPoint(hitOrigin, hitTarget, pos) > MAX_HIT_RADIUS_VEHICLE)
		{
			// Don't use MAX_HIT_RADIUS
			continue;
		}

		// Is the vehicle in the damage range
		float vehicleDistance = glm::distance(hitOrigin, pos);
		if (vehicleDistance > range)
		{
			continue;
		}

		// Is the vehicle closer than another vehicle
		if (!closestVehicle || vehicleDistance < distance)
		{
			distance = vehicleDistance;
			closestVehicle = vehicle;
		}
	}
	return closestVehicle;
}

inline IObject* getClosestObjectInBetween(IObjectsComponent* objects, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance)
{
	IObject* closestObject = nullptr;

	// Loop through all the objects
	for (auto object : *objects)
	{
		if (!object)
		{
			continue;
		}

		auto pos = object->getPosition();

		// Is the object on the ray
		if (getDistanceFromRayToPoint(hitOrigin, hitTarget, pos) > MAX_HIT_RADIUS)
		{
			continue;
		}

		// Is the object in the damage range
		float objectDistance = glm::distance(hitOrigin, pos);
		if (objectDistance > range)
		{
			continue;
		}

		// Is the object closer than another object
		if (!closestObject || objectDistance < distance)
		{
			distance = objectDistance;
			closestObject = object;
		}
	}
	return closestObject;
}

inline IPlayerObject* getClosestPlayerObjectInBetween(IPlayerPool* players, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, int ownerId)
{
	IPlayerObject* closestPlayerObject = nullptr;

	// Validate the owner
	auto player = players->get(ownerId);
	if (!player)
	{
		return closestPlayerObject;
	}

	// Loop through all the player objects of the owner
	auto playerObjects = queryExtension<IPlayerObjectData>(player);
	if (playerObjects)
	{
		for (auto object : *playerObjects)
		{
			if (!object)
			{
				continue;
			}

			auto pos = object->getPosition();

			// Is the player object on the ray
			if (getDistanceFromRayToPoint(hitOrigin, hitTarget, pos) > MAX_HIT_RADIUS)
			{
				continue;
			}

			// Is the player object in the damage range
			float playerObjectDistance = glm::distance(hitOrigin, pos);
			if (playerObjectDistance > range)
			{
				continue;
			}

			// Is the player object closer than another player object
			if (!closestPlayerObject || playerObjectDistance < distance)
			{
				distance = playerObjectDistance;
				closestPlayerObject = object;
			}
		}
	}
	return closestPlayerObject;
}

inline int getClosestEntityInBetween(NPCComponent* npcs, const Vector3& hitOrigin, const Vector3& hitTarget, float range, EntityCheckType betweenCheckFlags, int playerId, int targetId, EntityCheckType& entityType, int& playerObjectOwnerId, Vector3& hitMap)
{
	int closestEntityId = INVALID_PLAYER_ID;
	float closestEntityDistance = 0.0f;
	entityType = EntityCheckType::None;

	IPlayer* closestPlayer = nullptr;
	if (int(betweenCheckFlags) & int(EntityCheckType::Player))
	{
		float closestPlayerDistance = 0.0f;
		closestPlayer = getClosestPlayerInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerDistance, playerId, targetId);
		if (closestPlayer != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestPlayerDistance < closestEntityDistance))
		{
			entityType = EntityCheckType::Player;
			closestEntityDistance = closestPlayerDistance;
			closestEntityId = closestPlayer->getID();
		}
	}

	INPC* closestNPC = nullptr;
	if (int(betweenCheckFlags) & int(EntityCheckType::NPC))
	{
		float closestNPCDistance = 0.0f;
		closestNPC = getClosestNpcInBetween(npcs, hitOrigin, hitTarget, range, closestNPCDistance, playerId, targetId);
		if (closestNPC != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestNPCDistance < closestEntityDistance))
		{
			entityType = EntityCheckType::NPC;
			closestEntityDistance = closestNPCDistance;
			closestEntityId = closestNPC->getID();
		}
	}

	if (int(betweenCheckFlags) & int(EntityCheckType::Actor))
	{
		float closestActorDistance = 0.0f;
		if (npcs->getActorsPool())
		{
			IActor* closestActor = getClosestActorInBetween(npcs->getActorsPool(), hitOrigin, hitTarget, range, closestActorDistance);
			if (closestActor != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestActorDistance < closestEntityDistance))
			{
				entityType = EntityCheckType::Actor;
				closestEntityDistance = closestActorDistance;
				closestEntityId = closestActor->getID();
			}
		}
	}

	if (int(betweenCheckFlags) & int(EntityCheckType::Vehicle))
	{
		float closestVehicleDistance = 0.0f;
		if (npcs->getVehiclesPool())
		{
			IVehicle* closestVehicle = getClosestVehicleInBetween(npcs->getVehiclesPool(), hitOrigin, hitTarget, range, closestVehicleDistance);
			if (closestVehicle != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestVehicleDistance < closestEntityDistance))
			{
				entityType = EntityCheckType::Vehicle;
				closestEntityDistance = closestVehicleDistance;
				closestEntityId = closestVehicle->getID();
			}
		}
	}

	if (int(betweenCheckFlags) & int(EntityCheckType::Object))
	{
		float closestObjectDistance = 0.0f;
		if (npcs->getObjectsPool())
		{
			IObject* closestObject = getClosestObjectInBetween(npcs->getObjectsPool(), hitOrigin, hitTarget, range, closestObjectDistance);
			if (closestObject != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestObjectDistance < closestEntityDistance))
			{
				entityType = EntityCheckType::Object;
				closestEntityDistance = closestObjectDistance;
				closestEntityId = closestObject->getID();
			}
		}
	}

	if (int(betweenCheckFlags) & int(EntityCheckType::ProjectOrig))
	{
		float closestPlayerObjectDistance = 0.0f;
		IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, playerId);
		if (closestPlayerObject != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestPlayerObjectDistance < closestEntityDistance))
		{
			entityType = EntityCheckType::ProjectOrig;
			playerObjectOwnerId = playerId;
			closestEntityDistance = closestPlayerObjectDistance;
			closestEntityId = closestPlayerObject->getID();
		}
	}

	if (int(betweenCheckFlags) & int(EntityCheckType::ProjectTarg))
	{
		// One flag for 3 checks
		// Check if a player object of the target is in between the origin and the target
		if (targetId != INVALID_PLAYER_ID)
		{
			float closestPlayerObjectDistance = 0.0;
			IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, targetId);
			if (closestPlayerObject != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestPlayerObjectDistance < closestEntityDistance))
			{
				entityType = EntityCheckType::ProjectTarg;
				playerObjectOwnerId = targetId;
				closestEntityDistance = closestPlayerObjectDistance;
				closestEntityId = closestPlayerObject->getID();
			}
		}

		// Check if a player object of the closestPlayer is in between the origin and the target when the closestPlayer is currently the closest entity
		if (closestPlayer != nullptr && closestEntityId == closestPlayer->getID())
		{
			float closestPlayerObjectDistance = 0.0;
			IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, closestPlayer->getID());
			if (closestPlayerObject != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestPlayerObjectDistance < closestEntityDistance))
			{
				entityType = EntityCheckType::ProjectTarg;
				playerObjectOwnerId = closestPlayer->getID();
				closestEntityDistance = closestPlayerObjectDistance;
				closestEntityId = closestPlayerObject->getID();
			}
		}

		// Check if a player object of the closestNPC is in between the origin and the target when the closestNPC is currently the closest entity
		if (closestNPC != nullptr && closestEntityId == closestNPC->getID())
		{
			float closestPlayerObjectDistance = 0.0;
			IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, closestNPC->getID());
			if (closestPlayerObject != nullptr && (closestEntityId == INVALID_PLAYER_ID || closestPlayerObjectDistance < closestEntityDistance))
			{
				entityType = EntityCheckType::ProjectTarg;
				playerObjectOwnerId = closestNPC->getID();
				// closestEntityDistance = closestPlayerObjectDistance;
				closestEntityId = closestPlayerObject->getID();
			}
		}
	}

	return closestEntityId;
}

inline float getNearestFloatValue(float value, const DynamicArray<float>& floatArray)
{
	float nearest = floatArray[0];

	for (auto f : floatArray)
	{
		if (std::abs(f - value) < std::abs(nearest - value))
		{
			nearest = f;
		}
	}

	return nearest;
}

inline bool isEqualFloat(float a, float b)
{
	return glm::epsilonEqual(a, b, glm::epsilon<float>());
}

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

inline Vector3 getVehicleSeatPos(IVehicle& vehicle, int seatId)
{
	// Get the seat position
	Vector3 seatPosFromModelInfo;

	if (seatId == 0 || seatId == 1)
	{
		Impl::getVehicleModelInfo(vehicle.getModel(), VehicleModelInfo_FrontSeat, seatPosFromModelInfo);
	}
	else
	{
		Impl::getVehicleModelInfo(vehicle.getModel(), VehicleModelInfo_RearSeat, seatPosFromModelInfo);
	}

	// Adjust the seat vector
	Vector3 seatPosNoAngle(seatPosFromModelInfo.x + 1.3f, seatPosFromModelInfo.y - 0.6f, seatPosFromModelInfo.z);

	if (seatId == 0 || seatId == 2)
	{
		seatPosNoAngle.x = -seatPosNoAngle.x;
	}

	// Get vehicle angle
	float angle = vehicle.getZAngle();
	float _angle = angle * 0.01570796326794897f;

	Vector3 seatPos(seatPosNoAngle.x * cos(_angle) - seatPosNoAngle.y * sin(_angle),
		seatPosNoAngle.x * sin(_angle) + seatPosNoAngle.y * cos(_angle),
		seatPosNoAngle.z);

	return seatPos + vehicle.getPosition();
}
