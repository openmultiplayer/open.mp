#pragma once

#include <sdk.hpp>

/// Player class interface
struct IClass : public IIDProvider {
	/// Get the class's team
	virtual int& team() = 0;

	/// Get the class's skin ID
	virtual int& skin() = 0;

	/// Get the class's spawn position
	virtual vector3& spawn() = 0;

	/// Get the class's spawn angle
	virtual float& angle() = 0;

	/// Get the class's spawn weapons
	virtual WeaponSlots& weapons() = 0;
};

static const UUID PlayerClassData_UUID = UUID(0x185655ded843788b);
struct IPlayerClassData : public IPlayerData {
	PROVIDE_UUID(PlayerClassData_UUID)

	virtual IClass& getClass() = 0;
};

/// The player class event handler
struct ClassEventHandler {
	virtual bool onPlayerRequestClass(IPlayer& player, unsigned int classId) { return true; }
};

/// The player class pool
struct IClassPool : public IEventDispatcherPool<IClass, MAX_CLASSES, ClassEventHandler> {

};

static const UUID ClassesPlugin_UUID = UUID(0x8cfb3183976da208);
struct IClassesPlugin : public IPlugin {
	PROVIDE_UUID(ClassesPlugin_UUID)

	virtual IClassPool& getClasses() = 0;
};
