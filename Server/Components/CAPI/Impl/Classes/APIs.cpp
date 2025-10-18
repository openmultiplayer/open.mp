/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Class_Add, objectPtr(uint8_t team, int skin, float x, float y, float z, float angle, uint8_t weapon1, uint32_t ammo1, uint8_t weapon2, uint32_t ammo2, uint8_t weapon3, uint32_t ammo3, int* id))
{
	IClassesComponent* component = ComponentManager::Get()->classes;
	if (component)
	{

		WeaponSlots slots = {
			WeaponSlotData { weapon1, ammo1 },
			WeaponSlotData { weapon2, ammo2 },
			WeaponSlotData { weapon3, ammo3 }
		};

		IClass* class_ = component->create(skin, team, { x, y, z }, angle, slots);

		if (class_)
		{
			*id = class_->getID();
			return class_;
		}
	}

	return nullptr;
}

OMP_CAPI(Class_FromID, objectPtr(int classid))
{
	IClassesComponent* component = ComponentManager::Get()->classes;
	if (component)
	{
		return component->get(classid);
	}
	return nullptr;
}

OMP_CAPI(Class_GetID, int(objectPtr cls))
{
	POOL_ENTITY_RET(classes, IClass, cls, class_, 0xFFFF);
	return class_->getID();
}

OMP_CAPI(Player_SetSpawnInfo, bool(objectPtr player, uint8_t team, int skin, float x, float y, float z, float angle, uint8_t weapon1, uint32_t ammo1, uint8_t weapon2, uint32_t ammo2, uint8_t weapon3, uint32_t ammo3))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerClassData* classData = queryExtension<IPlayerClassData>(player_);
	if (classData)
	{
		WeaponSlots slots = {
			WeaponSlotData { weapon1, ammo1 },
			WeaponSlotData { weapon2, ammo2 },
			WeaponSlotData { weapon3, ammo3 }
		};

		classData->setSpawnInfo(PlayerClass(skin, team, { x, y, z }, angle, slots));
		return true;
	}
	return false;
}

OMP_CAPI(Player_GetSpawnInfo, bool(objectPtr player, uint8_t* team, int* skin, float* x, float* y, float* z, float* angle, uint8_t* weapon1, uint32_t* ammo1, uint8_t* weapon2, uint32_t* ammo2, uint8_t* weapon3, uint32_t* ammo3))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerClassData* playerData = queryExtension<IPlayerClassData>(player_);
	if (playerData)
	{
		const PlayerClass& data = playerData->getClass();
		*team = uint8_t(data.team);
		*skin = data.skin;
		*x = data.spawn.x;
		*y = data.spawn.y;
		*z = data.spawn.z;
		*angle = data.angle;
		*weapon1 = data.weapons[0].id;
		*ammo1 = data.weapons[0].ammo;
		*weapon2 = data.weapons[1].id;
		*ammo2 = data.weapons[1].ammo;
		*weapon3 = data.weapons[2].id;
		*ammo3 = data.weapons[2].ammo;
		return true;
	}
	return false;
}

OMP_CAPI(Class_Count, int())
{
	IClassesComponent* component = ComponentManager::Get()->classes;
	if (component)
	{
		int count = component->count();
		return count;
	}
	return 0;
}

OMP_CAPI(Class_GetData, bool(objectPtr classptr, uint8_t* teamid, int* skin, float* x, float* y, float* z, float* angle, uint8_t* weapon1, uint32_t* weapon1_ammo, uint8_t* weapon2, uint32_t* weapon2_ammo, uint8_t* weapon3, uint32_t* weapon3_ammo))
{
	POOL_ENTITY_RET(classes, IClass, classptr, class_, false);
	const PlayerClass& data = class_->getClass();
	*teamid = uint8_t(data.team);
	*skin = data.skin;
	*x = data.spawn.x;
	*y = data.spawn.y;
	*z = data.spawn.z;
	*angle = data.angle;
	*weapon1 = data.weapons[0].id;
	*weapon1_ammo = data.weapons[0].ammo;
	*weapon2 = data.weapons[1].id;
	*weapon2_ammo = data.weapons[1].ammo;
	*weapon3 = data.weapons[2].id;
	*weapon3_ammo = data.weapons[2].ammo;
	return true;
}

OMP_CAPI(Class_Edit, bool(objectPtr classptr, uint8_t teamid, int skin, float x, float y, float z, float angle, uint8_t weapon1, uint32_t ammo1, uint8_t weapon2, uint32_t ammo2, uint8_t weapon3, uint32_t ammo3))
{
	POOL_ENTITY_RET(classes, IClass, classptr, class_, false);
	WeaponSlots weapons;
	weapons[0].id = weapon1;
	weapons[0].ammo = ammo1;
	weapons[1].id = weapon2;
	weapons[1].ammo = ammo2;
	weapons[2].id = weapon3;
	weapons[2].ammo = ammo3;
	PlayerClass data = PlayerClass(skin, teamid, { x, y, z }, angle, weapons);
	class_->setClass(data);
	return true;
}
