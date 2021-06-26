#pragma once

#include "values.hpp"
#include "entity.hpp"

struct IPlayer;

struct IVehicle : public IEntity {
	virtual const std::array<IPlayer*, MAX_SEATS>& getPassengers() = 0;
};

struct VehicleEventHandler {
	virtual void onDeath(int reason) {}
};

struct IVehiclePool : public IEventDispatcherPool<IVehicle, MAX_VEHICLES, VehicleEventHandler> {
	virtual std::array<uint8_t, MAX_VEHICLE_MODELS>& models() = 0;
};
