#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "vehicle.hpp"

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
    PoolStorage<Vehicle, IVehicle, VehiclePlugin::Cnt> storage;
    DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
	std::array<uint8_t, MAX_VEHICLE_MODELS> preloadModels;

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

    void onStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override {
        if (oldState == PlayerState_Driver || oldState == PlayerState_Passenger) {
            IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
            if (data->getVehicle()) {
                data->getVehicle()->setDriver(nullptr);
            }

            data->setVehicle(nullptr);
            data->setSeat(-1);
        }
    }

    VehiclePlugin() :
        playerEnterVehicleHandler(*this),
        playerExitVehicleHandler(*this)
	{
		preloadModels.fill(0);
	}

	~VehiclePlugin()
	{
        core->getEventDispatcher().removeEventHandler(this);
        core->getPlayers().getEventDispatcher().removeEventHandler(this);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
	}

	void onInit(ICore* core) override {
		this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
        claim(0);
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

        IVehicle& vehicle = storage.get(pid);
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

    bool release(int index) override {
        return storage.release(index);
    }

    /// Get a set of all the available objects
    const DynamicArray<IVehicle*>& entries() const override {
        return storage.entries();
    }

    void onTick(uint64_t tick) override {
        const float maxDist = STREAM_DISTANCE * STREAM_DISTANCE;
        for (IVehicle* const& vehicle : storage.entries()) {
            if (vehicle->getID() == 0) {
                continue;
            }

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
        }
    }
};

PLUGIN_ENTRY_POINT() {
	return new VehiclePlugin();
}
