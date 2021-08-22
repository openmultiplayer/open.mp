#pragma once

#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "vehicle.hpp"
#include "vehicle_components.hpp"

class VehiclesComponent final : public IVehiclesComponent, public PlayerEventHandler, public PlayerUpdateEventHandler {
private:
	ICore* core;
    MarkedPoolStorage<Vehicle, IVehicle, IVehiclesComponent::Capacity> storage;
    DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
    StaticArray<uint8_t, MAX_VEHICLE_MODELS> preloadModels;
    StreamConfigHelper streamConfigHelper;

	friend class Vehicle;

public:
    IEventDispatcher<VehicleEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    struct PlayerEnterVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerEnterVehicleHandler(VehiclesComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerEnterVehicle onPlayerEnterVehicleRPC;
            if (!onPlayerEnterVehicleRPC.read(bs) || !self.storage.valid(onPlayerEnterVehicleRPC.VehicleID)) {
                return false;
            }

            {
                ScopedPoolReleaseLock lock(self, onPlayerEnterVehicleRPC.VehicleID);
                self.eventDispatcher.dispatch(
                    &VehicleEventHandler::onPlayerEnterVehicle,
                    peer,
                    lock.getEntry(),
                    onPlayerEnterVehicleRPC.Passenger
                );
            }

            NetCode::RPC::EnterVehicle enterVehicleRPC;
            enterVehicleRPC.PlayerID = peer.getID();
            enterVehicleRPC.VehicleID = onPlayerEnterVehicleRPC.VehicleID;
            enterVehicleRPC.Passenger = onPlayerEnterVehicleRPC.Passenger;
            peer.broadcastRPCToStreamed(enterVehicleRPC, true);
            return true;
        }
    } playerEnterVehicleHandler;

    struct PlayerExitVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerExitVehicleHandler(VehiclesComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerExitVehicle onPlayerExitVehicleRPC;
            if (!onPlayerExitVehicleRPC.read(bs) || !self.storage.valid(onPlayerExitVehicleRPC.VehicleID)) {
                return false;
            }

            {
                ScopedPoolReleaseLock lock(self, onPlayerExitVehicleRPC.VehicleID);
                self.eventDispatcher.dispatch(
                    &VehicleEventHandler::onPlayerExitVehicle,
                    peer,
                    lock.getEntry()
                );
            }

            NetCode::RPC::ExitVehicle exitVehicleRPC;
            exitVehicleRPC.PlayerID = peer.getID();
            exitVehicleRPC.VehicleID = onPlayerExitVehicleRPC.VehicleID;
            peer.broadcastRPCToStreamed(exitVehicleRPC, true);
            return true;
        }
    } playerExitVehicleHandler;

    struct PlayerUpdateVehicleDamageStatus : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerUpdateVehicleDamageStatus(VehiclesComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::SetVehicleDamageStatus onDamageStatus;
            if (!onDamageStatus.read(bs) || !self.storage.valid(onDamageStatus.VehicleID)) {
                return false;
            }

            IPlayerVehicleData* data = peer.queryData<IPlayerVehicleData>();
            IVehicle* vehicle = data->getVehicle();
            if (vehicle && vehicle->getDriver() == &peer) {
                vehicle->setDamageStatus(onDamageStatus.PanelStatus, onDamageStatus.DoorStatus, onDamageStatus.LightStatus, onDamageStatus.TyreStatus, &peer);
            }
            return true;
        }
    } vehicleDamageStatusHandler;

    struct PlayerSCMEventHandler : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        PlayerSCMEventHandler(VehiclesComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::SCMEvent scmEvent;
            if (!scmEvent.read(bs) || !self.storage.valid(scmEvent.VehicleID)) {
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
                }
                else {
                    NetCode::RPC::RemoveVehicleComponent modRPC;
                    modRPC.VehicleID = scmEvent.VehicleID;
                    modRPC.Component = scmEvent.Arg1;
                    peer.sendRPC(modRPC);
                }
                break;
            }

            case VehicleSCMEvent_SetColour: {
                bool allowed = self.eventDispatcher.stopAtFalse(
                    [&peer, &vehicle, &scmEvent](VehicleEventHandler* handler) {
                        return handler->onVehicleRespray(peer, vehicle, scmEvent.Arg1, scmEvent.Arg2);
                    }
                );

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
                enterExitRPC.VehicleID = vehicle.getID();
                enterExitRPC.EventType = VehicleSCMEvent_EnterExitModShop;
                enterExitRPC.Arg1 = scmEvent.Arg1;
                enterExitRPC.Arg2 = scmEvent.Arg2;

                for (IPlayer* player : vehicle.streamedFor_.entries()) {
                    if (player != &peer) {
                        player->sendRPC(enterExitRPC);
                    }
                }
                break;
            }
            }
            return true;
        }
    } playerSCMEventHandler;

    struct VehicleDeathHandler final : public SingleNetworkInOutEventHandler {
        VehiclesComponent& self;
        VehicleDeathHandler(VehiclesComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::VehicleDeath vehicleDeath;
            if (!vehicleDeath.read(bs) || !self.storage.valid(vehicleDeath.VehicleID)) {
                return false;
            }

            Vehicle& vehicle = self.storage.get(vehicleDeath.VehicleID);
            if (!vehicle.isStreamedInForPlayer(peer)) {
                return false;
            }
            else if ((vehicle.isDead() || vehicle.isRespawning()) && vehicle.getDriver() != nullptr && vehicle.getDriver() != &peer) {
                return false;
            }

            vehicle.setDead(peer);
            return true;
        }
    } vehicleDeathHandler;

    void onStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override {
        if (oldState == PlayerState_Driver || oldState == PlayerState_Passenger) {
            PlayerVehicleData* data = player.queryData<PlayerVehicleData>();
            Vehicle* vehicle = static_cast<Vehicle*>(data->vehicle);
            if (vehicle) {
                vehicle->setBeenOccupied(true);
                if (vehicle->driver == &player) {
                    vehicle->driver = nullptr;
                }
            }

            data->vehicle = nullptr;
            data->seat = -1;
        }
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override {
        const int pid = player.getID();
        for (IVehicle* v : storage.entries()) {
            Vehicle* vehicle = static_cast<Vehicle*>(v);
            if (vehicle->streamedFor_.valid(pid)) {
                vehicle->streamedFor_.remove(pid, player);
                if (vehicle->driver == &player) {
                    vehicle->driver = nullptr;
                }
            }
        }
    }

    VehiclesComponent() :
        playerEnterVehicleHandler(*this),
        playerExitVehicleHandler(*this),
        vehicleDamageStatusHandler(*this),
        playerSCMEventHandler(*this),
        vehicleDeathHandler(*this)
    {
        preloadModels.fill(1);
    }

    ~VehiclesComponent()
    {
        core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(this);
        core->getPlayers().getEventDispatcher().removeEventHandler(this);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::SetVehicleDamageStatus>(&vehicleDamageStatusHandler);
        core->removePerRPCEventHandler<NetCode::RPC::SCMEvent>(&playerSCMEventHandler);
        core->removePerRPCEventHandler<NetCode::RPC::VehicleDeath>(&vehicleDeathHandler);
    }

    void onLoad(ICore* core) override {
        this->core = core;
        core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
        core->addPerRPCEventHandler<NetCode::RPC::SetVehicleDamageStatus>(&vehicleDamageStatusHandler);
        core->addPerRPCEventHandler<NetCode::RPC::SCMEvent>(&playerSCMEventHandler);
        core->addPerRPCEventHandler<NetCode::RPC::VehicleDeath>(&vehicleDeathHandler);
        storage.claimUnusable(0);
        streamConfigHelper = StreamConfigHelper(core->getConfig());
    }

    IPlayerData* onPlayerDataRequest(IPlayer& player) override {
        return new PlayerVehicleData();
    }

    StringView componentName() override {
        return "VehiclesComponent";
    }

    StaticArray<uint8_t, MAX_VEHICLE_MODELS>& models() override {
        return preloadModels;
    }

    IVehicle* create(int modelID, Vector3 position, float Z, int colour1, int colour2, Seconds respawnDelay, bool addSiren) override {
        IVehicle* ret = create(VehicleSpawnData{ modelID, position, Z, colour1, colour2, respawnDelay, addSiren });
        if (modelID == 538 || modelID == 537) {
            int carridgeModel = modelID == 538 ? 570 : 569;
            ret->addCarriage(create(VehicleSpawnData{ carridgeModel, position, Z, colour1, colour2, respawnDelay }), 0);
            ret->addCarriage(create(VehicleSpawnData{ carridgeModel, position, Z, colour1, colour2, respawnDelay }), 1);
            ret->addCarriage(create(VehicleSpawnData{ carridgeModel, position, Z, colour1, colour2, respawnDelay }), 2);
        }
        return ret;
    }

    IVehicle* create(VehicleSpawnData data) override {
        int freeIdx = storage.findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = storage.claim(freeIdx);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        Vehicle& vehicle = storage.get(pid);
        vehicle.pool = this;
        vehicle.setSpawnData(data);
        return &vehicle;
    }

    void free() override {
        delete this;
    }

    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        int res = storage.claim();
        return res;
    }

    int claim(int hint) override {
        int res = storage.claim(hint);
        return res;
    }

    bool valid(int index) const override {
        if (index == 0) {
            return false;
        }
        return storage.valid(index);
    }

    IVehicle& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        Vehicle& vehicle = storage.get(index);
        if (vehicle.spawnData.modelID == 538 || vehicle.spawnData.modelID == 537) {
            auto carriages = storage.get(index).getCarriages();
            for (IVehicle* carriage : carriages) {
                storage.release(carriage->getID(), false);
            }
        }
        storage.release(index, false);
    }

    void lock(int index) override {
        storage.lock(index);
    }

    void unlock(int index) override {
        storage.unlock(index);
    }

    /// Get a set of all the available objects
    const FlatPtrHashSet<IVehicle>& entries() override {
        return storage.entries();
    }

    bool onUpdate(IPlayer& player, TimePoint now) override {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        if (streamConfigHelper.shouldStream(player.getID(), now)) {
            for (IVehicle* v : storage.entries()) {
                Vehicle* vehicle = static_cast<Vehicle*>(v);
                bool occupied = false;
                const PlayerState state = player.getState();
                const Vector2 dist2D = vehicle->pos - player.getPosition();
                const bool shouldBeStreamedIn =
                    state != PlayerState_None &&
                    player.getVirtualWorld() == vehicle->virtualWorld_ &&
                    glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = vehicle->isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    vehicle->streamInForPlayer(player);
                }
                else if (isStreamedIn && !shouldBeStreamedIn) {
                    vehicle->streamOutForPlayer(player);
                }

                if (!occupied && isStreamedIn && shouldBeStreamedIn) {
                    PlayerState state = player.getState();
                    if ((state == PlayerState_Driver || state == PlayerState_Passenger)) {
                        occupied = player.queryData<IPlayerVehicleData>()->getVehicle() == vehicle;
                    }
                }

                if (vehicle->isDead() && vehicle->getRespawnDelay() != Seconds(-1) && !occupied) {
                    if (now - vehicle->timeOfDeath >= vehicle->getRespawnDelay()) {
                        vehicle->respawn();
                    }
                }
                else if (!occupied && vehicle->hasBeenOccupied() && vehicle->getRespawnDelay() != Seconds(-1)) {
                    if (now - vehicle->lastOccupied >= vehicle->getRespawnDelay()) {
                        vehicle->respawn();
                    }
                }
            }
        }

        return true;
    }
};
