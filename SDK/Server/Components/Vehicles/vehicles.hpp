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

enum VehicleSCMEvent : uint32_t {
	VehicleSCMEvent_SetPaintjob = 1,
	VehicleSCMEvent_AddComponent,
	VehicleSCMEvent_SetColour,
	VehicleSCMEvent_EnterExitModShop
};

enum VehicleComponentSlot {
	VehicleComponent_None = -1,
	VehicleComponent_Spoiler = 0,
	VehicleComponent_Hood = 1,
	VehicleComponent_Roof = 2,
	VehicleComponent_SideSkirt = 3,
	VehicleComponent_Lamps = 4,
	VehicleComponent_Nitro = 5,
	VehicleComponent_Exhaust = 6,
	VehicleComponent_Wheels = 7,
	VehicleComponent_Stereo = 8,
	VehicleComponent_Hydraulics = 9,
	VehicleComponent_FrontBumper = 10,
	VehicleComponent_RearBumper = 11,
	VehicleComponent_VentRight = 12,
	VehicleComponent_VentLeft = 13,
};

/// A vehicle interface
struct IVehicle : public IEntity {
	/// Get an array of passengers
	virtual const std::array<IPlayer*, MAX_SEATS>& getPassengers() = 0;

	/// Set the inital spawn data of the vehicle
	virtual void setSpawnData(VehicleSpawnData data) = 0;

	/// Checks if player has the vehicle streamed in for themselves
	virtual bool isStreamedInForPlayer(const IPlayer& player) const = 0;

	/// Streams in the vehicle for a specific player
	virtual void streamInForPlayer(IPlayer& player) = 0;

	/// Streams out the vehicle for a specific player
	virtual void streamOutForPlayer(IPlayer& player) = 0;

	/// Set the vehicle's colour
	virtual void setColour(int col1, int col2) = 0;

	/// Set the vehicle's health
	virtual void setHealth(float Health) = 0;

	/// Update the vehicle from a sync packet
	virtual bool updateFromSync(const NetCode::Packet::PlayerVehicleSync& vehicleSync, IPlayer& player) = 0;

	/// Sets the current driver of the vehicle
	virtual void setDriver(IPlayer* player) = 0;

	/// Returns the current driver of the vehicle
	virtual IPlayer* getDriver() = 0;

	/// Sets the vehicle's number plate
	virtual void setPlate(String plate) = 0;

	/// Get the vehicle's number plate
	virtual const String& getPlate() = 0;

	/// Sets the vehicle's damage status
	virtual void setDamageStatus(int PanelStatus, int DoorStatus, uint8_t LightStatus, uint8_t TyreStatus, IPlayer* vehicleUpdater = nullptr) = 0;

	/// Gets the vehicle's damage status
	virtual void getDamageStatus(int& PanelStatus, int& DoorStatus, uint8_t& LightStatus, uint8_t& TyreStatus) = 0;

	/// Sets the vehicle's paintjob
	virtual void setPaintJob(int paintjob) = 0;
	
	/// Gets the vehicle's paintjob
	virtual int getPaintJob() = 0;

	/// Adds a component to the vehicle.
	virtual void addComponent(int component) = 0;

	/// Gets the vehicle's component in a designated slot
	virtual int getComponentInSlot(int slot) = 0;

	/// Removes a component from the vehicle.
	virtual void removeComponent(int component) = 0;

	virtual void putPlayer(int VehicleID, int SeatID) = 0;
	
	virtual void removePlayer() = 0;
};

/// A vehicle event handler
struct VehicleEventHandler {
	virtual void onStreamIn(IVehicle& vehicle, IPlayer& player) {}
	virtual void onStreamOut(IVehicle& vehicle, IPlayer& player) {}
	virtual void onDeath(IVehicle& vehicle, int reason) {}
	virtual void onPlayerEnterVehicle(IPlayer& player, IVehicle& vehicle, bool passenger) {}
	virtual void onPlayerExitVehicle(IPlayer& player, IVehicle& vehicle) {}
	virtual void onDamageStatusUpdate(IVehicle& vehicle, IPlayer& player) {}
	virtual bool onPaintJob(IPlayer& player, IVehicle& vehicle, int paintJob) { return true; }
	virtual bool onMod(IPlayer& player, IVehicle& vehicle, int component) { return true; }
	virtual bool onRespray(IPlayer& player, IVehicle& vehicle, int colour1, int colour2) { return true; }
	virtual void onEnterExitModShop(IPlayer& player, bool enterexit, int interiorID) {}
};

/// A vehicle pool
static const UUID VehiclePlugin_UUID = UUID(0x3f1f62ee9e22ab19);
struct IVehiclesPlugin : public IPlugin, public IPool<IVehicle, MAX_VEHICLES> {
	PROVIDE_UUID(VehiclePlugin_UUID)

	/// Get the number of model instances for each model
	virtual std::array<uint8_t, MAX_VEHICLE_MODELS>& models() = 0;

	virtual IVehicle* create(int modelID, glm::vec3 position, float Z = 0.0f, int colour1 = 0, int colour2 = 0, int respawnDelay = -1) = 0;
	virtual IVehicle* create(VehicleSpawnData data) = 0;

	virtual IEventDispatcher<VehicleEventHandler>& getEventDispatcher() = 0;
};

/// Player vehicle data
static const UUID SomePlayerData_UUID = UUID(0xa960485be6c70fb2);
struct IPlayerVehicleData : public IPlayerData {
	PROVIDE_UUID(SomePlayerData_UUID)

	/// Get the player's vehicle
	/// Returns nullptr if they aren't in a vehicle
	virtual IVehicle* getVehicle() = 0;

	/// Get the player's seat
	/// Returns -1 if they aren't in a vehicle.
	virtual int getSeat() const = 0;

	virtual void setVehicle(IVehicle* vehicle) = 0;

	virtual void setSeat(int seat) = 0;

};