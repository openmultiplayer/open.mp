#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "vehicle.hpp"
#include "vehicle_components.hpp"

struct PlayerVehicleData final : public IPlayerVehicleData {
    IVehicle* vehicle = nullptr;
    int seat = -1;

    /// Get the player's vehicle
    /// Returns nullptr if they aren't in a vehicle
    IVehicle* getVehicle() override {
        return vehicle;
    }

    /// Get the player's seat
    /// Returns -1 if they aren't in a vehicle.
    int getSeat() const override {
        return seat;
    }

    void setVehicle(IVehicle* vehicle) override {
        this->vehicle = vehicle;
    }

    void setSeat(int seat) override {
        this->seat = seat;
    }

    void free() override {
        delete this;
    }
};

struct VehiclePlugin final : public IVehiclesPlugin, public CoreEventHandler, public PlayerEventHandler {
	ICore* core;
    MarkedPoolStorage<Vehicle, IVehicle, VehiclePlugin::Cnt> storage;
    DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
	std::array<uint8_t, MAX_VEHICLE_MODELS> preloadModels;

    IEventDispatcher<VehicleEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    struct PlayerEnterVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclePlugin& self;
        PlayerEnterVehicleHandler(VehiclePlugin& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerEnterVehicle onPlayerEnterVehicleRPC;
            if (!onPlayerEnterVehicleRPC.read(bs) || !self.storage.valid(onPlayerEnterVehicleRPC.VehicleID)) {
                return false;
            }

            self.eventDispatcher.dispatch(
                &VehicleEventHandler::onPlayerEnterVehicle,
                peer,
                self.storage.get(onPlayerEnterVehicleRPC.VehicleID),
                onPlayerEnterVehicleRPC.Passenger
            );

            NetCode::RPC::EnterVehicle enterVehicleRPC;
            enterVehicleRPC.PlayerID = peer.getID();
            enterVehicleRPC.VehicleID = onPlayerEnterVehicleRPC.VehicleID;
            enterVehicleRPC.Passenger = onPlayerEnterVehicleRPC.Passenger;
            self.core->getPlayers().broadcastRPC(enterVehicleRPC, BroadcastStreamed, &peer, true);
            return true;
        }
    } playerEnterVehicleHandler;

    struct PlayerExitVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclePlugin& self;
        PlayerExitVehicleHandler(VehiclePlugin& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerExitVehicle onPlayerExitVehicleRPC;
            if (!onPlayerExitVehicleRPC.read(bs) || !self.storage.valid(onPlayerExitVehicleRPC.VehicleID)) {
                return false;
            }

            self.eventDispatcher.dispatch(
                &VehicleEventHandler::onPlayerExitVehicle,
                peer,
                self.storage.get(onPlayerExitVehicleRPC.VehicleID)
            );

            NetCode::RPC::ExitVehicle exitVehicleRPC;
            exitVehicleRPC.PlayerID = peer.getID();
            exitVehicleRPC.VehicleID = onPlayerExitVehicleRPC.VehicleID;
            self.core->getPlayers().broadcastRPC(exitVehicleRPC, BroadcastStreamed, &peer, true);
            return true;
        }
    } playerExitVehicleHandler;

    struct PlayerUpdateVehicleDamageStatus : public SingleNetworkInOutEventHandler {
        VehiclePlugin& self;
        PlayerUpdateVehicleDamageStatus(VehiclePlugin& self) : self(self) {}

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
        VehiclePlugin& self;
        PlayerSCMEventHandler(VehiclePlugin& self) : self(self) {}

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
                            return handler->onPaintJob(peer, vehicle, scmEvent.Arg1);
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
                            return handler->onMod(peer, vehicle, scmEvent.Arg1);
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
                            return handler->onRespray(peer, vehicle, scmEvent.Arg1, scmEvent.Arg2);
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
                    
                    for (IPlayer* player : vehicle.streamedPlayers_.entries()) {
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

    void onStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override {
        if (oldState == PlayerState_Driver || oldState == PlayerState_Passenger) {
            IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
            if (data->getVehicle() && data->getVehicle()->getDriver() == &player) {
                data->getVehicle()->setDriver(nullptr);
            }

            data->setVehicle(nullptr);
            data->setSeat(-1);
        }
    }

    void onDisconnect(IPlayer& player, int reason) override {
        for (IVehicle* vehicle : entries()) {
            vehicle->streamOutForPlayer(player);
        }
    }

    VehiclePlugin() :
        playerEnterVehicleHandler(*this),
        playerExitVehicleHandler(*this),
        vehicleDamageStatusHandler(*this),
        playerSCMEventHandler(*this)
	{
		preloadModels.fill(0);
	}

	~VehiclePlugin()
	{
        core->getEventDispatcher().removeEventHandler(this);
        core->getPlayers().getEventDispatcher().removeEventHandler(this);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::SetVehicleDamageStatus>(&vehicleDamageStatusHandler);
        core->removePerRPCEventHandler<NetCode::RPC::SCMEvent>(&playerSCMEventHandler);
	}

	void onInit(ICore* core) override {
		this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
        core->addPerRPCEventHandler<NetCode::RPC::SetVehicleDamageStatus>(&vehicleDamageStatusHandler);
        core->addPerRPCEventHandler<NetCode::RPC::SCMEvent>(&playerSCMEventHandler);
        storage.claimUnusable(0);
	}

    IPlayerData* onPlayerDataRequest(IPlayer& player) override {
        return new PlayerVehicleData();
    }

	const char* pluginName() override {
		return "VehiclesPlugin";
	}

	std::array<uint8_t, MAX_VEHICLE_MODELS>& models() override {
		return preloadModels;
	}

    IVehicle* create(int modelID, glm::vec3 position, float Z = 0.0f, int colour1 = 0, int colour2 = 0, int respawnDelay = -1) override {
        return create(VehicleSpawnData{ modelID, position, Z, colour1, colour2, respawnDelay });
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
        vehicle.eventDispatcher = &eventDispatcher;
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

    bool valid(int index) override {
        if (index == 0) {
            return false;
        }
        return storage.valid(index);
    }

    IVehicle& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        storage.mark(index);
    }

    /// Get a set of all the available objects
    const DynamicArray<IVehicle*>& entries() const override {
        return storage.entries();
    }

    void onTick(std::chrono::microseconds elapsed) override {
        const float maxDist = STREAM_DISTANCE * STREAM_DISTANCE;
        for (auto it = storage.entries().begin(); it != storage.entries().end();) {
            IVehicle* vehicle = *it;
            const int vw = vehicle->getVirtualWorld();
            const Vector3 pos = vehicle->getPosition();
            for (IPlayer* const& player : core->getPlayers().entries()) {
                const PlayerState state = player->getState();
                const Vector2 dist2D = pos - player->getPosition();
                const bool shouldBeStreamedIn =
                    state != PlayerState_Spectating &&
                    state != PlayerState_None &&
                    player->getVirtualWorld() == vw &&
                    glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = vehicle->isStreamedInForPlayer(*player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    vehicle->streamInForPlayer(*player);
                    eventDispatcher.dispatch(&VehicleEventHandler::onStreamIn, *vehicle, *player);
                }
                else if (isStreamedIn && !shouldBeStreamedIn) {
                    vehicle->streamOutForPlayer(*player);
                    eventDispatcher.dispatch(&VehicleEventHandler::onStreamOut, *vehicle, *player);
                }
            }

            int vid = vehicle->getID();
            it = storage.marked(vid) ? storage.release(vid) : it + 1;
        }
    }
};

PLUGIN_ENTRY_POINT() {
	return new VehiclePlugin();
}
