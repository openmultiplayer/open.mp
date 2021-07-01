#pragma once

#include "values.hpp"
#include "entity.hpp"
#include "pool.hpp"

struct IPlayer;

/// A vehicle interface
struct IVehicle : public IEntity {
	/// Get an array of passengers
	virtual const std::array<IPlayer*, MAX_SEATS>& getPassengers() = 0;
};

/// A vehicle event handler
struct VehicleEventHandler {
	virtual void onDeath(int reason) {}
};

/// A vehicle pool
struct IVehiclePool : public IEventDispatcherPool<IVehicle, MAX_VEHICLES, VehicleEventHandler> {
	/// Get the number of model instances for each model
	virtual std::array<uint8_t, MAX_VEHICLE_MODELS>& models() = 0;
};
