#include "actor.hpp"

struct ActorsPlugin final : public IActorsPlugin, public CoreEventHandler {
	ICore * core;
	MarkedPoolStorage<Actor, IActor, IActorsPlugin::Cnt> storage;
	DefaultEventDispatcher<ActorEventHandler> eventDispatcher;
	IPlayerPool* players;

	struct PlayerDamageActorEventHandler : public SingleNetworkInOutEventHandler {
		ActorsPlugin & self;
		PlayerDamageActorEventHandler(ActorsPlugin& self) : self(self) {}

		bool received(IPlayer & peer, INetworkBitStream & bs) override {
			NetCode::RPC::OnPlayerDamageActor onPlayerDamageActorRPC;
			if (!onPlayerDamageActorRPC.read(bs)) {
				return false;
			}

			if (self.valid(onPlayerDamageActorRPC.ActorID)) {
				Actor& actor = self.storage.get(onPlayerDamageActorRPC.ActorID);
				if (actor.isStreamedInForPlayer(peer) && !actor.invulnerable_) {
					ScopedPoolReleaseLock lock(self, onPlayerDamageActorRPC.ActorID);
					self.eventDispatcher.dispatch(
						&ActorEventHandler::onPlayerDamageActor,
						peer,
						lock.entry,
						onPlayerDamageActorRPC.Damage,
						onPlayerDamageActorRPC.WeaponID,
						BodyPart(onPlayerDamageActorRPC.Bodypart)
					);
				}
			}
			return true;
		}
	} playerDamageActorEventHandler;

	const char * pluginName() override {
		return "Actors";
	}

	ActorsPlugin() :
		players(nullptr),
		playerDamageActorEventHandler(*this)
	{}

	void onInit(ICore * core) override {
		this->core = core;
		players = &core->getPlayers();
		core->getEventDispatcher().addEventHandler(this);
		core->addPerRPCEventHandler<NetCode::RPC::OnPlayerDamageActor>(&playerDamageActorEventHandler);
	}

	~ActorsPlugin() {
		if (core) {
			core->getEventDispatcher().removeEventHandler(this);
			core->removePerRPCEventHandler<NetCode::RPC::OnPlayerDamageActor>(&playerDamageActorEventHandler);
		}
	}

	IActor* create(int skin, Vector3 pos, float angle) override {
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

		Actor& actor = storage.get(pid);
		actor.pos_ = pos;
		actor.skin_ = skin;
		actor.angle_ = angle;
		return &actor;
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

	IActor& get(int index) override {
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

	IEventDispatcher<ActorEventHandler> & getEventDispatcher() override {
		return eventDispatcher;
	}

	/// Get a set of all the available labels
	const FlatPtrHashSet<IActor>& entries() override {
		return storage.entries();
	}

	void onTick(std::chrono::microseconds elapsed) override {
		const float maxDist = STREAM_DISTANCE * STREAM_DISTANCE;
		for (IActor* a : storage.entries()) {
			Actor* actor = static_cast<Actor*>(a);

			for (IPlayer* player : players->entries()) {
				const PlayerState state = player->getState();
				const Vector2 dist2D = actor->pos_ - player->getPosition();
				const bool shouldBeStreamedIn =
					state != PlayerState_Spectating &&
					state != PlayerState_None &&
					player->getVirtualWorld() == actor->virtualWorld_ &&
					glm::dot(dist2D, dist2D) < maxDist;

				const bool isStreamedIn = actor->isStreamedInForPlayer(*player);
				if (!isStreamedIn && shouldBeStreamedIn) {
					actor->streamInForPlayer(*player);
				}
				else if (isStreamedIn && !shouldBeStreamedIn) {
					actor->streamOutForPlayer(*player);
				}
			}
		}
	}
};

PLUGIN_ENTRY_POINT() {
	return new ActorsPlugin();
}
