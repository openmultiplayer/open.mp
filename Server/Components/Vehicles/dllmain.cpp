#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "vehicle.hpp"

struct VehiclePlugin final : public IVehiclesPlugin, public CoreEventHandler {
	ICore* core;
    PoolStorage<Vehicle, IVehicle, VehiclePlugin::Cnt> storage;
    DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
	std::array<uint8_t, MAX_VEHICLE_MODELS> preloadModels;

    struct PlayerEnterVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclePlugin& self;
        PlayerEnterVehicleHandler(VehiclePlugin& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::EnterVehicle enterVehicle;
            if (!enterVehicle.read(bs) || !self.storage.valid(enterVehicle.VehicleID)) {
                return false;
            }

            enterVehicle.PlayerID = peer.getID();
            self.core->getPlayers().broadcastRPC(enterVehicle, BroadcastStreamed, &peer, true);
            return true;
        }
    } playerEnterVehicleHandler;

    struct PlayerExitVehicleHandler : public SingleNetworkInOutEventHandler {
        VehiclePlugin& self;
        PlayerExitVehicleHandler(VehiclePlugin& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::ExitVehicle exitVehicle;
            if (!exitVehicle.read(bs) || !self.storage.valid(exitVehicle.VehicleID)) {
                return false;
            }

            exitVehicle.PlayerID = peer.getID();
            self.core->getPlayers().broadcastRPC(exitVehicle, BroadcastStreamed, &peer, true);
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
        core->removePerRPCEventHandler<NetCode::RPC::EnterVehicle>(&playerEnterVehicleHandler);
        core->removePerRPCEventHandler<NetCode::RPC::ExitVehicle>(&playerExitVehicleHandler);
	}

	void onInit(ICore* core) override {
		this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::EnterVehicle>(&playerEnterVehicleHandler);
        core->addPerRPCEventHandler<NetCode::RPC::ExitVehicle>(&playerExitVehicleHandler);
        claim(0);
	}
    
    void onInit() override {
    }

	const char* pluginName() override {
		return "VehiclesPlugin";
	}

	UUID getUUID() override {
		return 0x3f1f62ee9e22ab19;
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
    const OrderedSet<IVehicle*>& entries() const override {
        return storage.entries();
    }

    void onTick(uint64_t tick) override {
        const float maxDist = 200.f * 200.f;
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
