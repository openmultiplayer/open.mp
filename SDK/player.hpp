#pragma once

#include <string>
#include "network.hpp"
#include "entity.hpp"
#include "pool.hpp"

/// Holds weapon slot data
struct WeaponSlotData {
	uint8_t id;
	uint32_t ammo;

	uint8_t slot()
	{
		static const uint8_t slots[] = { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 10, 10, 10, 10, 10, 10, 8, 8, 8, INVALID_WEAPON_SLOT, INVALID_WEAPON_SLOT, INVALID_WEAPON_SLOT, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 4, 6, 6, 7, 7, 7, 7, 8, 12, 9, 9, 9, 11, 11, 11 };
		if (id >= GLM_COUNTOF(slots))
		{
			return INVALID_WEAPON_SLOT;
		}
		return slots[id];
	}
};

/// An array of weapon slots
typedef std::array<WeaponSlotData, MAX_WEAPON_SLOTS> WeaponSlots;

/// A player data interface for per-player data
struct IPlayerData : public IUUIDProvider {
	/// Frees the player data object, called on player disconnect, usually defaults to delete this
	virtual void free() = 0;
};

struct IVehicle;

/// A player interface
struct IPlayer : public IEntity, public INetworkPeer {
	virtual ~IPlayer() {}

	/// Get the player's current vehicle
	virtual IVehicle* getVehicle() = 0;

	/// Get the player's version number
	virtual int& versionNumber() = 0;

	/// Get the player's modded status
	virtual char& modded() = 0;

	/// Get the player's name
	virtual String& name() = 0;

	/// Get the player's challenge response
	virtual unsigned int& challengeResponse() = 0;

	/// Get the player's key
	virtual String& key() = 0;

	/// Get the player's version string
	virtual String& versionString() = 0;

	virtual void giveWeapon(WeaponSlotData weapon) = 0;

	virtual void resetWeapons() = 0;

	virtual void setArmedWeapon(uint32_t weapon) = 0;

	virtual Color& color() = 0;

	virtual void streamInPlayer(IPlayer& other) = 0;

	virtual bool isPlayerStreamedIn(IPlayer& other) = 0;

	virtual void streamOutPlayer(IPlayer& other) = 0;

	/// Add data associated with the player, preferrably used on player connect
	virtual void addData(IPlayerData* playerData) = 0;

	/// Query player data by its ID
	/// @param id The UUID of the data
	/// @return A pointer to the data or nullptr if not available
	virtual IPlayerData* queryData(UUID id) = 0;

	/// Query player data by its type
	/// @typeparam PlayerDataT The data type, must derive from IPlayerData
	template <class PlayerDataT>
	PlayerDataT* queryData() {
		static_assert(std::is_base_of<IPlayerData, PlayerDataT>::value, "queryData parameter must inherit from IPlayerData");
		return static_cast<PlayerDataT*>(queryData(PlayerDataT::IID));
	}
};

/// A player event handler
struct PlayerEventHandler {
	virtual IPlayerData* onPlayerDataRequest(IPlayer& player) { return nullptr; }
	virtual void onConnect(IPlayer& player) {}
	virtual void onDisconnect(IPlayer& player, int reason) {}
	virtual bool onPlayerRequestSpawn(IPlayer& player) { return true; }
	virtual void onSpawn(IPlayer& player) {}
};

/// A player pool interface
struct IPlayerPool : public IEventDispatcherPool<IPlayer, MAX_PLAYERS, PlayerEventHandler> {

};
