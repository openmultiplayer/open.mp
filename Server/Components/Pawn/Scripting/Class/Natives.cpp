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

SCRIPT_API(AddPlayerClass, int(int modelid, Vector3 position, float angle, uint8_t weapon1, uint32_t weapon1_ammo, uint8_t weapon2, uint32_t weapon2_ammo, uint8_t weapon3, uint32_t weapon3_ammo))
{
    IClassesComponent* component = PawnManager::Get()->classes;
    if (component) {

        WeaponSlots slots = {
            WeaponSlotData { weapon1, weapon1_ammo },
            WeaponSlotData { weapon2, weapon2_ammo },
            WeaponSlotData { weapon3, weapon3_ammo }
        };

        IClass* _class = component->create(modelid, TEAM_NONE, position, angle, slots);

        if (_class) {
            return _class->getID();
        }
    }

    return 0;
}

SCRIPT_API(AddPlayerClassEx, int(int teamid, int modelid, Vector3 position, float angle, uint8_t weapon1, uint32_t weapon1_ammo, uint8_t weapon2, uint32_t weapon2_ammo, uint8_t weapon3, uint32_t weapon3_ammo))
{
    IClassesComponent* component = PawnManager::Get()->classes;
    if (component) {

        WeaponSlots slots = {
            WeaponSlotData { weapon1, weapon1_ammo },
            WeaponSlotData { weapon2, weapon2_ammo },
            WeaponSlotData { weapon3, weapon3_ammo }
        };

        IClass* _class = component->create(modelid, teamid, position, angle, slots);
        if (_class) {
            return _class->getID();
        }
    }
    return 0;
}

SCRIPT_API(SetSpawnInfo, bool(IPlayer& player, int team, int skin, Vector3 position, float angle, uint8_t weapon1, uint32_t weapon1_ammo, uint8_t weapon2, uint32_t weapon2_ammo, uint8_t weapon3, uint32_t weapon3_ammo))
{
    IPlayerClassData* classData = queryExtension<IPlayerClassData>(player);
    if (classData) {
        WeaponSlots slots = {
            WeaponSlotData { weapon1, weapon1_ammo },
            WeaponSlotData { weapon2, weapon2_ammo },
            WeaponSlotData { weapon3, weapon3_ammo }
        };

        classData->setSpawnInfo(PlayerClass(skin, team, position, angle, slots));
    }
    return false;
}

SCRIPT_API(GetSpawnInfo, bool(IPlayer& player, int& teamid, int& modelid, Vector3& spawnPos, float& z_angle, int& weapon1, int& weapon1_ammo, int& weapon2, int& weapon2_ammo, int& weapon3, int& weapon3_ammo))
{
    IPlayerClassData* playerData = queryExtension<IPlayerClassData>(player);
    if (playerData) {
        const PlayerClass& data = playerData->getClass();
        teamid = data.team;
        modelid = data.skin;
        spawnPos = data.spawn;
        z_angle = data.angle;
        weapon1 = data.weapons[0].id;
        weapon1_ammo = data.weapons[0].ammo;
        weapon2 = data.weapons[1].id;
        weapon2_ammo = data.weapons[1].ammo;
        weapon3 = data.weapons[2].id;
        weapon3_ammo = data.weapons[2].ammo;
        return true;
    }
    return false;
}

SCRIPT_API(GetAvailableClasses, int())
{
    IClassesComponent* component = PawnManager::Get()->classes;
    if (component) {
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
