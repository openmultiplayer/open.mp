#include "pickup.hpp"

struct PickupsPlugin final : public IPickupsPlugin, public CoreEventHandler, public PlayerEventHandler {
	ICore * core;
	MarkedPoolStorage<Pickup, IPickup, IPickupsPlugin::Cnt> storage;
	DefaultEventDispatcher<PickupEventHandler> eventDispatcher;
	IPlayerPool * players = nullptr;

	struct PlayerPickUpPickupEventHandler : public SingleNetworkInOutEventHandler {
		PickupsPlugin & self;
		PlayerPickUpPickupEventHandler(PickupsPlugin & self) : self(self) {}

		bool received(IPlayer & peer, INetworkBitStream & bs) override {
			NetCode::RPC::OnPlayerPickUpPickup onPlayerPickUpPickupRPC;
			if (!onPlayerPickUpPickupRPC.read(bs)) {
				return false;
			}

			if (self.valid(onPlayerPickUpPickupRPC.PickupID)) {
				ScopedPoolReleaseLock lock(self, onPlayerPickUpPickupRPC.PickupID);
				self.eventDispatcher.dispatch(
					&PickupEventHandler::onPlayerPickUpPickup,
					peer,
					lock.entry
				);
			}
			return true;
		}
	} playerPickUpPickupEventHandler;

	const char * pluginName() override {
		return "Pickups";
	}

	PickupsPlugin() :
		playerPickUpPickupEventHandler(*this)
	{}

	void onLoad(ICore * core) override {
		this->core = core;
		players = &core->getPlayers();
		core->getEventDispatcher().addEventHandler(this);
		players->getEventDispatcher().addEventHandler(this);
		core->addPerRPCEventHandler<NetCode::RPC::OnPlayerPickUpPickup>(&playerPickUpPickupEventHandler);
	}

	~PickupsPlugin() {
		if (core) {
			core->getEventDispatcher().removeEventHandler(this);
			players->getEventDispatcher().removeEventHandler(this);
			core->removePerRPCEventHandler<NetCode::RPC::OnPlayerPickUpPickup>(&playerPickUpPickupEventHandler);
		}
	}

	IPickup * create(int modelId, PickupType type, Vector3 pos, uint32_t virtualWorld, bool isStatic) override {
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

		Pickup & pickup = storage.get(pid);
		pickup.pos = pos;
		pickup.type = type;
		pickup.virtualWorld = virtualWorld;
		pickup.modelId = modelId;
		pickup.isStatic = isStatic;
		return &pickup;
	}

	void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override {
		const int pid = player.getID();
		for (IPickup* p : storage.entries()) {
			Pickup* pickup = static_cast<Pickup*>(p);
			if (pickup->streamedFor_.valid(pid)) {
				pickup->streamedFor_.remove(pid, player);
			}
		}
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
		return storage.valid(index);
	}

	IPickup & get(int index) override {
		return storage.get(index);
	}

	void release(int index) override {
		if (!storage.get(index).isStatic) {
			storage.release(index, false);
		}
	}

	void lock(int index) override {
		storage.lock(index);
	}

	void unlock(int index) override {
		storage.unlock(index);
	}

	IEventDispatcher<PickupEventHandler> & getEventDispatcher() override {
		return eventDispatcher;
	}

	/// Get a set of all the available labels
	const FlatPtrHashSet<IPickup>& entries() override {
		return storage.entries();
	}

	void onTick(std::chrono::microseconds elapsed) override {
		const float maxDist = STREAM_DISTANCE * STREAM_DISTANCE;
		for (IPickup* p : storage.entries()) {
			Pickup* pickup = static_cast<Pickup*>(p);

			for (IPlayer* player : players->entries()) {
				const PlayerState state = player->getState();
				const Vector3 dist3D = pickup->pos - player->getPosition();
				const bool shouldBeStreamedIn =
					state != PlayerState_Spectating &&
					state != PlayerState_None &&
					player->getVirtualWorld() == pickup->virtualWorld &&
					glm::dot(dist3D, dist3D) < maxDist;

				const bool isStreamedIn = pickup->isStreamedInForPlayer(*player);
				if (!isStreamedIn && shouldBeStreamedIn) {
					pickup->streamInForPlayer(*player);
				}
				else if (isStreamedIn && !shouldBeStreamedIn) {
					pickup->streamOutForPlayer(*player);
				}
			}
		}
	}
};

PLUGIN_ENTRY_POINT() {
	return new PickupsPlugin();
}
