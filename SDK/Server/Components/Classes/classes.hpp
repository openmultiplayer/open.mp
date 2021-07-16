#pragma once

#include <sdk.hpp>

/// Player class interface
struct PlayerClass {
	int team; ///< The class's team
	int skin; ///< The class's skin ID
	Vector3 spawn; ///< The class's spawn position
	float angle; ///< The class's angle
	WeaponSlots weapons; ///< The class's weapons
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

static const UUID ClassesPlugin_UUID = UUID(0x8cfb3183976da208);
struct IClassesPlugin : public IPlugin, IPool<PlayerClass, CLASS_POOL_SIZE> {
	PROVIDE_UUID(ClassesPlugin_UUID)

	virtual IEventDispatcher<ClassEventHandler>& getEventDispatcher() = 0;
};
