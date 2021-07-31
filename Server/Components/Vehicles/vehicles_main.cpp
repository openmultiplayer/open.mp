#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "vehicle.hpp"
#include "vehicle_components.hpp"

struct VehiclePlugin final : public IVehiclesPlugin, public CoreEventHandler, public PlayerEventHandler {
	ICore* core;
    MarkedPoolStorage<Vehicle, IVehicle, VehiclePlugin::Cnt> storage;
    DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
    StaticArray<uint8_t, MAX_VEHICLE_MODELS> preloadModels;
    StreamConfigHelper streamConfigHelper;

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

            {
                ScopedPoolReleaseLock lock(self, onPlayerEnterVehicleRPC.VehicleID);
                self.eventDispatcher.dispatch(
                    &VehicleEventHandler::onPlayerEnterVehicle,
                    peer,
                    lock.entry,
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
        VehiclePlugin& self;
        PlayerExitVehicleHandler(VehiclePlugin& self) : self(self) {}

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
                    lock.entry
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
        VehiclePlugin& self;
        VehicleDeathHandler(VehiclePlugin& self) : self(self) {}

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

    VehiclePlugin() :
        playerEnterVehicleHandler(*this),
        playerExitVehicleHandler(*this),
        vehicleDamageStatusHandler(*this),
        playerSCMEventHandler(*this),
        vehicleDeathHandler(*this)
	{
		preloadModels.fill(1);
	}

	~VehiclePlugin()
	{
        core->getEventDispatcher().removeEventHandler(this);
        core->getPlayers().getEventDispatcher().removeEventHandler(this);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::SetVehicleDamageStatus>(&vehicleDamageStatusHandler);
        core->removePerRPCEventHandler<NetCode::RPC::SCMEvent>(&playerSCMEventHandler);
        core->removePerRPCEventHandler<NetCode::RPC::VehicleDeath>(&vehicleDeathHandler);
	}

	void onLoad(ICore* core) override {
		this->core = core;
        core->getEventDispatcher().addEventHandler(this);
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

	const char* pluginName() override {
		return "VehiclesPlugin";
	}

    StaticArray<uint8_t, MAX_VEHICLE_MODELS>& models() override {
		return preloadModels;
	}

    IVehicle* create(int modelID, glm::vec3 position, float Z, int colour1, int colour2, std::chrono::seconds respawnDelay, bool addSiren) override {
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

    void onTick(std::chrono::microseconds elapsed) override {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        const auto time = std::chrono::steady_clock::now();
        if (streamConfigHelper.shouldStream(time)) {
            for (IVehicle* v : storage.entries()) {
                Vehicle* vehicle = static_cast<Vehicle*>(v);
                bool occupied = false;
                for (IPlayer* player : core->getPlayers().entries()) {
                    const PlayerState state = player->getState();
                    const Vector2 dist2D = vehicle->pos - player->getPosition();
                    const bool shouldBeStreamedIn =
                        state != PlayerState_Spectating &&
                        state != PlayerState_None &&
                        player->getVirtualWorld() == vehicle->virtualWorld_ &&
                        glm::dot(dist2D, dist2D) < maxDist;

                    const bool isStreamedIn = vehicle->isStreamedInForPlayer(*player);
                    if (!isStreamedIn && shouldBeStreamedIn) {
                        vehicle->streamInForPlayer(*player);
                    }
                    else if (isStreamedIn && !shouldBeStreamedIn) {
                        vehicle->streamOutForPlayer(*player);
                    }

                    if (!occupied && isStreamedIn && shouldBeStreamedIn) {
                        PlayerState state = player->getState();
                        if ((state == PlayerState_Driver || state == PlayerState_Passenger)) {
                            occupied = player->queryData<IPlayerVehicleData>()->getVehicle() == vehicle;
                        }
                    }
                }

                if (vehicle->isDead() && vehicle->getRespawnDelay() != std::chrono::seconds(-1) && !occupied) {
                    if (time - vehicle->timeOfDeath >= std::chrono::seconds(vehicle->getRespawnDelay())) {
                        vehicle->respawn();
                    }
                }
                else if (!occupied && vehicle->hasBeenOccupied() && vehicle->getRespawnDelay() != std::chrono::seconds(-1)) {
                    if (time - vehicle->lastOccupied >= std::chrono::seconds(vehicle->getRespawnDelay())) {
                        vehicle->respawn();
                    }
                }
            }
        }
    }

    const std::array<VehicleModelInfo, MAX_VEHICLE_MODELS> allModelInfo = {{
        {{2.329999f, 5.119999f, 1.639999f}, {0.330000f, 0.029999f, -0.200000f}, {0.439999f, -0.699999f, -0.219999f}, {-1.100000f, -2.059999f, -0.070000f}, {0.879999f, 1.379999f, -0.610000f}, {0.879999f, -1.389999f, -0.610000f}, {0.000000f, 0.000000f, 0.000000f}, -0.490000f, -0.460000f},
        {{2.569999f, 5.829999f, 1.710000f}, {0.500000f, -0.059999f, -0.119999f}, {0.000000f, 0.000000f, 0.000000f}, {1.090000f, -0.939999f, 0.000000f}, {0.980000f, 1.629999f, -0.349999f}, {0.980000f, -1.490000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.109999f},
        {{2.410000f, 5.800000f, 1.529999f}, {0.540000f, -0.349999f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, {1.039999f, -1.919999f, 0.140000f}, {0.970000f, 1.519999f, -0.349999f}, {0.970000f, -1.549999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.479999f, -0.310000f},
        {{3.150000f, 9.220000f, 4.179999f}, {0.430000f, 1.450000f, 0.340000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.450000f, 0.070000f, -0.800000f}, {1.120000f, 3.559999f, -0.949999f}, {1.120000f, -3.450000f, -0.949999f}, {1.120000f, -2.250000f, -0.949999f}, -0.699999f, -50.000000f},
        {{2.200000f, 5.809999f, 1.840000f}, {0.460000f, -0.129999f, -0.029999f}, {0.469999f, -1.070000f, -0.009999f}, {-0.939999f, -2.359999f, -0.009999f}, {0.800000f, 1.539999f, -0.330000f}, {0.800000f, -1.570000f, -0.330000f}, {0.000000f, 0.000000f, 0.000000f}, -0.140000f, -0.150000f},
        {{2.349999f, 6.000000f, 1.490000f}, {0.430000f, 0.000000f, -0.189999f}, {0.400000f, -0.980000f, -0.239999f}, {-1.039999f, -2.180000f, -0.039999f}, {0.839999f, 1.639999f, -0.449999f}, {0.839999f, -1.639999f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, -0.200000f, -0.200000f},
        {{5.269999f, 11.590000f, 4.420000f}, {1.379999f, 2.559999f, 0.779999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {2.009999f, 3.200000f, -1.250000f}, {2.009999f, -1.970000f, -1.250000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.849999f, 8.960000f, 2.700000f}, {0.600000f, 2.890000f, 0.200000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.110000f, -3.660000f, -0.540000f}, {1.049999f, 2.450000f, -0.699999f}, {1.049999f, -2.049999f, -0.699999f}, {0.000000f, 0.000000f, 0.000000f}, -0.500000f, -50.000000f},
        {{3.119999f, 10.689999f, 3.910000f}, {0.439999f, 2.730000f, -0.109999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.230000f, 1.299999f, -0.660000f}, {1.090000f, 4.039999f, -0.970000f}, {1.090000f, -2.220000f, -0.970000f}, {1.090000f, -1.080000f, -0.970000f}, -0.529999f, -50.000000f},
        {{2.369999f, 8.180000f, 1.529999f}, {0.460000f, 1.059999f, -0.140000f}, {0.460000f, -1.870000f, -0.219999f}, {-0.980000f, -2.829999f, 0.119999f}, {0.889999f, 2.750000f, -0.349999f}, {0.889999f, -2.740000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.310000f, -0.310000f},
        {{2.259999f, 5.010000f, 1.799999f}, {0.419999f, -0.170000f, -0.050000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.019999f, -1.669999f, 0.209999f}, {0.819999f, 1.389999f, -0.289999f}, {0.819999f, -1.379999f, -0.289999f}, {0.000000f, 0.000000f, 0.000000f}, -0.039999f, -0.009999f},
        {{2.390000f, 5.780000f, 1.370000f}, {0.490000f, 0.050000f, -0.209999f}, {0.000000f, 0.000000f, 0.000000f}, {1.090000f, -2.099999f, 0.090000f}, {0.949999f, 1.710000f, -0.349999f}, {0.949999f, -1.710000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.509999f, -0.370000f},
        {{2.450000f, 7.309999f, 1.389999f}, {0.460000f, -0.129999f, -0.230000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, -3.549999f, -0.170000f}, {0.930000f, 1.889999f, -0.349999f}, {0.930000f, -1.879999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.360000f, -0.349999f},
        {{2.279999f, 5.880000f, 2.230000f}, {0.490000f, 0.779999f, 0.059999f}, {0.529999f, -1.740000f, -0.019999f}, {-1.049999f, 0.340000f, -0.529999f}, {0.879999f, 1.799999f, -0.639999f}, {0.920000f, -1.850000f, -0.639999f}, {0.000000f, 0.000000f, 0.000000f}, -0.319999f, -0.319999f},
        {{2.519999f, 7.070000f, 4.599999f}, {0.439999f, 0.959999f, 0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {-0.920000f, -0.740000f, -0.699999f}, {0.910000f, 2.140000f, -0.620000f}, {0.910000f, -2.130000f, -0.620000f}, {0.000000f, 0.000000f, 0.000000f}, -0.400000f, -0.280000f},
        {{2.319999f, 5.510000f, 1.139999f}, {0.419999f, 0.000000f, -0.330000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.129999f, -2.069999f, 0.019999f}, {0.910000f, 1.570000f, -0.360000f}, {0.910000f, -1.610000f, -0.360000f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.070000f},
        {{2.730000f, 8.010000f, 3.400000f}, {0.419999f, 0.949999f, 0.059999f}, {0.850000f, -2.730000f, 0.079999f}, {-1.350000f, -2.740000f, -0.189999f}, {0.959999f, 2.220000f, -0.670000f}, {1.179999f, -2.180000f, -0.670000f}, {0.000000f, 0.000000f, 0.000000f}, -0.340000f, -50.000000f},
        {{5.449999f, 23.270000f, 6.610000f}, {0.439999f, 3.269999f, 0.600000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.569999f, 5.670000f, 2.140000f}, {0.519999f, 0.460000f, -0.129999f}, {0.449999f, -0.680000f, -0.239999f}, {1.210000f, -1.570000f, -0.079999f}, {1.009999f, 1.799999f, -0.680000f}, {1.009999f, -1.700000f, -0.680000f}, {0.000000f, 0.000000f, 0.000000f}, -0.490000f, -0.490000f},
        {{2.400000f, 6.219999f, 1.409999f}, {0.409999f, -0.209999f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.080000f, -1.990000f, 0.029999f}, {0.899999f, 1.620000f, -0.430000f}, {0.899999f, -1.620000f, -0.430000f}, {0.000000f, 0.000000f, 0.000000f}, -0.419999f, -0.360000f},
        {{2.410000f, 5.909999f, 1.769999f}, {0.519999f, 0.059999f, -0.140000f}, {0.519999f, -1.029999f, -0.109999f}, {-1.100000f, -2.150000f, 0.100000f}, {0.920000f, 1.639999f, -0.349999f}, {0.920000f, -1.639999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.319999f},
        {{2.250000f, 6.389999f, 1.370000f}, {0.449999f, 0.109999f, -0.230000f}, {0.430000f, -1.080000f, -0.230000f}, {-1.070000f, -2.440000f, -0.170000f}, {0.839999f, 1.740000f, -0.479999f}, {0.839999f, -1.740000f, -0.479999f}, {0.000000f, 0.000000f, 0.000000f}, -0.330000f, -0.289999f},
        {{2.269999f, 5.380000f, 1.549999f}, {0.409999f, 0.070000f, -0.180000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.080000f, -0.419999f, -0.200000f}, {0.860000f, 1.490000f, -0.550000f}, {0.860000f, -1.490000f, -0.550000f}, {0.000000f, 0.000000f, 0.000000f}, -0.310000f, -0.360000f},
        {{2.319999f, 4.840000f, 4.909999f}, {0.579999f, 0.670000f, 0.200000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.169999f, -1.899999f, -0.319999f}, {0.910000f, 1.710000f, -0.629999f}, {0.889999f, -1.299999f, -0.629999f}, {0.000000f, 0.000000f, 0.000000f}, -0.560000f, -0.620000f},
        {{2.460000f, 3.859999f, 1.779999f}, {0.389999f, -0.090000f, -0.090000f}, {0.000000f, 0.000000f, 0.000000f}, {0.180000f, 1.080000f, 0.430000f}, {1.000000f, 1.090000f, -0.280000f}, {1.000000f, -1.080000f, -0.280000f}, {0.000000f, 0.000000f, 0.000000f}, -0.360000f, 0.009999f},
        {{5.159999f, 18.629999f, 5.190000f}, {0.000000f, 2.549999f, 0.189999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.410000f, 5.909999f, 1.769999f}, {0.519999f, 0.059999f, -0.140000f}, {0.519999f, -1.029999f, -0.109999f}, {-1.100000f, -2.150000f, 0.090000f}, {0.920000f, 1.639999f, -0.349999f}, {0.920000f, -1.639999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.319999f},
        {{2.640000f, 8.199999f, 3.230000f}, {0.379999f, 1.100000f, 0.250000f}, {0.709999f, -2.759999f, 0.140000f}, {-1.269999f, -3.200000f, -0.159999f}, {0.910000f, 2.519999f, -0.529999f}, {1.009999f, -2.289999f, -0.529999f}, {0.000000f, 0.000000f, 0.000000f}, -0.409999f, -50.000000f},
        {{2.730000f, 6.280000f, 3.480000f}, {0.649999f, 0.649999f, 0.140000f}, {0.750000f, -2.420000f, 0.209999f}, {-1.009999f, -3.059999f, -0.490000f}, {0.910000f, 1.950000f, -0.639999f}, {1.029999f, -1.750000f, -0.639999f}, {0.000000f, 0.000000f, 0.000000f}, -0.519999f, -50.000000f},
        {{2.210000f, 5.179999f, 1.279999f}, {0.490000f, -0.500000f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, {0.990000f, -2.140000f, 0.140000f}, {0.870000f, 1.570000f, -0.250000f}, {0.870000f, -1.570000f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.259999f},
        {{4.769999f, 16.899999f, 5.920000f}, {0.400000f, 0.519999f, 0.980000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.009999f, 12.210000f, 4.420000f}, {0.709999f, 4.730000f, 0.379999f}, {0.750000f, 5.139999f, -0.529999f}, {-1.450000f, -5.469999f, -0.129999f}, {1.110000f, 3.940000f, -0.500000f}, {1.110000f, -3.900000f, -0.500000f}, {1.110000f, -2.859999f, -0.500000f}, -0.389999f, -0.389999f},
        {{4.300000f, 9.170000f, 3.880000f}, {0.379999f, 2.769999f, -0.189999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {1.629999f, 3.099999f, -0.349999f}, {1.629999f, -3.099999f, -0.349999f}, {1.629999f, 0.000000f, -0.349999f}, -50.000000f, -50.000000f},
        {{3.400000f, 10.000000f, 4.860000f}, {0.490000f, 1.159999f, 0.519999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.519999f, 0.159999f, -0.680000f}, {1.289999f, 2.990000f, -0.800000f}, {1.289999f, -2.990000f, -0.800000f}, {1.289999f, -1.559999f, -0.800000f}, -0.360000f, -0.050000f},
        {{2.289999f, 4.570000f, 1.720000f}, {0.259999f, -0.449999f, -0.330000f}, {0.000000f, 0.000000f, 0.000000f}, {-0.730000f, -1.580000f, 0.289999f}, {0.899999f, 1.799999f, -0.589999f}, {1.019999f, -1.389999f, -0.500000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.160000f, 13.529999f, 4.769999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {1.009999f, -1.440000f, -0.949999f}, {1.009999f, -2.730000f, -0.949999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.279999f, 5.510000f, 1.720000f}, {0.400000f, -0.170000f, -0.079999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.070000f, -1.820000f, 0.140000f}, {0.850000f, 1.379999f, -0.349999f}, {0.850000f, -1.399999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.070000f, -0.070000f},
        {{3.039999f, 11.760000f, 4.010000f}, {0.209999f, 4.289999f, 1.000000f}, {0.750000f, 2.710000f, -0.529999f}, {-1.490000f, -4.969999f, -0.500000f}, {1.220000f, 3.950000f, -0.500000f}, {1.220000f, -3.940000f, -0.500000f}, {1.220000f, -2.829999f, -0.500000f}, -0.379999f, -0.439999f},
        {{2.410000f, 5.820000f, 1.720000f}, {0.460000f, 0.419999f, -0.289999f}, {0.460000f, -0.610000f, -0.289999f}, {-1.090000f, -1.929999f, -0.019999f}, {0.949999f, 1.919999f, -0.550000f}, {0.949999f, -1.909999f, -0.550000f}, {0.000000f, 0.000000f, 0.000000f}, -0.579999f, -0.579999f},
        {{2.220000f, 5.289999f, 1.470000f}, {0.490000f, -0.540000f, -0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.110000f, -1.509999f, 0.059999f}, {0.879999f, 1.559999f, -0.449999f}, {0.879999f, -1.509999f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, -0.349999f, -0.129999f},
        {{2.309999f, 5.559999f, 2.759999f}, {0.500000f, 0.750000f, -0.050000f}, {0.540000f, -1.789999f, -0.090000f}, {-1.090000f, -0.460000f, -0.419999f}, {0.879999f, 1.779999f, -0.750000f}, {0.879999f, -1.850000f, -0.750000f}, {0.000000f, 0.000000f, 0.000000f}, -0.589999f, -0.629999f},
        {{0.870000f, 1.409999f, 1.019999f}, {0.059999f, -0.070000f, 0.159999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.259999f, 0.400000f, 0.029999f}, {0.259999f, -0.400000f, 0.029999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.609999f, 6.679999f, 1.759999f}, {0.490000f, 0.270000f, -0.189999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.210000f, -2.210000f, 0.059999f}, {0.980000f, 2.000000f, -0.460000f}, {0.980000f, -2.000000f, -0.460000f}, {0.000000f, 0.000000f, 0.000000f}, -0.330000f, -0.200000f},
        {{4.159999f, 20.049999f, 4.429999f}, {0.389999f, 2.880000f, 0.319999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.529999f, 1.240000f, -0.899999f}, {1.220000f, 4.860000f, -0.959999f}, {1.220000f, -5.170000f, -0.959999f}, {1.220000f, -4.039999f, -0.959999f}, -0.740000f, -50.000000f},
        {{3.660000f, 6.019999f, 3.289999f}, {0.540000f, 0.349999f, 0.600000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, -2.599999f, -0.059999f}, {1.370000f, 1.909999f, -0.449999f}, {1.370000f, -1.909999f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.299999f, 5.860000f, 1.750000f}, {0.479999f, -0.029999f, -0.159999f}, {0.479999f, -1.139999f, -0.159999f}, {-1.039999f, -1.940000f, 0.129999f}, {0.910000f, 1.740000f, -0.460000f}, {0.910000f, -1.740000f, -0.460000f}, {0.000000f, 0.000000f, 0.000000f}, -0.370000f, -0.340000f},
        {{4.769999f, 17.020000f, 4.309999f}, {0.689999f, 0.250000f, 1.070000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.420000f, 14.800000f, 3.150000f}, {0.419999f, 0.949999f, -0.070000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.709999f, 2.190000f, 1.629999f}, {0.000000f, -0.340000f, 0.419999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.680000f, -0.270000f}, {0.000000f, -0.670000f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.029999f, 9.029999f, 4.989999f}, {0.460000f, 3.359999f, -0.300000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.069999f, 13.510000f, 3.720000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {1.009999f, -1.440000f, -0.949999f}, {1.009999f, -2.730000f, -0.949999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.319999f, 5.469999f, 1.220000f}, {0.529999f, -0.200000f, -0.319999f}, {0.000000f, 0.000000f, 0.000000f}, {1.059999f, -1.220000f, -0.039999f}, {0.899999f, 1.299999f, -0.209999f}, {0.879999f, -1.700000f, -0.219999f}, {0.000000f, 0.000000f, 0.000000f}, -0.129999f, -50.000000f},
        {{3.609999f, 14.569999f, 3.289999f}, {0.500000f, 2.980000f, 0.819999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{5.139999f, 13.770000f, 9.289999f}, {0.409999f, 1.570000f, 1.429999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{6.619999f, 19.049999f, 13.840000f}, {-0.560000f, 2.730000f, 3.109999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.319999f, 9.699999f, 3.630000f}, {0.490000f, 1.169999f, 0.600000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.529999f, -0.009999f, -0.759999f}, {1.289999f, 2.990000f, -0.800000f}, {1.289999f, -2.990000f, -0.800000f}, {1.289999f, -1.559999f, -0.800000f}, -0.560000f, -0.250000f},
        {{3.230000f, 9.520000f, 4.980000f}, {0.419999f, 1.389999f, 0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {0.790000f, -0.050000f, -0.610000f}, {1.019999f, 2.559999f, -0.579999f}, {1.129999f, -2.769999f, -0.579999f}, {0.000000f, 0.000000f, 0.000000f}, -0.239999f, -0.509999f},
        {{1.830000f, 2.609999f, 2.720000f}, {0.340000f, -0.270000f, 0.200000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.550000f, 0.970000f, -0.349999f}, {0.550000f, -0.980000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -0.280000f},
        {{2.410000f, 6.139999f, 1.470000f}, {0.449999f, 0.140000f, -0.230000f}, {0.430000f, -1.019999f, -0.219999f}, {-1.110000f, -2.049999f, -0.109999f}, {0.939999f, 1.700000f, -0.439999f}, {0.939999f, -1.720000f, -0.439999f}, {0.000000f, 0.000000f, 0.000000f}, -0.239999f, -0.239999f},
        {{2.299999f, 5.719999f, 2.230000f}, {0.490000f, 0.779999f, 0.059999f}, {0.670000f, -1.740000f, 0.000000f}, {-1.049999f, -2.289999f, 0.219999f}, {0.879999f, 1.799999f, -0.639999f}, {0.920000f, -1.850000f, -0.639999f}, {0.000000f, 0.000000f, 0.000000f}, -0.319999f, -0.319999f},
        {{10.850000f, 13.550000f, 4.440000f}, {0.319999f, 0.280000f, -0.039999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.699999f, 2.460000f, 1.679999f}, {0.000000f, -0.400000f, 0.469999f}, {0.000000f, -0.850000f, 0.600000f}, {0.000000f, 0.140000f, 0.540000f}, {0.000000f, 0.709999f, -0.180000f}, {0.000000f, -0.889999f, -0.180000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.709999f, 2.190000f, 1.629999f}, {0.000000f, -0.340000f, 0.419999f}, {0.000000f, -0.620000f, 0.379999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.680000f, -0.270000f}, {0.000000f, -0.670000f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.699999f, 2.420000f, 1.340000f}, {0.000000f, -0.409999f, 0.150000f}, {0.000000f, -0.839999f, 0.340000f}, {0.000000f, 0.170000f, 0.469999f}, {0.000000f, 0.920000f, -0.180000f}, {0.000000f, -0.920000f, -0.180000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{1.580000f, 1.549999f, 1.149999f}, {0.000000f, 0.000000f, -0.360000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.400000f, 0.219999f, -0.280000f}, {0.100000f, -0.750000f, -0.059999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.870000f, 1.409999f, 1.019999f}, {0.000000f, 0.000000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.300000f, 0.500000f, -0.250000f}, {0.300000f, -0.349999f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.529999f, 6.170000f, 1.649999f}, {0.500000f, 0.150000f, -0.140000f}, {0.460000f, -0.769999f, -0.140000f}, {0.000000f, -3.000000f, -0.129999f}, {0.920000f, 1.789999f, -0.349999f}, {0.920000f, -1.779999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.289999f, -0.310000f},
        {{2.529999f, 6.360000f, 1.669999f}, {0.460000f, 0.109999f, -0.140000f}, {0.460000f, -0.870000f, -0.159999f}, {-1.070000f, -2.299999f, 0.119999f}, {0.899999f, 1.799999f, -0.349999f}, {0.899999f, -1.779999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.319999f, -0.319999f},
        {{0.709999f, 2.230000f, 1.419999f}, {0.000000f, -0.200000f, 0.400000f}, {0.000000f, -0.529999f, 0.419999f}, {0.000000f, 0.009999f, 0.409999f}, {0.000000f, 0.819999f, -0.259999f}, {0.000000f, -0.750000f, -0.289999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.420000f, 14.800000f, 3.150000f}, {0.419999f, 0.949999f, -0.070000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.660000f, 5.480000f, 2.099999f}, {0.529999f, -0.039999f, 0.090000f}, {0.479999f, -0.959999f, 0.090000f}, {-1.259999f, -2.380000f, 0.239999f}, {1.029999f, 1.649999f, -0.310000f}, {1.029999f, -1.889999f, -0.310000f}, {0.000000f, 0.000000f, 0.000000f}, -0.289999f, -0.100000f},
        {{1.419999f, 2.000000f, 1.710000f}, {0.000000f, -0.349999f, 0.389999f}, {0.000000f, -0.670000f, 0.389999f}, {0.000000f, 0.000000f, 0.000000f}, {0.409999f, 0.610000f, -0.129999f}, {0.409999f, -0.610000f, -0.129999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.670000f, 9.340000f, 4.869999f}, {-0.200000f, -0.159999f, 1.070000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.900000f, 5.400000f, 2.220000f}, {0.000000f, 0.579999f, 0.889999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.430000f, 6.030000f, 1.690000f}, {0.490000f, -0.300000f, -0.140000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.949999f, 1.809999f, -0.349999f}, {0.949999f, -1.809999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.469999f, -0.400000f},
        {{2.450000f, 5.780000f, 1.480000f}, {0.419999f, -0.129999f, -0.200000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.070000f, -1.600000f, 0.129999f}, {0.920000f, 1.539999f, -0.349999f}, {0.930000f, -1.549999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.300000f, -0.239999f},
        {{11.029999f, 11.289999f, 3.289999f}, {0.000000f, -0.449999f, -0.319999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.670000f, 5.929999f, 1.399999f}, {0.550000f, -0.250000f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.200000f, -1.529999f, 0.239999f}, {1.019999f, 1.700000f, -0.319999f}, {1.019999f, -1.620000f, -0.319999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -0.079999f},
        {{2.450000f, 5.570000f, 1.740000f}, {0.439999f, 0.090000f, -0.059999f}, {0.000000f, 0.000000f, 0.000000f}, {1.090000f, -0.340000f, 0.230000f}, {0.870000f, 1.490000f, -0.550000f}, {0.870000f, -1.620000f, -0.550000f}, {0.000000f, 0.000000f, 0.000000f}, -0.379999f, -0.379999f},
        {{2.250000f, 6.150000f, 1.990000f}, {0.490000f, -0.070000f, 0.000000f}, {0.469999f, -1.080000f, 0.039999f}, {-1.090000f, -1.970000f, 0.029999f}, {0.910000f, 1.700000f, -0.379999f}, {0.910000f, -1.590000f, -0.379999f}, {0.000000f, 0.000000f, 0.000000f}, -0.270000f, -0.270000f},
        {{2.269999f, 5.260000f, 1.419999f}, {0.409999f, -0.280000f, -0.150000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.000000f, -0.879999f, 0.100000f}, {0.910000f, 1.330000f, -0.349999f}, {0.910000f, -1.330000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.330000f},
        {{0.709999f, 1.879999f, 1.320000f}, {-0.009999f, -0.340000f, 0.430000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.550000f, -0.239999f}, {0.000000f, -0.589999f, -0.239999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.339999f, 5.699999f, 1.870000f}, {0.469999f, 0.529999f, -0.129999f}, {0.529999f, -1.570000f, -0.150000f}, {1.070000f, -2.190000f, 0.000000f}, {0.879999f, 1.779999f, -0.649999f}, {0.879999f, -1.789999f, -0.649999f}, {0.000000f, 0.000000f, 0.000000f}, -0.509999f, -0.600000f},
        {{2.049999f, 6.190000f, 2.109999f}, {0.379999f, 1.649999f, -0.180000f}, {0.300000f, 0.059999f, -0.209999f}, {0.930000f, -2.509999f, -0.039999f}, {0.790000f, 1.700000f, -0.670000f}, {0.779999f, -2.059999f, -0.670000f}, {0.000000f, 0.000000f, 0.000000f}, -0.740000f, -0.720000f},
        {{5.349999f, 26.200000f, 7.159999f}, {0.000000f, -5.010000f, 1.740000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{1.970000f, 4.070000f, 1.440000f}, {0.150000f, -0.449999f, 0.370000f}, {0.000000f, 0.000000f, 0.000000f}, {-0.889999f, 0.720000f, 0.019999f}, {0.629999f, 1.320000f, -0.300000f}, {0.629999f, -0.990000f, -0.300000f}, {0.000000f, 0.000000f, 0.000000f}, -0.250000f, -0.250000f},
        {{4.340000f, 7.840000f, 4.449999f}, {0.000000f, -0.389999f, 1.129999f}, {0.000000f, 0.000000f, 0.000000f}, {-0.699999f, -3.170000f, 0.579999f}, {1.240000f, 1.679999f, -0.349999f}, {1.240000f, -1.679999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.329999f, 15.039999f, 4.670000f}, {0.460000f, 1.620000f, -0.090000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.329999f, 12.609999f, 4.650000f}, {0.449999f, 0.730000f, -0.090000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.539999f, 5.699999f, 2.140000f}, {0.500000f, -0.090000f, 0.059999f}, {0.000000f, 0.000000f, 0.000000f}, {1.159999f, -0.740000f, 0.000000f}, {1.029999f, 1.720000f, -0.660000f}, {1.029999f, -1.470000f, -0.660000f}, {0.000000f, 0.000000f, 0.000000f}, -0.379999f, -0.340000f},
        {{2.920000f, 6.929999f, 2.140000f}, {0.529999f, 0.460000f, 0.059999f}, {0.529999f, -0.620000f, 0.059999f}, {1.370000f, -1.250000f, 0.000000f}, {1.029999f, 2.289999f, -0.660000f}, {1.029999f, -1.929999f, -0.660000f}, {0.000000f, 0.000000f, 0.000000f}, -0.379999f, -0.340000f},
        {{2.309999f, 6.329999f, 1.289999f}, {0.430000f, -0.280000f, -0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.070000f, -2.180000f, 0.000000f}, {0.829999f, 1.570000f, -0.419999f}, {0.829999f, -1.570000f, -0.419999f}, {0.000000f, 0.000000f, 0.000000f}, -0.230000f, -0.239999f},
        {{2.349999f, 6.179999f, 1.789999f}, {0.460000f, 0.140000f, -0.100000f}, {0.460000f, -0.910000f, -0.100000f}, {-0.980000f, -2.240000f, 0.119999f}, {0.930000f, 1.710000f, -0.349999f}, {0.920000f, -1.700000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.300000f, -0.289999f},
        {{4.769999f, 17.829999f, 3.849999f}, {-0.670000f, 0.600000f, 0.899999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.250000f, 6.489999f, 1.500000f}, {0.479999f, -0.289999f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.019999f, -2.200000f, 0.119999f}, {0.829999f, 1.450000f, -0.400000f}, {0.829999f, -1.779999f, -0.400000f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.289999f},
        {{2.779999f, 5.449999f, 1.990000f}, {0.490000f, 0.000000f, -0.079999f}, {0.000000f, 0.000000f, 0.000000f}, {1.210000f, -1.909999f, -0.100000f}, {1.139999f, 1.679999f, -0.759999f}, {1.139999f, -1.309999f, -0.759999f}, {0.000000f, 0.000000f, 0.000000f}, -0.430000f, -0.430000f},
        {{2.279999f, 4.750000f, 1.789999f}, {0.479999f, -0.250000f, -0.119999f}, {0.000000f, 0.000000f, 0.000000f}, {1.059999f, -1.860000f, 0.150000f}, {0.899999f, 1.440000f, -0.319999f}, {0.899999f, -1.389999f, -0.319999f}, {0.000000f, 0.000000f, 0.000000f}, -0.050000f, -0.059999f},
        {{2.329999f, 15.039999f, 4.659999f}, {0.460000f, 1.620000f, -0.109999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.900000f, 6.599999f, 4.280000f}, {0.730000f, 1.379999f, 0.300000f}, {0.600000f, -2.460000f, 0.039999f}, {-1.299999f, -0.009999f, 0.090000f}, {1.090000f, 2.410000f, -0.579999f}, {0.990000f, -1.740000f, -0.579999f}, {0.000000f, 0.000000f, 0.000000f}, -0.469999f, -0.560000f},
        {{2.640000f, 7.199999f, 3.750000f}, {0.449999f, 0.209999f, -0.009999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.110000f, -1.070000f, -0.349999f}, {0.860000f, 1.700000f, -0.550000f}, {0.860000f, -2.130000f, -0.550000f}, {0.000000f, 0.000000f, 0.000000f}, -0.330000f, -0.259999f},
        {{2.289999f, 5.010000f, 1.850000f}, {0.449999f, -0.239999f, -0.059999f}, {0.000000f, 0.000000f, 0.000000f}, {-0.980000f, -1.710000f, -0.070000f}, {0.870000f, 1.590000f, -0.620000f}, {0.870000f, -1.169999f, -0.620000f}, {0.000000f, 0.000000f, 0.000000f}, -0.349999f, -0.330000f},
        {{0.870000f, 1.409999f, 1.019999f}, {0.000000f, 0.000000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.300000f, 0.500000f, -0.250000f}, {0.300000f, -0.349999f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.349999f, 5.969999f, 1.519999f}, {0.419999f, -0.119999f, -0.280000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.080000f, -1.879999f, 0.140000f}, {0.870000f, 1.620000f, -0.400000f}, {0.870000f, -1.490000f, -0.400000f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.100000f},
        {{2.210000f, 6.139999f, 1.629999f}, {0.409999f, -0.289999f, -0.289999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.100000f, -2.049999f, 0.029999f}, {0.870000f, 1.450000f, -0.400000f}, {0.870000f, -1.649999f, -0.400000f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.250000f},
        {{2.529999f, 6.039999f, 1.649999f}, {0.460000f, 0.180000f, -0.140000f}, {0.460000f, -0.769999f, -0.140000f}, {-1.159999f, -1.830000f, 0.109999f}, {0.920000f, 1.789999f, -0.349999f}, {0.920000f, -1.779999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.289999f, -0.310000f},
        {{2.539999f, 5.699999f, 2.140000f}, {0.500000f, -0.090000f, 0.059999f}, {0.000000f, 0.000000f, 0.000000f}, {1.159999f, -0.740000f, 0.000000f}, {1.029999f, 1.720000f, -0.660000f}, {1.029999f, -1.470000f, -0.660000f}, {0.000000f, 0.000000f, 0.000000f}, -0.379999f, -0.340000f},
        {{2.250000f, 5.219999f, 1.169999f}, {0.469999f, -0.340000f, -0.259999f}, {0.000000f, 0.000000f, 0.000000f}, {1.049999f, -1.110000f, -0.050000f}, {0.889999f, 1.220000f, -0.340000f}, {0.889999f, -1.659999f, -0.340000f}, {0.000000f, 0.000000f, 0.000000f}, -0.189999f, -0.180000f},
        {{2.569999f, 6.590000f, 1.620000f}, {0.540000f, 0.050000f, -0.250000f}, {0.540000f, -1.149999f, -0.259999f}, {-1.169999f, -2.299999f, 0.109999f}, {1.039999f, 1.799999f, -0.449999f}, {1.039999f, -1.809999f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, -0.409999f, -0.409999f},
        {{2.960000f, 8.050000f, 3.339999f}, {0.540000f, 1.419999f, -0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.379999f, -3.049999f, -0.670000f}, {0.990000f, 2.289999f, -0.899999f}, {1.220000f, -2.289999f, -0.899999f}, {0.000000f, 0.000000f, 0.000000f}, -0.879999f, -0.870000f},
        {{0.709999f, 1.899999f, 1.320000f}, {0.000000f, -0.479999f, 0.460000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.639999f, -0.239999f}, {0.000000f, -0.589999f, -0.189999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.720000f, 1.740000f, 1.120000f}, {0.000000f, -0.270000f, 0.540000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.600000f, -0.259999f}, {0.000000f, -0.569999f, -0.259999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{21.219999f, 21.190000f, 5.059999f}, {0.419999f, 2.009999f, -0.819999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{11.159999f, 6.159999f, 2.990000f}, {0.000000f, -0.219999f, -0.100000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{8.699999f, 9.000000f, 2.230000f}, {0.000000f, -0.910000f, -0.439999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.190000f, 10.069999f, 3.059999f}, {0.449999f, 1.600000f, 0.189999f}, {0.000000f, 0.000000f, 0.000000f}, {1.429999f, 0.379999f, -0.649999f}, {1.230000f, 3.529999f, -0.949999f}, {1.230000f, -4.429999f, -0.949999f}, {1.230000f, -3.269999f, -0.949999f}, -0.860000f, -50.000000f},
        {{3.549999f, 9.949999f, 3.420000f}, {0.560000f, 1.340000f, 0.090000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.450000f, 0.479999f, -1.309999f}, {1.440000f, 3.490000f, -1.389999f}, {1.440000f, -4.079999f, -1.389999f}, {1.440000f, -2.640000f, -1.389999f}, -1.159999f, -50.000000f},
        {{2.599999f, 6.239999f, 1.710000f}, {0.469999f, 0.070000f, -0.150000f}, {0.490000f, -0.990000f, -0.129999f}, {-1.100000f, -2.450000f, -0.029999f}, {0.910000f, 1.669999f, -0.419999f}, {0.910000f, -1.600000f, -0.419999f}, {0.000000f, 0.000000f, 0.000000f}, -0.170000f, -0.170000f},
        {{2.529999f, 6.329999f, 1.649999f}, {0.509999f, -0.019999f, -0.180000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.179999f, -1.950000f, 0.070000f}, {0.920000f, 1.679999f, -0.409999f}, {0.920000f, -1.480000f, -0.409999f}, {0.000000f, 0.000000f, 0.000000f}, -0.310000f, -0.219999f},
        {{2.430000f, 6.000000f, 1.570000f}, {0.509999f, 0.070000f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, {1.210000f, -2.190000f, -0.070000f}, {0.939999f, 1.809999f, -0.259999f}, {0.980000f, -1.460000f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, -0.330000f, -0.230000f},
        {{20.309999f, 19.290000f, 6.940000f}, {0.349999f, 5.179999f, -0.119999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{8.750000f, 14.319999f, 2.160000f}, {0.000000f, 2.839999f, 0.090000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.699999f, 2.460000f, 1.679999f}, {0.000000f, -0.400000f, 0.469999f}, {0.000000f, -0.850000f, 0.600000f}, {0.000000f, 0.129999f, 0.610000f}, {0.000000f, 0.759999f, -0.159999f}, {0.000000f, -0.850000f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.699999f, 2.460000f, 1.679999f}, {0.000000f, -0.400000f, 0.469999f}, {0.000000f, -0.850000f, 0.629999f}, {0.000000f, 0.140000f, 0.610000f}, {0.000000f, 0.759999f, -0.159999f}, {0.000000f, -0.879999f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.699999f, 2.470000f, 1.679999f}, {0.000000f, -0.400000f, 0.469999f}, {0.000000f, -0.680000f, 0.550000f}, {0.000000f, 0.150000f, 0.550000f}, {0.000000f, 0.759999f, -0.159999f}, {0.000000f, -0.850000f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.589999f, 8.840000f, 3.640000f}, {0.490000f, 1.450000f, -0.140000f}, {0.000000f, 0.000000f, 0.000000f}, {1.529999f, 0.479999f, -0.990000f}, {1.279999f, 2.950000f, -1.240000f}, {1.330000f, -2.980000f, -1.309999f}, {1.330000f, -1.860000f, -1.309999f}, -1.000000f, -0.230000f},
        {{3.049999f, 6.469999f, 3.289999f}, {0.509999f, 0.259999f, 0.340000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.389999f, -0.509999f, -0.090000f}, {1.029999f, 2.089999f, -0.379999f}, {1.100000f, -2.099999f, -0.379999f}, {0.000000f, 0.000000f, 0.000000f}, 0.050000f, -50.000000f},
        {{2.200000f, 5.400000f, 1.259999f}, {0.469999f, -0.079999f, -0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {-0.990000f, -1.960000f, 0.059999f}, {0.870000f, 1.490000f, -0.400000f}, {0.879999f, -1.500000f, -0.400000f}, {0.000000f, 0.000000f, 0.000000f}, -0.379999f, -0.310000f},
        {{2.430000f, 5.719999f, 1.740000f}, {0.409999f, -0.019999f, -0.079999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.139999f, -1.690000f, 0.140000f}, {0.910000f, 1.610000f, -0.349999f}, {0.910000f, -1.340000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.079999f, -0.079999f},
        {{2.549999f, 5.550000f, 2.140000f}, {0.500000f, -0.140000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.090000f, -2.029999f, 0.119999f}, {0.949999f, 1.740000f, -0.610000f}, {0.949999f, -1.539999f, -0.610000f}, {0.000000f, 0.000000f, 0.000000f}, -0.490000f, -0.389999f},
        {{2.380000f, 5.630000f, 1.860000f}, {0.490000f, -0.079999f, -0.039999f}, {0.490000f, -1.039999f, -0.039999f}, {-1.200000f, -2.220000f, 0.200000f}, {0.949999f, 1.450000f, -0.250000f}, {0.949999f, -1.659999f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -0.019999f, -0.029999f},
        {{1.580000f, 4.230000f, 2.680000f}, {0.000000f, -0.319999f, 0.389999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.490000f, 0.569999f, -0.379999f}, {0.509999f, -0.709999f, -0.419999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{1.960000f, 3.700000f, 1.669999f}, {0.000000f, -0.460000f, 0.070000f}, {0.000000f, 0.000000f, 0.000000f}, {-0.009999f, 0.819999f, 0.430000f}, {0.829999f, 1.429999f, -0.620000f}, {0.709999f, -1.070000f, -0.300000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{8.619999f, 11.390000f, 4.179999f}, {1.009999f, 3.000000f, 0.589999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {2.880000f, 3.690000f, -1.710000f}, {1.759999f, -1.669999f, -1.500000f}, {1.820000f, 1.669999f, -1.149999f}, -50.000000f, -50.000000f},
        {{2.380000f, 5.420000f, 1.490000f}, {0.479999f, -0.239999f, -0.119999f}, {0.000000f, 0.000000f, 0.000000f}, {1.019999f, -1.960000f, 0.140000f}, {0.889999f, 1.570000f, -0.349999f}, {0.889999f, -1.570000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.239999f, -0.239999f},
        {{2.180000f, 6.269999f, 1.159999f}, {0.589999f, 0.029999f, -0.280000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.039999f, -0.819999f, -0.200000f}, {0.930000f, 1.860000f, -0.270000f}, {0.930000f, -1.529999f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, -0.400000f, -0.330000f},
        {{2.670000f, 5.480000f, 1.580000f}, {0.419999f, 0.009999f, -0.150000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.200000f, -0.560000f, 0.270000f}, {0.970000f, 1.590000f, -0.330000f}, {0.970000f, -1.620000f, -0.330000f}, {0.000000f, 0.000000f, 0.000000f}, -0.370000f, -50.000000f},
        {{2.460000f, 6.420000f, 1.299999f}, {0.519999f, -0.219999f, -0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.070000f, -1.690000f, 0.090000f}, {0.899999f, 1.750000f, -0.349999f}, {0.899999f, -1.730000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.340000f, -0.370000f},
        {{3.329999f, 18.430000f, 5.199999f}, {0.460000f, 5.130000f, 0.529999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.269999f, 16.590000f, 4.949999f}, {0.709999f, 5.989999f, 0.370000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.500000f, 3.869999f, 2.559999f}, {0.000000f, 0.349999f, 0.109999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.589999f, 6.070000f, 1.500000f}, {0.519999f, -0.059999f, -0.259999f}, {0.550000f, -1.350000f, -0.259999f}, {-1.169999f, -2.519999f, -0.019999f}, {1.000000f, 1.799999f, -0.469999f}, {1.000000f, -1.899999f, -0.469999f}, {0.000000f, 0.000000f, 0.000000f}, -0.209999f, -0.209999f},
        {{2.269999f, 4.949999f, 1.240000f}, {0.479999f, -0.119999f, -0.209999f}, {0.000000f, 0.000000f, 0.000000f}, {1.009999f, -1.990000f, 0.150000f}, {0.899999f, 1.500000f, -0.250000f}, {0.899999f, -1.500000f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -0.300000f, -0.239999f},
        {{2.480000f, 6.400000f, 1.700000f}, {0.509999f, -0.150000f, -0.119999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.120000f, -1.919999f, 0.310000f}, {0.889999f, 1.620000f, -0.370000f}, {0.889999f, -1.580000f, -0.370000f}, {0.000000f, 0.000000f, 0.000000f}, -0.129999f, 0.059999f},
        {{2.380000f, 5.730000f, 1.860000f}, {0.449999f, 0.090000f, -0.050000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.100000f, -0.949999f, 0.000000f}, {0.930000f, 1.580000f, -0.449999f}, {0.930000f, -1.580000f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, -0.389999f, -0.330000f},
        {{2.809999f, 12.859999f, 3.890000f}, {0.600000f, 2.509999f, 0.289999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.299999f, 2.069999f, 0.319999f}, {1.049999f, 2.450000f, -0.699999f}, {1.049999f, -2.049999f, -0.699999f}, {0.000000f, 0.000000f, 0.000000f}, -0.620000f, -50.000000f},
        {{2.190000f, 4.809999f, 1.690000f}, {0.330000f, -0.340000f, -0.170000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, -2.220000f, -0.319999f}, {0.850000f, 1.419999f, -0.449999f}, {0.850000f, -1.419999f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, -0.529999f, -0.529999f},
        {{2.569999f, 5.860000f, 1.669999f}, {0.569999f, -0.159999f, -0.140000f}, {0.509999f, -1.159999f, -0.109999f}, {1.090000f, -2.029999f, 0.150000f}, {0.990000f, 1.629999f, -0.340000f}, {0.970000f, -1.919999f, -0.340000f}, {0.000000f, 0.000000f, 0.000000f}, -0.070000f, -0.050000f},
        {{2.490000f, 5.840000f, 1.769999f}, {0.519999f, 0.000000f, -0.059999f}, {0.519999f, -1.000000f, -0.059999f}, {-1.169999f, -2.009999f, 0.119999f}, {0.920000f, 1.610000f, -0.349999f}, {0.920000f, -1.480000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.079999f, -0.070000f},
        {{4.179999f, 24.420000f, 4.909999f}, {0.649999f, 4.570000f, -0.810000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.200000f, 5.389999f, -2.430000f}, {1.669999f, -1.500000f, -2.430000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.400000f, 5.530000f, 1.429999f}, {0.469999f, 0.250000f, -0.150000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.080000f, -1.179999f, 0.219999f}, {0.970000f, 1.740000f, -0.360000f}, {0.970000f, -1.460000f, -0.360000f}, {0.000000f, 0.000000f, 0.000000f}, -0.129999f, -0.119999f},
        {{2.539999f, 5.889999f, 1.539999f}, {0.519999f, 0.050000f, -0.189999f}, {0.519999f, -0.920000f, -0.230000f}, {-1.070000f, -2.490000f, 0.039999f}, {0.980000f, 1.679999f, -0.419999f}, {0.980000f, -1.870000f, -0.419999f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.289999f},
        {{2.660000f, 6.719999f, 1.769999f}, {0.490000f, 0.109999f, -0.129999f}, {0.490000f, -1.059999f, -0.109999f}, {-1.149999f, -2.670000f, 0.090000f}, {0.939999f, 1.830000f, -0.419999f}, {0.939999f, -1.769999f, -0.430000f}, {0.000000f, 0.000000f, 0.000000f}, -0.150000f, -0.140000f},
        {{2.650000f, 6.719999f, 3.559999f}, {0.620000f, 0.639999f, 0.330000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.289999f, -0.959999f, 0.490000f}, {1.039999f, 2.259999f, -0.180000f}, {1.039999f, -1.429999f, -0.180000f}, {0.000000f, 0.000000f, 0.000000f}, -0.029999f, -50.000000f},
        {{28.739999f, 23.489999f, 7.389999f}, {0.430000f, 9.529999f, -0.100000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.680000f, 6.170000f, 2.089999f}, {0.540000f, 0.100000f, -0.029999f}, {0.000000f, 0.000000f, 0.000000f}, {1.210000f, -2.390000f, 0.119999f}, {1.100000f, 1.679999f, -0.569999f}, {1.100000f, -1.700000f, -0.569999f}, {0.000000f, 0.000000f, 0.000000f}, -0.340000f, -0.360000f},
        {{2.000000f, 5.130000f, 1.419999f}, {0.439999f, -0.170000f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, {-0.790000f, -1.500000f, 0.180000f}, {0.759999f, 1.529999f, -0.330000f}, {0.720000f, -1.539999f, -0.330000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.660000f, 6.369999f, 3.289999f}, {0.540000f, 0.100000f, 0.639999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {1.389999f, 1.710000f, -0.449999f}, {1.389999f, -1.710000f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, 0.270000f, 0.270000f},
        {{3.660000f, 6.260000f, 3.289999f}, {0.540000f, 0.000000f, 0.629999f}, {0.000000f, 0.000000f, 0.000000f}, {1.190000f, -2.569999f, 0.850000f}, {1.389999f, 1.669999f, -0.449999f}, {1.389999f, -1.659999f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, 0.270000f, 0.250000f},
        {{2.230000f, 5.250000f, 1.750000f}, {0.540000f, -0.239999f, -0.019999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.090000f, -1.940000f, 0.270000f}, {0.889999f, 1.350000f, -0.250000f}, {0.889999f, -1.519999f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, 0.050000f, 0.039999f},
        {{2.279999f, 5.480000f, 1.399999f}, {0.479999f, -0.189999f, -0.200000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.080000f, -1.710000f, 0.270000f}, {0.939999f, 1.460000f, -0.250000f}, {0.930000f, -1.460000f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -0.100000f, 0.009999f},
        {{2.319999f, 5.400000f, 1.620000f}, {0.479999f, 0.109999f, -0.079999f}, {0.509999f, -0.750000f, -0.090000f}, {1.139999f, -1.899999f, 0.129999f}, {0.949999f, 1.549999f, -0.250000f}, {0.949999f, -1.450000f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, 0.019999f, 0.029999f},
        {{2.500000f, 5.800000f, 1.789999f}, {0.519999f, 0.090000f, -0.129999f}, {0.519999f, -0.819999f, -0.129999f}, {1.110000f, -2.299999f, 0.109999f}, {0.930000f, 1.710000f, -0.409999f}, {0.910000f, -1.409999f, -0.409999f}, {0.000000f, 0.000000f, 0.000000f}, -0.180000f, -0.189999f},
        {{2.259999f, 5.309999f, 1.509999f}, {0.509999f, -0.050000f, -0.140000f}, {0.000000f, 0.000000f, 0.000000f}, {1.039999f, -0.699999f, 0.070000f}, {0.899999f, 1.549999f, -0.310000f}, {0.910000f, -1.299999f, -0.310000f}, {0.000000f, 0.000000f, 0.000000f}, -0.050000f, -0.019999f},
        {{3.390000f, 18.620000f, 4.710000f}, {0.419999f, 2.640000f, -0.340000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.870000f, 1.409999f, 1.019999f}, {0.059999f, -0.070000f, 0.159999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.259999f, 0.400000f, 0.029999f}, {0.259999f, -0.400000f, 0.029999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.029999f, 4.820000f, 1.500000f}, {0.479999f, -0.059999f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, {0.910000f, -0.870000f, 0.029999f}, {0.829999f, 1.389999f, -0.270000f}, {0.829999f, -1.389999f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, -0.070000f, -0.059999f},
        {{2.509999f, 6.460000f, 1.659999f}, {0.540000f, 0.090000f, -0.170000f}, {0.540000f, -1.059999f, -0.180000f}, {1.080000f, -2.440000f, 0.019999f}, {0.980000f, 1.710000f, -0.349999f}, {0.980000f, -1.710000f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.250000f, -0.270000f},
        {{2.710000f, 6.630000f, 1.580000f}, {0.509999f, 0.250000f, -0.270000f}, {0.509999f, -1.000000f, -0.270000f}, {0.000000f, -2.859999f, -0.460000f}, {1.000000f, 2.069999f, -0.479999f}, {1.000000f, -1.480000f, -0.479999f}, {0.000000f, 0.000000f, 0.000000f}, -0.430000f, -0.449999f},
        {{2.710000f, 4.610000f, 1.419999f}, {0.000000f, -0.029999f, -0.189999f}, {0.000000f, 0.000000f, 0.000000f}, {-0.500000f, -0.660000f, 0.379999f}, {0.850000f, 1.750000f, -0.500000f}, {0.870000f, -0.910000f, -0.430000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.259999f, 18.430000f, 5.030000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.470000f, 21.069999f, 5.190000f}, {0.000000f, 1.960000f, -1.129999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{1.570000f, 2.329999f, 1.580000f}, {0.000000f, -0.180000f, -0.100000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.189999f, 0.019999f}, {0.519999f, 0.569999f, -0.159999f}, {0.550000f, -0.629999f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, -0.170000f, -50.000000f},
        {{1.659999f, 2.349999f, 2.019999f}, {0.000000f, -0.620000f, 0.310000f}, {0.000000f, 0.000000f, 0.000000f}, {-0.379999f, -0.970000f, 0.159999f}, {0.479999f, 0.689999f, -0.349999f}, {0.479999f, -0.689999f, -0.289999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.930000f, 7.380000f, 3.160000f}, {0.560000f, 2.029999f, 0.270000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.179999f, 0.479999f, -0.479999f}, {1.049999f, 1.769999f, -0.930000f}, {0.959999f, -1.759999f, -0.930000f}, {0.000000f, 0.000000f, 0.000000f}, -0.629999f, -0.730000f},
        {{1.620000f, 3.849999f, 2.500000f}, {0.230000f, 0.550000f, 0.250000f}, {0.000000f, 0.000000f, 0.000000f}, {-0.750000f, -0.970000f, 0.280000f}, {0.569999f, 0.800000f, -0.439999f}, {0.569999f, -0.800000f, -0.439999f}, {0.000000f, 0.000000f, 0.000000f}, -0.300000f, -0.300000f},
        {{2.490000f, 5.829999f, 1.929999f}, {0.500000f, 0.059999f, 0.009999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, -2.779999f, -0.050000f}, {0.920000f, 1.710000f, -0.250000f}, {0.920000f, -1.710000f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.209999f},
        {{2.420000f, 6.360000f, 1.850000f}, {0.469999f, -0.159999f, -0.090000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, -3.160000f, -0.250000f}, {0.930000f, 1.710000f, -0.250000f}, {0.920000f, -1.710000f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -0.310000f, -0.270000f},
        {{62.490001f, 61.439998f, 34.950000f}, {1.289999f, 21.059999f, 4.119999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.150000f, 11.789999f, 2.779999f}, {0.469999f, 3.430000f, 0.059999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.240000f, 2.910000f, -0.019999f}, {1.250000f, 3.000000f, -1.000000f}, {1.250000f, -3.980000f, -1.000000f}, {1.250000f, -2.750000f, -1.000000f}, -0.860000f, -50.000000f},
        {{2.470000f, 6.210000f, 2.559999f}, {0.460000f, -0.159999f, 0.219999f}, {0.430000f, -1.230000f, 0.239999f}, {1.220000f, -2.299999f, 0.170000f}, {0.959999f, 1.580000f, -0.439999f}, {0.959999f, -1.580000f, -0.439999f}, {0.000000f, 0.000000f, 0.000000f}, -0.180000f, -0.180000f},
        {{2.660000f, 5.769999f, 2.240000f}, {0.540000f, -0.009999f, 0.059999f}, {0.430000f, -0.910000f, 0.050000f}, {1.190000f, -1.820000f, 0.239999f}, {1.080000f, 1.779999f, -0.300000f}, {1.080000f, -1.779999f, -0.300000f}, {0.000000f, 0.000000f, 0.000000f}, -0.349999f, -0.259999f},
        {{0.699999f, 2.460000f, 1.679999f}, {0.000000f, -0.400000f, 0.469999f}, {0.000000f, -0.850000f, 0.600000f}, {0.000000f, 0.129999f, 0.540000f}, {0.000000f, 0.759999f, -0.159999f}, {0.000000f, -0.819999f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.440000f, 7.219999f, 3.190000f}, {0.419999f, 0.560000f, -0.039999f}, {0.550000f, -2.490000f, -0.079999f}, {-1.059999f, 0.140000f, -0.280000f}, {0.899999f, 1.860000f, -0.620000f}, {0.899999f, -2.069999f, -0.620000f}, {0.000000f, 0.000000f, 0.000000f}, -0.449999f, -0.460000f},
        {{1.669999f, 3.660000f, 3.210000f}, {0.000000f, -1.009999f, 0.610000f}, {0.000000f, 0.000000f, 0.000000f}, {-0.759999f, 0.409999f, -0.059999f}, {0.629999f, 1.129999f, -0.219999f}, {0.620000f, -1.120000f, -0.140000f}, {0.000000f, 0.000000f, 0.000000f}, -0.029999f, -50.000000f},
        {{3.549999f, 15.909999f, 3.400000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {1.129999f, -2.640000f, -1.450000f}, {1.129999f, -3.779999f, -1.450000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.440000f, 6.530000f, 2.059999f}, {0.469999f, 0.000000f, 0.070000f}, {0.430000f, -1.049999f, 0.050000f}, {1.139999f, -2.299999f, 0.200000f}, {1.009999f, 1.799999f, -0.230000f}, {1.009999f, -1.799999f, -0.230000f}, {0.000000f, 0.000000f, 0.000000f}, 0.019999f, 0.070000f},
        {{0.699999f, 2.799999f, 1.960000f}, {0.000000f, -0.419999f, 0.379999f}, {0.000000f, -0.769999f, 0.490000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.850000f, -0.159999f}, {0.000000f, -0.839999f, -0.159999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.609999f, 5.760000f, 1.450000f}, {0.519999f, -0.239999f, -0.230000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.230000f, -1.220000f, 0.100000f}, {0.980000f, 1.480000f, -0.360000f}, {0.980000f, -1.620000f, -0.360000f}, {0.000000f, 0.000000f, 0.000000f}, -0.150000f, -0.059999f},
        {{3.079999f, 8.619999f, 7.530000f}, {0.699999f, 2.289999f, 0.469999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.460000f, -2.160000f, 0.219999f}, {1.289999f, 2.130000f, -0.469999f}, {1.289999f, -2.119999f, -0.469999f}, {0.000000f, 0.000000f, 0.000000f}, -0.560000f, -0.560000f},
        {{2.259999f, 5.090000f, 2.119999f}, {0.419999f, -0.019999f, 0.050000f}, {0.000000f, 0.000000f, 0.000000f}, {0.980000f, -0.889999f, 0.119999f}, {0.959999f, 1.639999f, -0.230000f}, {0.959999f, -1.629999f, -0.230000f}, {0.000000f, 0.000000f, 0.000000f}, 0.009999f, 0.009999f},
        {{3.440000f, 18.399999f, 5.030000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.180000f, 13.630000f, 4.659999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {1.009999f, -1.440000f, -0.949999f}, {1.009999f, -2.730000f, -0.949999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{44.450000f, 57.569999f, 18.430000f}, {0.560000f, 12.510000f, -0.419999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{12.590000f, 13.550000f, 3.569999f}, {0.319999f, 0.029999f, -0.070000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{0.500000f, 0.920000f, 0.300000f}, {0.059999f, -0.070000f, 0.159999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.219999f, 0.150000f, 0.070000f}, {0.219999f, -0.150000f, 0.070000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.849999f, 13.470000f, 2.210000f}, {0.479999f, 0.899999f, -0.009999f}, {-0.019999f, -4.289999f, 0.469999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.410000f, 5.909999f, 1.769999f}, {0.519999f, 0.059999f, -0.140000f}, {0.519999f, -1.029999f, -0.109999f}, {-1.110000f, -2.160000f, 0.100000f}, {0.920000f, 1.639999f, -0.349999f}, {0.920000f, -1.639999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.319999f},
        {{2.410000f, 5.909999f, 1.769999f}, {0.519999f, 0.059999f, -0.140000f}, {0.519999f, -1.029999f, -0.109999f}, {-1.100000f, -2.150000f, 0.100000f}, {0.920000f, 1.639999f, -0.349999f}, {0.920000f, -1.639999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.280000f, -0.319999f},
        {{2.410000f, 5.789999f, 1.769999f}, {0.519999f, 0.059999f, -0.059999f}, {0.519999f, -1.029999f, -0.109999f}, {-1.080000f, -1.960000f, 0.159999f}, {0.959999f, 1.639999f, -0.349999f}, {0.959999f, -1.639999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.219999f, -0.200000f},
        {{2.920000f, 6.159999f, 2.140000f}, {0.500000f, -0.090000f, 0.059999f}, {0.000000f, 0.000000f, 0.000000f}, {1.159999f, -0.740000f, 0.000000f}, {1.029999f, 1.720000f, -0.660000f}, {1.029999f, -1.470000f, -0.660000f}, {0.000000f, 0.000000f, 0.000000f}, -0.379999f, -0.340000f},
        {{2.400000f, 6.059999f, 1.559999f}, {0.519999f, 0.059999f, -0.100000f}, {0.000000f, 0.000000f, 0.000000f}, {1.090000f, -2.029999f, 0.079999f}, {0.959999f, 1.830000f, -0.340000f}, {0.959999f, -1.409999f, -0.340000f}, {0.000000f, 0.000000f, 0.000000f}, -0.310000f, -0.310000f},
        {{3.079999f, 6.960000f, 3.829999f}, {0.529999f, -0.300000f, 0.419999f}, {0.000000f, 0.000000f, 0.000000f}, {-1.330000f, -1.690000f, 0.920000f}, {0.930000f, 1.679999f, -0.070000f}, {0.920000f, -1.669999f, -0.070000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.319999f, 5.539999f, 1.289999f}, {0.479999f, -0.119999f, -0.219999f}, {0.000000f, 0.000000f, 0.000000f}, {1.090000f, -1.990000f, 0.109999f}, {0.930000f, 1.539999f, -0.400000f}, {0.930000f, -1.529999f, -0.400000f}, {0.000000f, 0.000000f, 0.000000f}, -0.419999f, -0.360000f},
        {{2.640000f, 6.070000f, 1.429999f}, {0.519999f, -0.419999f, -0.270000f}, {0.000000f, 0.000000f, 0.000000f}, {1.179999f, -2.190000f, -0.079999f}, {1.019999f, 1.600000f, -0.409999f}, {1.019999f, -1.580000f, -0.409999f}, {0.000000f, 0.000000f, 0.000000f}, -0.310000f, -0.259999f},
        {{2.529999f, 6.170000f, 1.649999f}, {0.500000f, 0.150000f, -0.140000f}, {0.460000f, -0.769999f, -0.140000f}, {0.000000f, -3.000000f, -0.129999f}, {0.920000f, 1.789999f, -0.349999f}, {0.920000f, -1.779999f, -0.349999f}, {0.000000f, 0.000000f, 0.000000f}, -0.289999f, -0.310000f},
        {{2.380000f, 5.730000f, 1.860000f}, {0.449999f, 0.090000f, -0.050000f}, {0.000000f, 0.000000f, 0.000000f}, {-1.100000f, -0.949999f, 0.000000f}, {0.930000f, 1.580000f, -0.449999f}, {0.930000f, -1.580000f, -0.449999f}, {0.000000f, 0.000000f, 0.000000f}, -0.389999f, -0.330000f},
        {{2.930000f, 3.380000f, 1.970000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.639999f, 1.169999f, -0.689999f}, {0.639999f, -1.169999f, -0.689999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{3.019999f, 3.259999f, 1.600000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.639999f, 1.190000f, -0.699999f}, {0.639999f, -1.000000f, -0.699999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{1.450000f, 4.650000f, 6.360000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.740000f, 1.240000f, -1.169999f}, {0.740000f, -1.269999f, -1.169999f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.900000f, 6.599999f, 4.219999f}, {0.730000f, 1.379999f, 0.300000f}, {0.600000f, -2.460000f, 0.039999f}, {-1.299999f, 0.000000f, 0.100000f}, {1.090000f, 2.410000f, -0.579999f}, {0.990000f, -1.740000f, -0.579999f}, {0.000000f, 0.000000f, 0.000000f}, -0.469999f, -0.560000f},
        {{2.480000f, 1.429999f, 1.629999f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {1.139999f, -0.409999f, -0.140000f}, {1.139999f, -0.720000f, -0.140000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f},
        {{2.130000f, 3.160000f, 1.830000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.000000f, 0.000000f, 0.000000f}, {0.930000f, 0.340000f, -0.250000f}, {0.930000f, -0.379999f, -0.250000f}, {0.000000f, 0.000000f, 0.000000f}, -50.000000f, -50.000000f}
    } };

    bool getModelInfo(int model, VehicleModelInfoType type, Vector3& out) override {
        if (model < 400 || model > 612) {
            return false;
        }

        model -= 400;
        switch (type) {
            case VehicleModelInfo_Size: {
                out = allModelInfo[model].Size;
                break;
            }
            case VehicleModelInfo_FrontSeat: {
                out = allModelInfo[model].FrontSeat;
                break;
            }
            case VehicleModelInfo_RearSeat: {
                out = allModelInfo[model].RearSeat;
                break;
            }
            case VehicleModelInfo_PetrolCap: {
                out = allModelInfo[model].PetrolCap;
                break;
            }
            case VehicleModelInfo_WheelsFront: {
                out = allModelInfo[model].FrontWheel;
                break;
            }
            case VehicleModelInfo_WheelsRear: {
                out = allModelInfo[model].RearWheel;
                break;
            }
            case VehicleModelInfo_WheelsMid: {
                out = allModelInfo[model].MidWheel;
                break;
            }
            case VehicleModelInfo_FrontBumperZ: {
                out.x = out.y = out.z = allModelInfo[model].FrontBumperZ;
                break;
            }
            case VehicleModelInfo_RearBumperZ: {
                out.x = out.y = out.z = allModelInfo[model].RearBumperZ;
                break;
            }
            default: {
                return false;
            }
        }
        return true;
    }
};

PLUGIN_ENTRY_POINT() {
	return new VehiclePlugin();
}
