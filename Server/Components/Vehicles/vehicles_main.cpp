#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "vehicle.hpp"

struct VehiclePlugin final : public IVehiclesPlugin, public CoreEventHandler {
	ICore* core;
    MarkedPoolStorage<Vehicle, IVehicle, VehiclePlugin::Cnt> storage;
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

    VehiclePlugin() :
        playerEnterVehicleHandler(*this),
        playerExitVehicleHandler(*this)
	{
		preloadModels.fill(0);
	}

	~VehiclePlugin()
	{
        core->getEventDispatcher().removeEventHandler(this);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
	}

	void onInit(ICore* core) override {
		this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEnterVehicle>(&playerEnterVehicleHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerExitVehicle>(&playerExitVehicleHandler);
        storage.claimUnusable(0);
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
        storage.release(index, false);
    }

    void lock(int index) override {
        storage.lock(index);
    }

    void unlock(int index) override {
        storage.unlock(index);
    }

    /// Get a set of all the available objects
    ContiguousRefList<IVehicle> entries() override {
        return storage.entries();
    }

    void onTick(std::chrono::microseconds elapsed) override {
        const float maxDist = STREAM_DISTANCE * STREAM_DISTANCE;
        for (IVehicle& vehicle : storage.entries()) {
            const int vw = vehicle.getVirtualWorld();
            const Vector3 pos = vehicle.getPosition();
            for (IPlayer& player : core->getPlayers().entries()) {
                const PlayerState state = player.getState();
                const Vector2 dist2D = pos - player.getPosition();
                const bool shouldBeStreamedIn =
                    state != PlayerState_Spectating &&
                    state != PlayerState_None &&
                    player.getVirtualWorld() == vw &&
                    glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = vehicle.isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    vehicle.streamInForPlayer(player);
                    ScopedPoolReleaseLock lock(*this, vehicle);
                    eventDispatcher.dispatch(&VehicleEventHandler::onStreamIn, lock.entry, player);
                }
                else if (isStreamedIn && !shouldBeStreamedIn) {
                    vehicle.streamOutForPlayer(player);
                    ScopedPoolReleaseLock lock(*this, vehicle);
                    eventDispatcher.dispatch(&VehicleEventHandler::onStreamOut, lock.entry, player);
                }
            }
        }
    }
};

PLUGIN_ENTRY_POINT() {
	return new VehiclePlugin();
}
