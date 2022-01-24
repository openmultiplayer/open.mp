#pragma once

#include "vehicle.hpp"
#include <Server/Components/Vehicles/vehicle_components.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

using namespace Impl;

struct VehiclesComponent final : public IVehiclesComponent, public CoreEventHandler, public PlayerEventHandler, public PlayerUpdateEventHandler {
    ICore* core = nullptr;
    MarkedPoolStorage<Vehicle, IVehicle, IVehiclesComponent::Capacity> storage;
    DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
    StaticArray<uint8_t, MAX_VEHICLE_MODELS> preloadModels;
    StreamConfigHelper streamConfigHelper;
    int* deathRespawnDelay = nullptr;

    IEventDispatcher<VehicleEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    struct PlayerEnterVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerEnterVehicleHandler(VehiclesComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerEnterVehicle onPlayerEnterVehicleRPC;
            if (!onPlayerEnterVehicleRPC.read(bs) || !self.valid(onPlayerEnterVehicleRPC.VehicleID)) {
                return false;
            }

            {
                ScopedPoolReleaseLock lock(self, onPlayerEnterVehicleRPC.VehicleID);
                self.eventDispatcher.dispatch(
                    &VehicleEventHandler::onPlayerEnterVehicle,
                    peer,
                    lock.entry,
                    onPlayerEnterVehicleRPC.Passenger);
            }

            NetCode::RPC::EnterVehicle enterVehicleRPC;
            enterVehicleRPC.PlayerID = peer.getID();
            enterVehicleRPC.VehicleID = onPlayerEnterVehicleRPC.VehicleID;
            enterVehicleRPC.Passenger = onPlayerEnterVehicleRPC.Passenger;
            PacketHelper::broadcastToStreamed(enterVehicleRPC, peer, true);
            return true;
        }
    } playerEnterVehicleHandler;

    struct PlayerExitVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerExitVehicleHandler(VehiclesComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerExitVehicle onPlayerExitVehicleRPC;
            if (!onPlayerExitVehicleRPC.read(bs) || !self.valid(onPlayerExitVehicleRPC.VehicleID)) {
                return false;
            }

            {
                ScopedPoolReleaseLock lock(self, onPlayerExitVehicleRPC.VehicleID);
                self.eventDispatcher.dispatch(
                    &VehicleEventHandler::onPlayerExitVehicle,
                    peer,
                    lock.entry);
            }

            NetCode::RPC::ExitVehicle exitVehicleRPC;
            exitVehicleRPC.PlayerID = peer.getID();
            exitVehicleRPC.VehicleID = onPlayerExitVehicleRPC.VehicleID;
            PacketHelper::broadcastToStreamed(exitVehicleRPC, peer, true);
            return true;
        }
    } playerExitVehicleHandler;

    struct PlayerUpdateVehicleDamageStatus : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerUpdateVehicleDamageStatus(VehiclesComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::SetVehicleDamageStatus onDamageStatus;
            if (!onDamageStatus.read(bs) || !self.valid(onDamageStatus.VehicleID)) {
                return false;
            }

            PlayerVehicleData* data = peer.queryData<PlayerVehicleData>();
            Vehicle* vehicle = data->vehicle;
            if (vehicle && vehicle->driver == &peer) {
                vehicle->setDamageStatus(onDamageStatus.PanelStatus, onDamageStatus.DoorStatus, onDamageStatus.LightStatus, onDamageStatus.TyreStatus, &peer);
            }
            return true;
        }
    } vehicleDamageStatusHandler;

    struct PlayerSCMEventHandler : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerSCMEventHandler(VehiclesComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::SCMEvent scmEvent;
            if (!scmEvent.read(bs) || !self.valid(scmEvent.VehicleID)) {
                return false;
            }

            Vehicle& vehicle = self.storage.get(scmEvent.VehicleID);
            if (!vehicle.isStreamedInForPlayer(peer)) {
                return false;
            }

            switch (scmEvent.EventType) {
            case VehicleSCMEvent_SetPaintjob: {
                bool allowed = self.eventDispatcher.stopAtFalse(
                    [&peer, &vehicle, &scmEvent](VehicleEventHandler* handler) {
                        return handler->onVehiclePaintJob(peer, vehicle, scmEvent.Arg1);
                    });
                if (allowed) {
                    vehicle.setPaintJob(scmEvent.Arg1);
                }
                break;
            }

            case VehicleSCMEvent_AddComponent: {
                if (getVehicleComponentSlot(scmEvent.Arg1) == VehicleComponent_None) {
                    break;
                }

                bool allowed = self.eventDispatcher.stopAtFalse(
                    [&peer, &vehicle, &scmEvent](VehicleEventHandler* handler) {
                        return handler->onVehicleMod(peer, vehicle, scmEvent.Arg1);
                    });

                if (allowed) {
                    vehicle.addComponent(scmEvent.Arg1);
                } else {
                    NetCode::RPC::RemoveVehicleComponent modRPC;
                    modRPC.VehicleID = scmEvent.VehicleID;
                    modRPC.Component = scmEvent.Arg1;
                    PacketHelper::send(modRPC, peer);
                }
                break;
            }

            case VehicleSCMEvent_SetColour: {
                bool allowed = self.eventDispatcher.stopAtFalse(
                    [&peer, &vehicle, &scmEvent](VehicleEventHandler* handler) {
                        return handler->onVehicleRespray(peer, vehicle, scmEvent.Arg1, scmEvent.Arg2);
                    });

                if (allowed) {
                    vehicle.setColour(scmEvent.Arg1, scmEvent.Arg2);
                }
                break;
            }

            case VehicleSCMEvent_EnterExitModShop: {
                self.eventDispatcher.all([&peer, scmEvent](VehicleEventHandler* handler) {
                    handler->onEnterExitModShop(peer, scmEvent.Arg1, scmEvent.Arg2);
                });

                NetCode::RPC::SCMEvent enterExitRPC;
                enterExitRPC.PlayerID = peer.getID();
                enterExitRPC.VehicleID = vehicle.poolID;
                enterExitRPC.EventType = VehicleSCMEvent_EnterExitModShop;
                enterExitRPC.Arg1 = scmEvent.Arg1;
                enterExitRPC.Arg2 = scmEvent.Arg2;
                PacketHelper::broadcastToSome(enterExitRPC, vehicle.streamedFor_.entries(), &peer);
                break;
            }
            }
            return true;
        }
    } playerSCMEventHandler;

    struct VehicleDeathHandler final : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        VehicleDeathHandler(VehiclesComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::VehicleDeath vehicleDeath;
            if (!vehicleDeath.read(bs) || !self.valid(vehicleDeath.VehicleID)) {
                return false;
            }

            Vehicle& vehicle = self.storage.get(vehicleDeath.VehicleID);
            if (!vehicle.isStreamedInForPlayer(peer)) {
                return false;
            } else if ((vehicle.isDead() || vehicle.isRespawning()) && vehicle.getDriver() != nullptr && vehicle.getDriver() != &peer) {
                return false;
            }

            vehicle.setDead(peer);
            return true;
        }
    } vehicleDeathHandler;

    void onStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override
    {
        if (oldState == PlayerState_Driver || oldState == PlayerState_Passenger) {
            PlayerVehicleData* data = player.queryData<PlayerVehicleData>();
            if (data->vehicle) {
                data->vehicle->unoccupy(player);
            }
            data->setVehicle(nullptr, -1);
        }
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        PlayerVehicleData* data = player.queryData<PlayerVehicleData>();
        if (data->vehicle) {
            data->vehicle->unoccupy(player);
        }

        const int pid = player.getID();
        for (IVehicle* v : storage) {
            Vehicle* vehicle = static_cast<Vehicle*>(v);
            if (vehicle->streamedFor_.valid(pid)) {
                vehicle->streamedFor_.remove(pid, player);
            }
        }
    }

    VehiclesComponent()
        : playerEnterVehicleHandler(*this)
        , playerExitVehicleHandler(*this)
        , vehicleDamageStatusHandler(*this)
        , playerSCMEventHandler(*this)
        , vehicleDeathHandler(*this)
    {
        preloadModels.fill(1);
    }

    ~VehiclesComponent()
    {
        if (core) {
            core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(this);
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            NetCode::RPC::OnPlayerEnterVehicle::removeEventHandler(*core, &playerEnterVehicleHandler);
            NetCode::RPC::OnPlayerExitVehicle::removeEventHandler(*core, &playerExitVehicleHandler);
            NetCode::RPC::SetVehicleDamageStatus::removeEventHandler(*core, &vehicleDamageStatusHandler);
            NetCode::RPC::SCMEvent::removeEventHandler(*core, &playerSCMEventHandler);
            NetCode::RPC::VehicleDeath::removeEventHandler(*core, &vehicleDeathHandler);
        }
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerEnterVehicle::addEventHandler(*core, &playerEnterVehicleHandler);
        NetCode::RPC::OnPlayerExitVehicle::addEventHandler(*core, &playerExitVehicleHandler);
        NetCode::RPC::SetVehicleDamageStatus::addEventHandler(*core, &vehicleDamageStatusHandler);
        NetCode::RPC::SCMEvent::addEventHandler(*core, &playerSCMEventHandler);
        NetCode::RPC::VehicleDeath::addEventHandler(*core, &vehicleDeathHandler);
        storage.claimUnusable(0);
        streamConfigHelper = StreamConfigHelper(core->getConfig());
        deathRespawnDelay = core->getConfig().getInt("vehicle_death_respawn_delay");
    }

    IPlayerData* onPlayerDataRequest(IPlayer& player) override
    {
        return new PlayerVehicleData();
    }

    StringView componentName() const override
    {
        return "Vehicles";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    StaticArray<uint8_t, MAX_VEHICLE_MODELS>& models() override
    {
        return preloadModels;
    }

    IVehicle* create(int modelID, Vector3 position, float Z, int colour1, int colour2, Seconds respawnDelay, bool addSiren) override
    {
        IVehicle* ret = create(VehicleSpawnData { modelID, position, Z, colour1, colour2, respawnDelay, addSiren });
        if (modelID == 538 || modelID == 537) {
            int carridgeModel = modelID == 538 ? 570 : 569;
            ret->addCarriage(create(VehicleSpawnData { carridgeModel, position, Z, colour1, colour2, respawnDelay }), 0);
            ret->addCarriage(create(VehicleSpawnData { carridgeModel, position, Z, colour1, colour2, respawnDelay }), 1);
            ret->addCarriage(create(VehicleSpawnData { carridgeModel, position, Z, colour1, colour2, respawnDelay }), 2);
        }
        return ret;
    }

    IVehicle* create(const VehicleSpawnData& data) override
    {
        return storage.emplace(this, data);
    }

    void free() override
    {
        delete this;
    }

    bool valid(int index) const override
    {
        if (index == 0) {
            return false;
        }
        return storage.valid(index);
    }

    IVehicle& get(int index) override
    {
        return storage.get(index);
    }

    int findFreeIndex() override
    {
        return storage.findFreeIndex();
    }

    void release(int index) override
    {
        Vehicle& vehicle = storage.get(index);
        if (vehicle.spawnData.modelID == 538 || vehicle.spawnData.modelID == 537) {
            for (IVehicle* c : vehicle.carriages) {
                Vehicle* carriage = static_cast<Vehicle*>(c);
                storage.release(carriage->poolID, false);
            }
        }
        storage.release(index, false);
    }

    void lock(int index) override
    {
        storage.lock(index);
    }

    bool unlock(int index) override
    {
        return storage.unlock(index);
    }

    /// Get a set of all the available objects
    const FlatPtrHashSet<IVehicle>& entries() override
    {
        return storage._entries();
    }

    IEventDispatcher<PoolEventHandler<IVehicle>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }

    void onTick(Microseconds elapsed, TimePoint now) override
    {
        for (IVehicle* v : storage) {
            Vehicle* vehicle = static_cast<Vehicle*>(v);
            const Seconds delay = vehicle->getRespawnDelay();

            if (!vehicle->isOccupied()) {
                if (vehicle->isDead()) {
                    TimePoint lastInteraction = vehicle->timeOfDeath;
                    if (vehicle->beenOccupied) {
                        lastInteraction = std::max(lastInteraction, vehicle->lastOccupiedChange);
                    }
                    if (now - lastInteraction >= Seconds(*deathRespawnDelay)) {
                        vehicle->respawn();
                    }
                } else if (vehicle->beenOccupied && delay != Seconds(-1)) {
                    if (now - vehicle->lastOccupiedChange >= delay) {
                        vehicle->respawn();
                    }
                }
            }
        }
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        if (streamConfigHelper.shouldStream(player.getID(), now)) {
            for (IVehicle* v : storage) {
                Vehicle* vehicle = static_cast<Vehicle*>(v);

                const PlayerState state = player.getState();
                const Vector2 dist2D = vehicle->pos - player.getPosition();
                const bool shouldBeStreamedIn = state != PlayerState_None && player.getVirtualWorld() == vehicle->virtualWorld_ && glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = vehicle->isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    vehicle->streamInForPlayer(player);
                } else if (isStreamedIn && !shouldBeStreamedIn) {
                    vehicle->streamOutForPlayer(player);
                }
            }
        }

        return true;
    }
};
