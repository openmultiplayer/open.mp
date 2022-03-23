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

SCRIPT_API(SetSpawnInfo, bool(IPlayer& player, int team, int skin, Vector3 position, float angle, uint8_t weapon1, uint8_t weapon2, uint8_t weapon3, uint32_t weapon1_ammo, uint32_t weapon2_ammo, uint32_t weapon3_ammo))
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
