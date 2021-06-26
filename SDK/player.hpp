#pragma once

#include <string>
#include "entity.hpp"

struct IVehicle;
struct INetwork;
struct INetworkBitStream;

struct IPlayer : public IEntity {
	virtual void setNetworkData(INetwork* network, const std::string& IP, unsigned short port) = 0;
	virtual INetwork& getNetwork() = 0;
	virtual IVehicle* getVehicle() = 0;
	virtual int& versionNumber() = 0;
	virtual char& modded() = 0;
	virtual std::string& name() = 0;
	virtual unsigned int& challengeResponse() = 0;
	virtual std::string& key() = 0;
	virtual std::string& versionString() = 0;
};

struct PlayerEventHandler {
	virtual void onConnect(IPlayer& player, INetworkBitStream& bs) {}
	virtual void onDisconnect(IPlayer& player, int reason) {}
};

struct IPlayerPool : public IEventDispatcherPool<IPlayer, MAX_PLAYERS, PlayerEventHandler> {
};

