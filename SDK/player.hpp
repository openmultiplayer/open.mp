#pragma once

#include <string>
#include "entity.hpp"
#include "class.hpp"

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
	virtual std::string& name() = 0;

	/// Get the player's challenge response
	virtual unsigned int& challengeResponse() = 0;

	/// Get the player's key
	virtual std::string& key() = 0;

	/// Get the player's version string
	virtual std::string& versionString() = 0;

	/// Get the player's class data
	virtual IClass& classData() = 0;

	/// Add data associated with the player, preferrably used on player connect
	virtual void addData(IPlayerData* playerData) = 0;

	/// Remove data associated with the player
	/// @note You don't need to do so manually as any player data is destroyed on player connect
	virtual void removeData(IPlayerData* playerData) = 0;

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
	virtual void onConnect(IPlayer& player) {}
	virtual void onDisconnect(IPlayer& player, int reason) {}
	virtual bool onPlayerRequestSpawn(IPlayer& player) { return true; }
};

/// A player pool interface
struct IPlayerPool : public IEventDispatcherPool<IPlayer, MAX_PLAYERS, PlayerEventHandler> {

};
