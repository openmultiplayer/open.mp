#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include <chrono>
#include "vehicle_colours.hpp"

struct Vehicle final : public IVehicle, public PoolIDProvider, public NoCopy {
    Vector3 pos;
    GTAQuat rot;
    int virtualWorld_ = 0;
    VehicleSpawnData spawnData;
    UniqueIDArray<IPlayer, IPlayerPool::Cnt> streamedPlayers_;
    std::array<int, MAX_VEHICLE_COMPONENT_SLOT> mods;
    float health = 1000.0f;
    uint8_t interior = 0;
    uint32_t doorDamage = 0;
    uint32_t panelDamage = 0;
    uint8_t lightDamage = 0;
    uint8_t tyreDamage = 0;
    uint8_t siren = 0;
    uint8_t paintJob;
    int32_t bodyColour1 = -1;
    int32_t bodyColour2 = -1;
    IPlayer* driver = nullptr;
    DefaultEventDispatcher<VehicleEventHandler>* eventDispatcher;
    String numberPlate = "XYZSR998";
    uint8_t objective;
    uint8_t doorsLocked;
    bool dead = false;
    std::chrono::milliseconds timeOfDeath;
    bool beenOccupied = false;
    std::chrono::milliseconds lastOccupied;
    bool respawning = false;
    Vector3 velocity;

    Vehicle() {
        mods.fill(0);
    }

    virtual int getVirtualWorld() const override {
        return virtualWorld_;
    }

    virtual void setVirtualWorld(int vw) override {
        virtualWorld_ = vw;
    }

    int getID() const override {
        return poolID;
    }

    Vector3 getPosition() const override;

    void setPosition(Vector3 position) override;

    GTAQuat getRotation() const override {
        return rot;
    }

    void setRotation(GTAQuat rotation) override {
        rot = rotation;
    }

    bool isStreamedInForPlayer(const IPlayer& player) const override {
        return streamedPlayers_.valid(player.getID());
    }

    void setSpawnData(VehicleSpawnData data) override {
        spawnData = data;

        if (spawnData.colour1 == -1 || spawnData.colour2 == -1) {
            int ignore;
            getRandomVehicleColour(spawnData.modelID, spawnData.colour1 == -1 ? spawnData.colour1 : ignore, spawnData.colour2 == -1 ? spawnData.colour2 : ignore);
        }
        pos = spawnData.position;
        rot = GTAQuat(0.0f, 0.0f, spawnData.zRotation);
    }

    void streamInForPlayer(IPlayer& player) override;
    void streamOutForPlayer(IPlayer& player) override;

    /// Update the vehicle's data from a player sync packet.
    bool updateFromSync(const NetCode::Packet::PlayerVehicleSync& vehicleSync, IPlayer& player) override;

    /// Update the vehicle's data from an unoccupied sync packet.
    bool updateFromUnoccupied(const NetCode::Packet::PlayerUnoccupiedSync& unoccupiedSync, IPlayer& player) override;

    /// Sets the vehicle's body colour
    void setColour(int col1, int col2) override;

    /// Sets the vehicle's health
    void setHealth(float Health) override;

    /// Gets the vehicle's current health
    float getHealth() override;

    /// Sets the current driver of the vehicle
    virtual void setDriver(IPlayer* player) override {
        driver = player;
    }

    /// Returns the current driver of the vehicle
    IPlayer* getDriver() override {
        return driver;
    }

    void setPlate(String plate) override;
    const String& getPlate() override;

    void setDamageStatus(int PanelStatus, int DoorStatus, uint8_t LightStatus, uint8_t TyreStatus, IPlayer* vehicleUpdater = nullptr) override;
    void getDamageStatus(int& PanelStatus, int& DoorStatus, uint8_t& LightStatus, uint8_t& TyreStatus) override;

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

    /// Removes the player from the vehicle.
    void removePlayer(IPlayer& player) override;

    /// Set the vehicle's Z angle.
	void setZAngle(float angle) override;

    /// Gets the vehicle's Z angle.
    float getZAngle() override;

	// Set the vehicle's parameters.
    void setParams(int objective, bool doorsLocked) override;

	// Get the vehicle's parameters.
    void getParams(int& objective, bool& doorsLocked) override;

    /// Sets the vehicle's death state.
    void setDead(IPlayer& killer) override;

    /// Checks if the vehicle is dead.
    bool isDead() override;

    /// Respawns the vehicle.
    void respawn() override;

    /// Get the vehicle's respawn delay.
    int getRespawnDelay() override;

    /// Checks if the vehicle has had any occupants.
    bool hasBeenOccupied() override;

    /// Gets the time the vehicle died.
    std::chrono::milliseconds getDeathTime() override;

    /// Gets the last time the vehicle has been occupied
    std::chrono::milliseconds getLastOccupiedTime() override;

    bool isRespawning() override { return respawning; }

    // Sets (links) the vehicle to an interior.
    void setInterior(int InteriorID) override;

	// Gets the vehicle's interior.
    int getInterior() override;

    /// Set if the vehicle has been occupied.
    void setBeenOccupied(bool occupied) override { this->beenOccupied = occupied; }
};
