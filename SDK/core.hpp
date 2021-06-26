#pragma once

#include "types.hpp"
#include "values.hpp"
#include "events.hpp"
#include "network.hpp"
#include "player.hpp"
#include "vehicle.hpp"

struct CoreEventHandler {
	virtual void onInit() {}
	virtual void onTick(uint64_t tick) {}
};

struct ICore {
	virtual int getVersion() = 0;
	virtual void printLn(const char* fmt, ...) = 0;
	virtual INetwork& getNetworkInterface() = 0;
	virtual IPlayerPool& getPlayers() = 0;
	virtual IVehiclePool& getVehicles() = 0;
	virtual IEventDispatcher<CoreEventHandler>& getEventDispatcher() = 0;
};
