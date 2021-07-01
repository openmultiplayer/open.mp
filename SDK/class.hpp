#pragma once

#include <array>
#include <pool.hpp>

struct IPlayer;

/// Holds weapon slot data
struct WeaponSlotData {
	uint32_t id;
	uint32_t ammo;
};

/// An array of weapon slots
typedef std::array<WeaponSlotData, MAX_WEAPON_SLOTS> WeaponSlots;

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

/// The player class event handler
struct ClassEventHandler {
	virtual bool onPlayerRequestClass(IPlayer& player, unsigned int classId) { return true; }
};

/// The player class pool
struct IClassPool : public IEventDispatcherPool<IClass, MAX_CLASSES, ClassEventHandler> {

};
