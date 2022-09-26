/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "Server/Components/Classes/classes.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(AddPlayerClass, int(int skin, Vector3 spawnPosition, float angle, uint8_t weapon1, uint32_t ammo1, uint8_t weapon2, uint32_t ammo2, uint8_t weapon3, uint32_t ammo3))
{
	IClassesComponent* component = PawnManager::Get()->classes;
	if (component)
	{

		WeaponSlots slots = {
			WeaponSlotData { weapon1, ammo1 },
			WeaponSlotData { weapon2, ammo2 },
			WeaponSlotData { weapon3, ammo3 }
		};

		IClass* _class = component->create(skin, TEAM_NONE, spawnPosition, angle, slots);

		if (_class)
		{
			return _class->getID();
		}
	}

	return 0;
}

SCRIPT_API(AddPlayerClassEx, int(uint8_t team, int skin, Vector3 spawnPosition, float angle, uint8_t weapon1, uint32_t ammo1, uint8_t weapon2, uint32_t ammo2, uint8_t weapon3, uint32_t ammo3))
{
	IClassesComponent* component = PawnManager::Get()->classes;
	if (component)
	{

		WeaponSlots slots = {
			WeaponSlotData { weapon1, ammo1 },
			WeaponSlotData { weapon2, ammo2 },
			WeaponSlotData { weapon3, ammo3 }
		};

		IClass* _class = component->create(skin, team, spawnPosition, angle, slots);
		if (_class)
		{
			return _class->getID();
		}
	}
	return 0;
}

SCRIPT_API(SetSpawnInfo, bool(IPlayer& player, int team, int skin, Vector3 spawnPosition, float angle, uint8_t weapon1, uint32_t ammo1, uint8_t weapon2, uint32_t ammo2, uint8_t weapon3, uint32_t ammo3))
{
	IPlayerClassData* classData = queryExtension<IPlayerClassData>(player);
	if (classData)
	{
		WeaponSlots slots = {
			WeaponSlotData { weapon1, ammo1 },
			WeaponSlotData { weapon2, ammo2 },
			WeaponSlotData { weapon3, ammo3 }
		};

		classData->setSpawnInfo(PlayerClass(skin, team, spawnPosition, angle, slots));
		return true;
	}
	return false;
}

SCRIPT_API(GetSpawnInfo, bool(IPlayer& player, uint8_t& team, int& skin, Vector3& spawnPosition, float& angle, int& weapon1, int& ammo1, int& weapon2, int& ammo2, int& weapon3, int& ammo3))
{
	IPlayerClassData* playerData = queryExtension<IPlayerClassData>(player);
	if (playerData)
	{
		const PlayerClass& data = playerData->getClass();
		team = data.team;
		skin = data.skin;
		spawnPosition = data.spawn;
		angle = data.angle;
		weapon1 = data.weapons[0].id;
		ammo1 = data.weapons[0].ammo;
		weapon2 = data.weapons[1].id;
		ammo2 = data.weapons[1].ammo;
		weapon3 = data.weapons[2].id;
		ammo3 = data.weapons[2].ammo;
		return true;
	}
	return false;
}

SCRIPT_API(GetAvailableClasses, int())
{
	IClassesComponent* component = PawnManager::Get()->classes;
	if (component)
	{
		return component->count();
	}
	return 0;
}

SCRIPT_API(GetPlayerClass, bool(IClass& class_, int& teamid, int& skin, Vector3& spawnPos, float& angle, int& weapon1, int& weapon1_ammo, int& weapon2, int& weapon2_ammo, int& weapon3, int& weapon3_ammo))
{
	const PlayerClass& data = class_.getClass();
	teamid = data.team;
	skin = data.skin;
	spawnPos = data.spawn;
	angle = data.angle;
	weapon1 = data.weapons[0].id;
	weapon1_ammo = data.weapons[0].ammo;
	weapon2 = data.weapons[1].id;
	weapon2_ammo = data.weapons[1].ammo;
	weapon3 = data.weapons[2].id;
	weapon3_ammo = data.weapons[2].ammo;
	return true;
}

SCRIPT_API(EditPlayerClass, bool(IClass& class_, int teamid, int skin, Vector3 spawnPos, float angle, int weapon1, int weapon1_ammo, int weapon2, int weapon2_ammo, int weapon3, int weapon3_ammo))
{
	WeaponSlots weapons;
	weapons[0].id = weapon1;
	weapons[0].ammo = weapon1_ammo;
	weapons[1].id = weapon2;
	weapons[1].ammo = weapon2_ammo;
	weapons[2].id = weapon3;
	weapons[2].ammo = weapon3_ammo;
	PlayerClass data = PlayerClass(skin, teamid, spawnPos, angle, weapons);
	class_.setClass(data);
	return true;
}
