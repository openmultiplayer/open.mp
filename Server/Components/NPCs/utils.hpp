#pragma once
#include <sdk.hpp>
#include "npcs_impl.hpp"
#include <Server/Components/Vehicles/vehicle_models.hpp>

#define MAX_HIT_RADIUS 0.4f
#define MAX_HIT_RADIUS_VEHICLE 1.0f
#define MAX_DISTANCE_TO_ENTER_VEHICLE 30.0f
#define MIN_VEHICLE_GO_TO_DISTANCE 1.0f

enum EntityCheckType
{
	EntityCheckType_None,
	EntityCheckType_Player,
	EntityCheckType_NPC,
	EntityCheckType_Actor,
	EntityCheckType_Vehicle,
	EntityCheckType_Object,
	EntityCheckType_ProjectOrig,
	EntityCheckType_ProjectTarg,
	EntityCheckType_Map,
	EntityCheckType_All
};

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

inline int getWeaponActualClipSize(uint8_t weapon, int currentAmmo, int weaponSkillLevel, bool isInfiniteAmmoEnabled)
{
	auto data = WeaponInfo::get(weapon);
	if (data.type != PlayerWeaponType_None)
	{
		int size = data.clipSize;
		if (isWeaponDoubleHanded(weapon, weaponSkillLevel))
		{
			size *= 2;
		}

		if (currentAmmo < size && !isInfiniteAmmoEnabled)
		{
			size = currentAmmo;
		}

		return size;
	}
	return 0;
}

inline int getWeaponActualReloadTime(uint8_t weapon, int skillLevel)
{
	auto data = WeaponInfo::get(weapon);
	if (data.type != PlayerWeaponType_None)
	{
		int time = data.reloadTime;
		if (isWeaponDoubleHanded(weapon, skillLevel))
		{
			time += 700;
		}

		return time;
	}
	return 0;
}

inline int getWeaponActualShootTime(uint8_t weapon)
{
	auto data = WeaponInfo::get(weapon);
	if (data.type != PlayerWeaponType_None)
	{
		return data.shootTime;
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

inline IPlayer* getClosestPlayerInBetween(IPlayerPool* players, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, int playerId, int targetId, std::pair<Vector3, Vector3>& results)
{
	IPlayer* closestPlayer = nullptr;

	// Loop through all the players
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

		// Is the player on the ray
		if (getDistanceFromRayToPoint(hitOrigin, hitTarget, pos) > MAX_HIT_RADIUS)
		{
			continue;
		}

		// Is the player in the damage range
		float playerDistance = glm::distance(hitOrigin, pos);
		if (playerDistance > range)
		{
			continue;
		}

		// Is the player closer than another player
		if (!closestPlayer || playerDistance < distance)
		{
			distance = playerDistance;
			closestPlayer = player;

			results.first = getNearestPointToRay(hitOrigin, hitTarget, pos);
			results.second = hitTarget - pos;
		}
	}
	return closestPlayer;
}

inline INPC* getClosestNpcInBetween(NPCComponent* npcs, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, int playerId, int targetId, std::pair<Vector3, Vector3>& results)
{
	INPC* closestNpc = nullptr;

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

		// Is the NPC on the ray
		if (getDistanceFromRayToPoint(hitOrigin, hitTarget, pos) > MAX_HIT_RADIUS)
		{
			continue;
		}

		// Is the NPC in the damage range
		float npcDistance = glm::distance(hitOrigin, pos);
		if (npcDistance > range)
		{
			continue;
		}

		// Is the NPC closer than another NPC
		if (!closestNpc || npcDistance < distance)
		{
			distance = npcDistance;
			closestNpc = npc;

			results.first = getNearestPointToRay(hitOrigin, hitTarget, pos);
			results.second = hitTarget - pos;
		}
	}

	return closestNpc;
}

inline IActor* getClosestActorInBetween(IActorsComponent* actors, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, std::pair<Vector3, Vector3>& results)
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

			results.first = getNearestPointToRay(hitOrigin, hitTarget, pos);
			results.second = hitTarget;
		}
	}
	return closestActor;
}

inline IVehicle* getClosestVehicleInBetween(IVehiclesComponent* vehicles, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, std::pair<Vector3, Vector3>& results)
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

			results.first = getNearestPointToRay(hitOrigin, hitTarget, pos);
			results.second = hitTarget - pos;
		}
	}
	return closestVehicle;
}

inline IObject* getClosestObjectInBetween(IObjectsComponent* objects, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, std::pair<Vector3, Vector3>& results)
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

			results.first = getNearestPointToRay(hitOrigin, hitTarget, pos);
			results.second = hitTarget - pos;
		}
	}
	return closestObject;
}

inline IPlayerObject* getClosestPlayerObjectInBetween(IPlayerPool* players, const Vector3& hitOrigin, const Vector3& hitTarget, float range, float& distance, int ownerId, std::pair<Vector3, Vector3>& results)
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

				results.first = getNearestPointToRay(hitOrigin, hitTarget, pos);
				results.second = hitTarget - pos;
			}
		}
	}
	return closestPlayerObject;
}

inline void* getClosestEntityInBetween(NPCComponent* npcs, const Vector3& hitOrigin, const Vector3& hitTarget, float range, uint8_t betweenCheckFlags, int playerId, int targetId, uint8_t& entityType, int& playerObjectOwnerId, Vector3& hitMap, std::pair<Vector3, Vector3>& results)
{
	void* closestEntity = nullptr;
	float closestEntityDistance = 0.0f;
	entityType = EntityCheckType_None;

	IPlayer* closestPlayer = nullptr;
	if (betweenCheckFlags & EntityCheckType_Player)
	{
		float closestPlayerDistance = 0.0f;
		closestPlayer = getClosestPlayerInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerDistance, playerId, targetId, results);
		if (closestPlayer != nullptr && (closestEntity == nullptr || closestPlayerDistance < closestEntityDistance))
		{
			entityType = EntityCheckType_Player;
			closestEntityDistance = closestPlayerDistance;
			closestEntity = static_cast<void*>(closestPlayer);
		}
	}

	INPC* closestNPC = nullptr;
	if (betweenCheckFlags & EntityCheckType_NPC)
	{
		float closestNPCDistance = 0.0f;
		closestNPC = getClosestNpcInBetween(npcs, hitOrigin, hitTarget, range, closestNPCDistance, playerId, targetId, results);
		if (closestNPC != nullptr && (closestEntity == nullptr || closestNPCDistance < closestEntityDistance))
		{
			entityType = EntityCheckType_NPC;
			closestEntityDistance = closestNPCDistance;
			closestEntity = static_cast<void*>(closestNPC);
		}
	}

	if (betweenCheckFlags & EntityCheckType_Actor)
	{
		float closestActorDistance = 0.0f;
		IActor* closestActor = getClosestActorInBetween(npcs->getActorsPool(), hitOrigin, hitTarget, range, closestActorDistance, results);
		if (closestActor != nullptr && (closestEntity == nullptr || closestActorDistance < closestEntityDistance))
		{
			entityType = EntityCheckType_Actor;
			closestEntityDistance = closestActorDistance;
			closestEntity = static_cast<void*>(closestActor);
		}
	}

	if (betweenCheckFlags & EntityCheckType_Vehicle)
	{
		float closestVehicleDistance = 0.0f;
		IVehicle* closestVehicle = getClosestVehicleInBetween(npcs->getVehiclesPool(), hitOrigin, hitTarget, range, closestVehicleDistance, results);
		if (closestVehicle != nullptr && (closestEntity == nullptr || closestVehicleDistance < closestEntityDistance))
		{
			entityType = EntityCheckType_Vehicle;
			closestEntityDistance = closestVehicleDistance;
			closestEntity = static_cast<void*>(closestVehicle);
		}
	}

	if (betweenCheckFlags & EntityCheckType_Object)
	{
		float closestObjectDistance = 0.0f;
		IObject* closestObject = getClosestObjectInBetween(npcs->getObjectsPool(), hitOrigin, hitTarget, range, closestObjectDistance, results);
		if (closestObject != nullptr && (closestEntity == nullptr || closestObjectDistance < closestEntityDistance))
		{
			entityType = EntityCheckType_Object;
			closestEntityDistance = closestObjectDistance;
			closestEntity = static_cast<void*>(closestObject);
		}
	}

	if (betweenCheckFlags & EntityCheckType_ProjectOrig)
	{
		float closestPlayerObjectDistance = 0.0f;
		IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, playerId, results);
		if (closestPlayerObject != nullptr && (closestEntity == nullptr || closestPlayerObjectDistance < closestEntityDistance))
		{
			entityType = EntityCheckType_ProjectOrig;
			playerObjectOwnerId = playerId;
			closestEntityDistance = closestPlayerObjectDistance;
			closestEntity = static_cast<void*>(closestPlayerObject);
		}
	}

	if (betweenCheckFlags & EntityCheckType_ProjectTarg)
	{
		// One flag for 3 checks
		// Check if a player object of the target is in between the origin and the target
		if (targetId != INVALID_PLAYER_ID)
		{
			float closestPlayerObjectDistance = 0.0;
			IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, targetId, results);
			if (closestPlayerObject != nullptr && (closestEntity == nullptr || closestPlayerObjectDistance < closestEntityDistance))
			{
				entityType = EntityCheckType_ProjectTarg;
				playerObjectOwnerId = targetId;
				closestEntityDistance = closestPlayerObjectDistance;
				closestEntity = static_cast<void*>(closestPlayerObject);
			}
		}

		// Check if a player object of the closestPlayer is in between the origin and the target when the closestPlayer is currently the closest entity
		if (closestPlayer != nullptr && closestEntity == closestPlayer)
		{
			float closestPlayerObjectDistance = 0.0;
			IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, closestPlayer->getID(), results);
			if (closestPlayerObject != nullptr && (closestEntity == nullptr || closestPlayerObjectDistance < closestEntityDistance))
			{
				entityType = EntityCheckType_ProjectTarg;
				playerObjectOwnerId = closestPlayer->getID();
				closestEntityDistance = closestPlayerObjectDistance;
				closestEntity = static_cast<void*>(closestPlayerObject);
			}
		}

		// Check if a player object of the closestNPC is in between the origin and the target when the closestNPC is currently the closest entity
		if (closestNPC != nullptr && closestEntity == closestNPC)
		{
			float closestPlayerObjectDistance = 0.0;
			IPlayerObject* closestPlayerObject = getClosestPlayerObjectInBetween(&npcs->getCore()->getPlayers(), hitOrigin, hitTarget, range, closestPlayerObjectDistance, closestNPC->getID(), results);
			if (closestPlayerObject != nullptr && (closestEntity == nullptr || closestPlayerObjectDistance < closestEntityDistance))
			{
				entityType = EntityCheckType_ProjectTarg;
				playerObjectOwnerId = closestNPC->getID();
				// closestEntityDistance = closestPlayerObjectDistance;
				closestEntity = static_cast<void*>(closestPlayerObject);
			}
		}
	}

	return closestEntity;
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
