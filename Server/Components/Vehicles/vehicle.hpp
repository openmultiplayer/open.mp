#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

struct Vehicle final : public IVehicle, public PoolIDProvider {
    Vector3 pos;
    GTAQuat rot;
    std::array<IPlayer*, MAX_SEATS> passengers;
    int virtualWorld_ = 0;
    VehicleSpawnData spawnData;
    UniqueIDArray<IPlayer, IPlayerPool::Cnt> streamedPlayers_;
    std::array<int, 14> mods;
    float health = 1000.0f;
    uint8_t interior = 0;
    uint32_t doorDamage = 0;
    uint32_t panelDamage = 0;
    uint8_t lightDamage = 0;
    uint8_t tyreDamage = 0;
    uint8_t siren = 0;
    uint8_t paintJob;
    int32_t bodyColour1;
    int32_t bodyColour2;
    IPlayer* driver;
    DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
    String numberPlate;

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

    Vector3 getPosition() const override {
        return pos;
    }

    void setPosition(Vector3 position) override {
        pos = position;
    }

    GTAQuat getRotation() const override {
        return rot;
    }

    void setRotation(GTAQuat rotation) override {
        rot = rotation;
    }

    const std::array<IPlayer*, MAX_SEATS>& getPassengers() override {
        return passengers;
    }

    bool isStreamedInForPlayer(const IPlayer& player) const override {
        return streamedPlayers_.valid(player.getID());
    }

    void setSpawnData(VehicleSpawnData data) override {
        spawnData = data;
        pos = spawnData.position;
        rot = GTAQuat(0.0f, 0.0f, spawnData.zRotation);
    }

    void streamInForPlayer(IPlayer& player) override;
    void streamOutForPlayer(IPlayer& player) override;
    bool updateFromSync(const NetCode::Packet::PlayerVehicleSync& vehicleSync, IPlayer& player) override;

    void setColour(int col1, int col2) override {
        bodyColour1 = col1;
        bodyColour2 = col2;
    }

    void setHealth(float Health) override {
        health = Health;
    }

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
};
