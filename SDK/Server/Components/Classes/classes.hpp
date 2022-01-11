#pragma once

#include <sdk.hpp>

/// Player class interface
struct PlayerClass : PoolIDProvider {
    int team; ///< The class's team
    int skin; ///< The class's skin ID
    Vector3 spawn; ///< The class's spawn position
    float angle; ///< The class's angle
    WeaponSlots weapons; ///< The class's weapons

    PlayerClass(int skin, int team, Vector3 spawn, float angle, const WeaponSlots& weapons)
        : team(team)
        , skin(skin)
        , spawn(spawn)
        , angle(angle)
        , weapons(weapons)
    {
    }
};

static const UUID PlayerClassData_UUID = UUID(0x185655ded843788b);
struct IPlayerClassData : public IPlayerData {
    PROVIDE_UUID(PlayerClassData_UUID)

    virtual const PlayerClass& getClass() = 0;
    virtual void setSpawnInfo(const PlayerClass& info) = 0;
};

/// The player class event handler
struct ClassEventHandler {
    virtual bool onPlayerRequestClass(IPlayer& player, unsigned int classId) { return true; }
};

static const UUID ClassesComponent_UUID = UUID(0x8cfb3183976da208);
struct IClassesComponent : public IPoolComponent<PlayerClass, CLASS_POOL_SIZE> {
    PROVIDE_UUID(ClassesComponent_UUID)

    virtual IEventDispatcher<ClassEventHandler>& getEventDispatcher() = 0;

    /// Attempt to create a new class from params
    /// @return A pointer to the new class or nullptr if unable to create
    virtual PlayerClass* create(int skin, int team, Vector3 spawn, float angle, const WeaponSlots& weapons) = 0;
};
