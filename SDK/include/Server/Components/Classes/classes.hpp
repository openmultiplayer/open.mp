#pragma once

#include <sdk.hpp>

/// Player class interface
struct PlayerClass
{
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

/// The class interface returned by the class pool
struct IClass : public IExtensible, public IIDProvider
{
	/// Get the player class associated with the class interface
	virtual const PlayerClass& getClass() = 0;

	/// Set the player class data
	virtual void setClass(const PlayerClass& data) = 0;
};

static const UID PlayerClassData_UID = UID(0x185655ded843788b);
struct IPlayerClassData : public IExtension
{
	PROVIDE_EXT_UID(PlayerClassData_UID)

	virtual const PlayerClass& getClass() = 0;
	virtual void setSpawnInfo(const PlayerClass& info) = 0;
	virtual void spawnPlayer() = 0;
};

/// The player class event handler
struct ClassEventHandler
{
	virtual bool onPlayerRequestClass(IPlayer& player, unsigned int classId) { return true; }
};

static const UID ClassesComponent_UID = UID(0x8cfb3183976da208);
struct IClassesComponent : public IPoolComponent<IClass>
{
	PROVIDE_UID(ClassesComponent_UID)

	virtual IEventDispatcher<ClassEventHandler>& getEventDispatcher() = 0;

	/// Attempt to create a new class from params
	/// @return A pointer to the new class or nullptr if unable to create
	virtual IClass* create(int skin, int team, Vector3 spawn, float angle, const WeaponSlots& weapons) = 0;
};
