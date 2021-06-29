#pragma once

#include <array>

struct IPlayer;

struct WeaponSlotData {
	uint32_t id;
	uint32_t ammo;
};

typedef std::array<WeaponSlotData, MAX_WEAPON_SLOTS> WeaponSlots;

struct IClass : public IIDProvider {
	virtual int& team() = 0;
	virtual int& skin() = 0;
	virtual vector3& spawn() = 0;
	virtual float& angle() = 0;
	virtual WeaponSlots& weapons() = 0;
};

struct ClassEventHandler {
	virtual bool onPlayerRequestClass(IPlayer& player, unsigned int classId) { return true; }
};

struct IClassPool : public IEventDispatcherPool<IClass, MAX_CLASSES, ClassEventHandler> {

};
