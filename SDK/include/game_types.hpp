/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "anim.hpp"
#include "types.hpp"
#include "values.hpp"
#include <chrono>
#include <string>
#include <type_traits>
#include <utility>

enum PlayerFightingStyle
{
	PlayerFightingStyle_Normal = 4,
	PlayerFightingStyle_Boxing = 5,
	PlayerFightingStyle_KungFu = 6,
	PlayerFightingStyle_KneeHead = 7,
	PlayerFightingStyle_GrabKick = 15,
	PlayerFightingStyle_Elbow = 16
};

enum PlayerState
{
	PlayerState_None = 0,
	PlayerState_OnFoot = 1,
	PlayerState_Driver = 2,
	PlayerState_Passenger = 3,
	PlayerState_ExitVehicle = 4,
	PlayerState_EnterVehicleDriver = 5,
	PlayerState_EnterVehiclePassenger = 6,
	PlayerState_Wasted = 7,
	PlayerState_Spawned = 8,
	PlayerState_Spectating = 9
};

enum PlayerWeaponSkill
{
	PlayerWeaponSkill_Pistol,
	PlayerWeaponSkill_SilencedPistol,
	PlayerWeaponSkill_DesertEagle,
	PlayerWeaponSkill_Shotgun,
	PlayerWeaponSkill_SawnOff,
	PlayerWeaponSkill_SPAS12,
	PlayerWeaponSkill_Uzi,
	PlayerWeaponSkill_MP5,
	PlayerWeaponSkill_AK47,
	PlayerWeaponSkill_M4,
	PlayerWeaponSkill_Sniper
};

enum PlayerSpecialAction
{
	SpecialAction_None,
	SpecialAction_Duck,
	SpecialAction_Jetpack,
	SpecialAction_EnterVehicle,
	SpecialAction_ExitVehicle,
	SpecialAction_Dance1,
	SpecialAction_Dance2,
	SpecialAction_Dance3,
	SpecialAction_Dance4,
	SpecialAction_HandsUp = 10,
	SpecialAction_Cellphone,
	SpecialAction_Sitting,
	SpecialAction_StopCellphone,
	SpecialAction_Beer = 20,
	Specialaction_Smoke,
	SpecialAction_Wine,
	SpecialAction_Sprunk,
	SpecialAction_Cuffed,
	SpecialAction_Carry,
	SpecialAction_Pissing = 68
};

enum PlayerAnimationSyncType
{
	PlayerAnimationSyncType_NoSync,
	PlayerAnimationSyncType_Sync,
	PlayerAnimationSyncType_SyncOthers
};

enum PlayerBulletHitType : uint8_t
{
	PlayerBulletHitType_None,
	PlayerBulletHitType_Player = 1,
	PlayerBulletHitType_Vehicle = 2,
	PlayerBulletHitType_Object = 3,
	PlayerBulletHitType_PlayerObject = 4,
};

enum BodyPart
{
	BodyPart_Torso = 3,
	BodyPart_Groin,
	BodyPart_LeftArm,
	BodyPart_RightArm,
	BodyPart_LeftLeg,
	BodyPart_RightLeg,
	BodyPart_Head
};

enum MapIconStyle
{
	MapIconStyle_Local,
	MapIconStyle_Global,
	MapIconStyle_LocalCheckpoint,
	MapIconStyle_GlobalCheckpoint
};

enum PlayerClickSource
{
	PlayerClickSource_Scoreboard
};

enum PlayerSpectateMode
{
	PlayerSpectateMode_Normal = 1,
	PlayerSpectateMode_Fixed,
	PlayerSpectateMode_Side
};

enum PlayerCameraCutType
{
	PlayerCameraCutType_Cut,
	PlayerCameraCutType_Move
};

enum PlayerMarkerMode
{
	PlayerMarkerMode_Off,
	PlayerMarkerMode_Global,
	PlayerMarkerMode_Streamed
};

enum LagCompMode
{
	LagCompMode_Disabled = 0,
	LagCompMode_PositionOnly = 2,
	LagCompMode_Enabled = 1
};

enum PlayerWeapon
{
	PlayerWeapon_Fist,
	PlayerWeapon_BrassKnuckle,
	PlayerWeapon_GolfClub,
	PlayerWeapon_NiteStick,
	PlayerWeapon_Knife,
	PlayerWeapon_Bat,
	PlayerWeapon_Shovel,
	PlayerWeapon_PoolStick,
	PlayerWeapon_Katana,
	PlayerWeapon_Chainsaw,
	PlayerWeapon_Dildo,
	PlayerWeapon_Dildo2,
	PlayerWeapon_Vibrator,
	PlayerWeapon_Vibrator2,
	PlayerWeapon_Flower,
	PlayerWeapon_Cane,
	PlayerWeapon_Grenade,
	PlayerWeapon_Teargas,
	PlayerWeapon_Moltov,
	PlayerWeapon_Colt45 = 22,
	PlayerWeapon_Silenced,
	PlayerWeapon_Deagle,
	PlayerWeapon_Shotgun,
	PlayerWeapon_Sawedoff,
	PlayerWeapon_Shotgspa,
	PlayerWeapon_UZI,
	PlayerWeapon_MP5,
	PlayerWeapon_AK47,
	PlayerWeapon_M4,
	PlayerWeapon_TEC9,
	PlayerWeapon_Rifle,
	PlayerWeapon_Sniper,
	PlayerWeapon_RocketLauncher,
	PlayerWeapon_HeatSeeker,
	PlayerWeapon_FlameThrower,
	PlayerWeapon_Minigun,
	PlayerWeapon_Satchel,
	PlayerWeapon_Bomb,
	PlayerWeapon_SprayCan,
	PlayerWeapon_FireExtinguisher,
	PlayerWeapon_Camera,
	PlayerWeapon_Night_Vis_Goggles,
	PlayerWeapon_Thermal_Goggles,
	PlayerWeapon_Parachute,
	PlayerWeapon_Vehicle = 49,
	PlayerWeapon_Heliblades,
	PlayerWeapon_Explosion,
	PlayerWeapon_Drown = 53,
	PlayerWeapon_Collision,
	PlayerWeapon_End
};

static const StringView PlayerWeaponNames[] = {
	"Fist",
	"Brass Knuckles",
	"Golf Club",
	"Nite Stick",
	"Knife",
	"Baseball Bat",
	"Shovel",
	"Pool Cue",
	"Katana",
	"Chainsaw",
	"Dildo",
	"Dildo",
	"Vibrator",
	"Vibrator",
	"Flowers",
	"Cane",
	"Grenade",
	"Teargas",
	"Molotov Cocktail", // 18
	"Invalid",
	"Invalid",
	"Invalid",
	"Colt 45", // 22
	"Silenced Pistol",
	"Desert Eagle",
	"Shotgun",
	"Sawn-off Shotgun",
	"Combat Shotgun",
	"UZI",
	"MP5",
	"AK47",
	"M4",
	"TEC9",
	"Rifle",
	"Sniper Rifle",
	"Rocket Launcher",
	"Heat Seaker",
	"Flamethrower",
	"Minigun",
	"Satchel Explosives",
	"Bomb",
	"Spray Can",
	"Fire Extinguisher",
	"Camera",
	"Night Vision Goggles",
	"Thermal Goggles",
	"Parachute", // 46
	"Invalid",
	"Invalid",
	"Vehicle", // 49
	"Helicopter Blades", // 50
	"Explosion", // 51
	"Invalid",
	"Drowned", // 53
	"Splat"
};

static const StringView BodyPartString[] = {
	"invalid",
	"invalid",
	"invalid",
	"torso",
	"groin",
	"left arm",
	"right arm",
	"left leg",
	"right leg",
	"head"
};

struct PlayerKeyData
{
	// todo fill with union
	uint32_t keys;
	int16_t upDown;
	int16_t leftRight;
};

struct PlayerAnimationData
{
	uint16_t ID;
	uint16_t flags;

	inline Pair<StringView, StringView> name() const
	{
		return splitAnimationNames(ID);
	}
};

struct PlayerSurfingData
{
	enum class Type
	{
		None,
		Vehicle,
		Object,
		PlayerObject
	} type;
	int ID;
	Vector3 offset;
};

/// Holds weapon slot data
struct WeaponSlotData
{
	uint8_t id;
	uint32_t ammo;

	WeaponSlotData()
		: id(0)
		, ammo(0)
	{
	}

	WeaponSlotData(uint8_t id)
		: id(id)
		, ammo(0)
	{
	}

	WeaponSlotData(uint8_t id, uint32_t ammo)
		: id(id)
		, ammo(ammo)
	{
	}

	uint8_t slot()
	{
		static const uint8_t slots[] = {
			0, // 0 - Fist
			0, // 1 - Brass Knuckles
			1, // 2 - Golf Club
			1, // 3 - Nite Stick
			1, // 4 - Knife
			1, // 5 - Baseball Bat
			1, // 6 - Shovel
			1, // 7 - Pool Cue
			1, // 8 - Katana
			1, // 9 - Chainsaw
			10, // 10 - Dildo
			10, // 11 - Dildo
			10, // 12 - Vibrator
			10, // 13 - Vibrator
			10, // 14 - Flowers
			10, // 15 - Cane
			8, // 16 - Grenade
			8, // 17 - Teargas
			8, // 18 - Molotov Cocktail
			INVALID_WEAPON_SLOT, //
			INVALID_WEAPON_SLOT, //
			INVALID_WEAPON_SLOT, //
			2, // 22 - Colt 45
			2, // 23 - Silenced Pistol
			2, // 24 - Desert Eagle
			3, // 25 - Shotgun
			3, // 26 - Sawn-off Shotgun
			3, // 27 - Combat Shotgun
			4, // 28 - UZI
			4, // 29 - MP5
			5, // 30 - AK47
			5, // 31 - M4
			4, // 32 - TEC9
			6, // 33 - Rifle
			6, // 34 - Sniper Rifle
			7, // 35 - Rocket Launcher
			7, // 36 - Heat Seaker
			7, // 37 - Flamethrower
			7, // 38 - Minigun
			8, // 39 - Satchel Explosives
			12, // 40 - Bomb
			9, // 41 - Spray Can
			9, // 42 - Fire Extinguisher
			9, // 43 - Camera
			11, // 44 - Night Vision Goggles
			11, // 45 - Thermal Goggles
			11 // 46 - Parachute
		};
		if (id >= GLM_COUNTOF(slots))
		{
			return INVALID_WEAPON_SLOT;
		}
		return slots[id];
	}

	bool shootable()
	{
		return (id >= 22 && id <= 34) || id == 38;
	}
};

/// An array of weapon slots
typedef StaticArray<WeaponSlotData, MAX_WEAPON_SLOTS> WeaponSlots;

enum PlayerWeaponState : int8_t
{
	PlayerWeaponState_Unknown = -1,
	PlayerWeaponState_NoBullets,
	PlayerWeaponState_LastBullet,
	PlayerWeaponState_MoreBullets,
	PlayerWeaponState_Reloading
};

struct PlayerAimData
{
	Vector3 camFrontVector;
	Vector3 camPos;
	float aimZ;
	float camZoom;
	float aspectRatio;
	PlayerWeaponState weaponState;
	uint8_t camMode;
};

struct PlayerBulletData
{
	Vector3 origin;
	Vector3 hitPos;
	Vector3 offset;
	uint8_t weapon;
	PlayerBulletHitType hitType;
	uint16_t hitID;
};

struct PlayerSpectateData
{
	enum ESpectateType
	{
		None,
		Vehicle,
		Player
	};

	bool spectating;
	int spectateID;
	ESpectateType type;
};

/// The player's name status returned when updating their name
enum EPlayerNameStatus
{
	Updated, ///< The name has successfully been updated
	Taken, ///< The name is already taken by another player
	Invalid ///< The name is invalid
};

