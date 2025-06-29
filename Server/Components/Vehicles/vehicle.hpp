/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "Server/Components/Vehicles/vehicle_colours.hpp"
#include <Impl/pool_impl.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <chrono>
#include <netcode.hpp>

using namespace Impl;

class VehiclesComponent;

struct VehicleDeathData
{
	bool dead = false;
	TimePoint time;
	int killerID = INVALID_PLAYER_ID; ///< Purposely made an ID instead of a pointer because a player might become invalid between reporting tick and next tick
};

class Vehicle final : public IVehicle, public PoolIDProvider, public NoCopy
{
private:
	Vector3 pos;
	GTAQuat rot;
	int virtualWorld_ = 0;
	VehiclesComponent* pool = nullptr;
	VehicleSpawnData spawnData;
	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> streamedFor_;
	StaticArray<int, MAX_VEHICLE_COMPONENT_SLOT> mods;
	float health = 1000.0f;
	IPlayer* driver = nullptr;
	uint32_t doorDamage = 0;
	uint32_t panelDamage = 0;
	uint8_t lightDamage = 0;
	uint8_t tyreDamage = 0;
	uint8_t paintJob = 0;
	uint8_t interior = 0;
	int32_t bodyColour1 = -1;
	int32_t bodyColour2 = -1;
	uint8_t landingGear = 1;
	bool respawning = false;
	bool detaching = false;
	bool beenOccupied = false;
	FlatHashSet<IPlayer*> passengers;
	HybridString<16> numberPlate = StringView("XYZSR998");
	uint8_t objective;
	uint8_t doorsLocked;
	uint8_t sirenState = 0;
	VehicleDeathData deathData;
	TimePoint timeOfSpawn;
	TimePoint lastOccupiedChange;
	Vector3 velocity = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 angularVelocity = Vector3(0.0f, 0.0f, 0.0f);
	TimePoint trailerUpdateTime;
	Vehicle* trailer = nullptr;
	Vehicle* cab = nullptr;
	StaticArray<IVehicle*, MAX_VEHICLE_CARRIAGES> carriages;
	VehicleParams params;
	uint32_t hydraThrustAngle = 0;
	float trainSpeed = 0.0f;
	int lastDriverPoolID = INVALID_PLAYER_ID;

	/// Update the vehicle occupied status - set beenOccupied to true and update the lastOccupied time.
	void updateOccupied()
	{
		beenOccupied = true;
		lastOccupiedChange = Time::now();
	}

	void setCab(Vehicle* cab)
	{
		this->cab = cab;
	}

	/// Set vehicle to respawn without emitting onRespawn event
	void _respawn();

public:
	int getLastDriverPoolID() const override
	{
		return lastDriverPoolID;
	}
	bool hasBeenOccupied() override
	{
		return beenOccupied;
	}

	const TimePoint& getLastOccupiedTime() override
	{
		return lastOccupiedChange;
	}

	void setLastOccupiedTime(TimePoint time)
	{
		lastOccupiedChange = time;
	}

	const TimePoint& getLastSpawnTime() override
	{
		return timeOfSpawn;
	}

	const VehicleDeathData& getDeathData() const
	{
		return deathData;
	}

	inline void setTimeOfDeath(TimePoint time)
	{
		deathData.time = time;
	}

	void removeFor(int pid, IPlayer& player)
	{
		if (streamedFor_.valid(pid))
		{
			streamedFor_.remove(pid, player);
		}
	}

	/// Sets the vehicle's death state.
	void setDead(IPlayer& killer);

	void unoccupy(IPlayer& player)
	{
		if (driver == &player)
		{
			driver = nullptr;
		}
		else
		{
			passengers.erase(&player);
		}
		updateOccupied();
	}

	Vehicle(VehiclesComponent* pool, const VehicleSpawnData& data)
		: pool(pool)
	{
		carriages.fill(nullptr);
		// Set spawn data then set to respawn without emitting onRespawn event
		setSpawnData(data);
		_respawn();
	}

	~Vehicle();
	void destream();

	int getVirtualWorld() const override
	{
		return virtualWorld_;
	}

	void setVirtualWorld(int vw) override
	{
		virtualWorld_ = vw;
	}

	void setSiren(bool status) override
	{
		spawnData.siren = status;
	}

	uint8_t getSirenState() const override
	{
		return sirenState;
	}

	uint32_t getHydraThrustAngle() const override
	{
		return hydraThrustAngle;
	}

	float getTrainSpeed() const override
	{
		return trainSpeed;
	}

	int getID() const override
	{
		return poolID;
	}

	Vector3 getPosition() const override;

	void setPosition(Vector3 position) override;

	GTAQuat getRotation() const override
	{
		return rot;
	}

	void setRotation(GTAQuat rotation) override
	{
		rot = rotation;
	}

	const FlatPtrHashSet<IPlayer>& streamedForPlayers() const override
	{
		return streamedFor_.entries();
	}

	bool isStreamedInForPlayer(const IPlayer& player) const override
	{
		return streamedFor_.valid(player.getID());
	}

	void setSpawnData(const VehicleSpawnData& data) override
	{
		spawnData = data;

		if (spawnData.colour1 == -1 || spawnData.colour2 == -1)
		{
			int ignore;
			getRandomVehicleColour(spawnData.modelID, spawnData.colour1 == -1 ? spawnData.colour1 : ignore, spawnData.colour2 == -1 ? spawnData.colour2 : ignore);
		}
	}

	const VehicleSpawnData& getSpawnData() override
	{
		return spawnData;
	}

	void streamInForPlayer(IPlayer& player) override;
	void streamOutForPlayer(IPlayer& player) override;

	void streamOutForClient(IPlayer& player);

	bool isOccupied() override
	{
		return driver != nullptr || passengers.size() != 0;
	}

	/// Update the vehicle's data from a player sync packet.
	bool updateFromDriverSync(const VehicleDriverSyncPacket& vehicleSync, IPlayer& player) override;

	/// Update the vehicle's data from an unoccupied sync packet.
	bool updateFromUnoccupied(const VehicleUnoccupiedSyncPacket& unoccupiedSync, IPlayer& player) override;

	/// Update the vehicle from a trailer sync packet
	bool updateFromTrailerSync(const VehicleTrailerSyncPacket& unoccupiedSync, IPlayer& player) override;

	bool updateFromPassengerSync(const VehiclePassengerSyncPacket& passengerSync, IPlayer& player) override;

	/// Sets the vehicle's body colour
	void setColour(int col1, int col2) override;

	/// Get the vehicle's body colour
	Pair<int, int> getColour() const override;

	/// Sets the vehicle's health
	void setHealth(float Health) override;

	/// Gets the vehicle's current health
	float getHealth() override;

	/// Returns the current driver of the vehicle
	IPlayer* getDriver() override
	{
		return driver;
	}

	/// Returns the passengers of the vehicle
	const FlatHashSet<IPlayer*>& getPassengers() override
	{
		return passengers;
	}

	void setPlate(StringView plate) override;
	const StringView getPlate() override;

	void setDamageStatus(int PanelStatus, int DoorStatus, uint8_t LightStatus, uint8_t TyreStatus, IPlayer* vehicleUpdater = nullptr) override;
	void getDamageStatus(int& PanelStatus, int& DoorStatus, int& LightStatus, int& TyreStatus) override;

	/// Sets the vehicle's paintjob
	void setPaintJob(int paintjob) override;

	/// Gets the vehicle's paintjob
	int getPaintJob() override;

	/// Adds a component to the vehicle.
	void addComponent(int component) override;

	/// Gets the vehicle's component in a designated slot
	int getComponentInSlot(int slot) override;

	/// Removes a vehicle's component.
	void removeComponent(int component) override;

	/// Puts the player inside this vehicle.
	void putPlayer(IPlayer& player, int SeatID) override;

	/// Set the vehicle's Z angle.
	void setZAngle(float angle) override;

	/// Gets the vehicle's Z angle.
	float getZAngle() override;

	// Set the vehicle's parameters.
	void setParams(const VehicleParams& params) override;

	// Set the vehicle's parameters for a specific player.
	void setParamsForPlayer(IPlayer& player, const VehicleParams& params) override;

	// Get the vehicle's parameters.
	VehicleParams const& getParams() override { return params; }

	/// Checks if the vehicle is dead.
	bool isDead() override;

	/// Respawns the vehicle.
	void respawn() override;

	/// Get the vehicle's respawn delay.
	Seconds getRespawnDelay() override;

	/// Set the vehicle's respawn delay.
	void setRespawnDelay(Seconds delay) override;

	bool isRespawning() override { return respawning; }

	// Sets (links) the vehicle to an interior.
	void setInterior(int InteriorID) override;

	// Gets the vehicle's interior.
	int getInterior() override;

	void repair() override
	{
		setHealth(1000.f);
		setDamageStatus(0, 0, 0, 0);
		params.bonnet = 0;
		params.boot = 0;
		params.doorDriver = -1;
		params.doorPassenger = -1;
		params.doorBackLeft = -1;
		params.doorBackRight = -1;
	}

	/// Attaches a vehicle as a trailer to this vehicle.
	void attachTrailer(IVehicle& vehicle) override;

	/// Detaches a vehicle from this vehicle.
	void detachTrailer() override;

	/// Checks if the current vehicle is a trailer.
	bool isTrailer() const override { return cab != nullptr; }

	/// Get the current vehicle's attached trailer.
	IVehicle* getTrailer() const override
	{
		return trailer;
	}

	/// Get the current vehicle's cab.
	IVehicle* getCab() const override
	{
		return cab;
	}

	/// Adds a train carriage to the vehicle (ONLY FOR TRAINS).
	void addCarriage(IVehicle* carriage, int pos) override
	{
		if (!carriage)
		{
			return;
		}

		if (spawnData.modelID != 538 && spawnData.modelID != 537)
		{
			return;
		}
		carriages.at(pos) = carriage;
	}
	void updateCarriage(Vector3 pos, Vector3 veloc) override
	{
		this->pos = pos;
		velocity = veloc;
	}

	const StaticArray<IVehicle*, MAX_VEHICLE_CARRIAGES>& getCarriages() override
	{
		return carriages;
	}

	bool isTrainCarriage()
	{
		const int model = getModel();
		if (model == 569 || model == 570)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	/// Sets the velocity of the vehicle.
	void setVelocity(Vector3 velocity) override;

	/// Gets the current velocity of the vehicle.
	Vector3 getVelocity() override
	{
		return velocity;
	}

	/// Sets the angular velocity of the vehicle.
	void setAngularVelocity(Vector3 velocity) override;

	/// Gets the current angular velocity of the vehicle.
	Vector3 getAngularVelocity() override
	{
		return angularVelocity;
	}

	/// Gets the current model ID of the vehicle.
	int getModel() override
	{
		return spawnData.modelID;
	}

	/// Gets the current landing gear state from a ID_VEHICLE_SYNC packet from the latest driver.
	uint8_t getLandingGearState() override
	{
		return landingGear;
	}
};

class PlayerVehicleData final : public IPlayerVehicleData
{
private:
	IPlayer& player;
	Vehicle* vehicle = nullptr;
	int seat = SEAT_NONE;
	int numStreamed = 0;
	bool inModShop = false;
	bool inDriveByMode = false;
	bool cuffed = false;

public:
	PlayerVehicleData(IPlayer& player)
		: player(player)
	{
	}

	void setNumStreamed(int num)
	{
		numStreamed = num;
	}

	/// Get the player's vehicle
	/// Returns nullptr if they aren't in a vehicle
	int getNumStreamed() const
	{
		return numStreamed;
	}

	void setVehicle(Vehicle* vehicle, int seat)
	{
		this->vehicle = vehicle;
		this->seat = seat;
	}

	/// Get the player's vehicle
	/// Returns nullptr if they aren't in a vehicle
	IVehicle* getVehicle() override
	{
		return vehicle;
	}

	void resetVehicle() override
	{
		// Remove player from vehicle occupants lists.
		if (vehicle)
		{
			vehicle->unoccupy(player);
		}
		vehicle = nullptr;
		seat = SEAT_NONE;
	}

	/// Get the player's seat
	/// Returns SEAT_NONE if they aren't in a vehicle.
	int getSeat() const override
	{
		return seat;
	}

	void setInModShop(bool enable)
	{
		inModShop = enable;
	}

	bool isInModShop() const override
	{
		return inModShop;
	}

	void setInDriveByMode(bool enable)
	{
		inDriveByMode = enable;
	}

	bool isInDriveByMode() const override
	{
		return vehicle && seat > 0 && inDriveByMode;
	}

	void setCuffed(bool enable)
	{
		cuffed = enable;
	}

	bool isCuffed() const override
	{
		return vehicle && seat > 0 && cuffed;
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
		vehicle = nullptr;
		seat = SEAT_NONE;
		numStreamed = 0;
	}
};
