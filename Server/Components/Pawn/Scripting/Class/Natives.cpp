#include "../Types.hpp"
#include "Server/Components/Classes/classes.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(AddPlayerClass, bool(int modelid, Vector3 position, float angle, uint8_t weapon1, uint32_t weapon1_ammo, uint8_t weapon2, uint32_t weapon2_ammo, uint8_t weapon3, uint32_t weapon3_ammo))
{
    IClassesComponent* component = PawnManager::Get()->classes;
    if (component) {

        WeaponSlots slots = {
            WeaponSlotData { weapon1, weapon1_ammo },
            WeaponSlotData { weapon2, weapon2_ammo },
            WeaponSlotData { weapon3, weapon3_ammo }
        };

        PlayerClass* _class = component->create(modelid, NO_TEAM, position, angle, slots);
        if (_class) {
            return true;
        }
    }
    return false;
}

SCRIPT_API(AddPlayerClassEx, bool(int teamid, int modelid, Vector3 position, float angle, uint8_t weapon1, uint32_t weapon1_ammo, uint8_t weapon2, uint32_t weapon2_ammo, uint8_t weapon3, uint32_t weapon3_ammo))
{
    IClassesComponent* component = PawnManager::Get()->classes;
    if (component) {

        WeaponSlots slots = {
            WeaponSlotData { weapon1, weapon1_ammo },
            WeaponSlotData { weapon2, weapon2_ammo },
            WeaponSlotData { weapon3, weapon3_ammo }
        };

        PlayerClass* _class = component->create(modelid, teamid, position, angle, slots);
        if (_class) {
            return true;
        }
    }
    return false;
}

SCRIPT_API(SetSpawnInfo, bool(IPlayer& player, int team, int skin, Vector3 position, float angle, uint8_t weapon1, uint8_t weapon2, uint8_t weapon3, uint32_t weapon1_ammo, uint32_t weapon2_ammo, uint32_t weapon3_ammo))
{
    IPlayerClassData* classData = player.queryData<IPlayerClassData>();
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
