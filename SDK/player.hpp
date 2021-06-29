#pragma once

#include <string>
#include "entity.hpp"
#include "class.hpp"

struct IVehicle;

/// A player interface
struct IPlayer : public IEntity, public INetworkPeer {
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

	virtual IClass& classData() = 0;
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
