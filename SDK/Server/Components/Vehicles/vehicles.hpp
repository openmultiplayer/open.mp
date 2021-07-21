#pragma once

#include <sdk.hpp>
#include <netcode.hpp>

struct IPlayer;

struct VehicleSpawnData {
	int modelID;
	glm::vec3 position;
	float zRotation;
	int colour1;
	int colour2;
	int respawnDelay;
};

/// A vehicle interface
struct IVehicle : public IEntity {
	/// Get an array of passengers
	virtual const StaticArray<IPlayer*, MAX_SEATS>& getPassengers() = 0;

	/// Set the inital spawn data of the vehicle
	virtual void setSpawnData(VehicleSpawnData data) = 0;

	/// Checks if player has the vehicle streamed in for themselves
	virtual bool isStreamedInForPlayer(const IPlayer& player) const = 0;

	/// Streams in the vehicle for a specific player
	virtual void streamInForPlayer(const IPlayer& player) = 0;

	/// Streams out the vehicle for a specific player
	virtual void streamOutForPlayer(const IPlayer& player) = 0;

	/// Set the vehicle's colour
	virtual void setColour(int col1, int col2) = 0;

	/// Set the vehicle's health
	virtual void setHealth(float Health) = 0;

	/// Update the vehicle from a sync packet
	virtual bool updateFromSync(const NetCode::Packet::PlayerVehicleSync& vehicleSync) = 0;
};

/// A vehicle event handler
struct VehicleEventHandler {
	virtual void onStreamIn(IVehicle& vehicle, IPlayer& player) {}
	virtual void onStreamOut(IVehicle& vehicle, IPlayer& player) {}
	virtual void onDeath(IVehicle& vehicle, int reason) {}
	virtual void onPlayerEnterVehicle(IPlayer& player, IVehicle& vehicle, bool passenger) {}
	virtual void onPlayerExitVehicle(IPlayer& player, IVehicle& vehicle) {}
};

/// A vehicle pool
static const UUID VehiclePlugin_UUID = UUID(0x3f1f62ee9e22ab19);
struct IVehiclesPlugin : public IPlugin, public IPool<IVehicle, VEHICLE_POOL_SIZE> {
	PROVIDE_UUID(VehiclePlugin_UUID)

	/// Get the number of model instances for each model
	virtual StaticArray<uint8_t, MAX_VEHICLE_MODELS>& models() = 0;

	virtual IVehicle* create(int modelID, glm::vec3 position, float Z = 0.0f, int colour1 = 0, int colour2 = 0, int respawnDelay = -1) = 0;
	virtual IVehicle* create(VehicleSpawnData data) = 0;
};
