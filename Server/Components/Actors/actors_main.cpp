#include "actor.hpp"

struct ActorsComponent final : public IActorsComponent, public PlayerEventHandler, public PlayerUpdateEventHandler {
    ICore* core;
    MarkedPoolStorage<Actor, IActor, IActorsComponent::Capacity> storage;
    DefaultEventDispatcher<ActorEventHandler> eventDispatcher;
    IPlayerPool* players;
    StreamConfigHelper streamConfigHelper;

    struct PlayerDamageActorEventHandler : public SingleNetworkInOutEventHandler {
        ActorsComponent& self;
        PlayerDamageActorEventHandler(ActorsComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, INetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerDamageActor onPlayerDamageActorRPC;
            if (!onPlayerDamageActorRPC.read(bs)) {
                return false;
            }

            if (self.valid(onPlayerDamageActorRPC.ActorID)) {
                Actor& actor = self.storage.get(onPlayerDamageActorRPC.ActorID);
                if (actor.isStreamedInForPlayer(peer) && !actor.invulnerable_) {
                    ScopedPoolReleaseLock<IActor, Capacity> lock(self, onPlayerDamageActorRPC.ActorID);
                    self.eventDispatcher.dispatch(
                        &ActorEventHandler::onPlayerDamageActor,
                        peer,
                        lock.entry,
                        onPlayerDamageActorRPC.Damage,
                        onPlayerDamageActorRPC.WeaponID,
                        BodyPart(onPlayerDamageActorRPC.Bodypart));
                }
            }
            return true;
        }
    } playerDamageActorEventHandler;

    StringView componentName() const override
    {
        return "Actors";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    ActorsComponent()
        : players(nullptr)
        , playerDamageActorEventHandler(*this)
    {
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        players = &core->getPlayers();
        players->getEventDispatcher().addEventHandler(this);
        players->getPlayerUpdateDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerDamageActor>(&playerDamageActorEventHandler);
        streamConfigHelper = StreamConfigHelper(core->getConfig());
    }

    ~ActorsComponent()
    {
        if (core) {
            players->getPlayerUpdateDispatcher().removeEventHandler(this);
            players->getEventDispatcher().removeEventHandler(this);
            core->removePerRPCEventHandler<NetCode::RPC::OnPlayerDamageActor>(&playerDamageActorEventHandler);
        }
    }

    IPlayerData* onPlayerDataRequest(IPlayer& player) override
    {
        return new PlayerActorData();
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (IActor* a : storage) {
            Actor* actor = static_cast<Actor*>(a);
            if (actor->streamedFor_.valid(pid)) {
                actor->streamedFor_.remove(pid, player);
            }
        }
    }

    IActor* create(int skin, Vector3 pos, float angle) override
    {
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
        actor.invulnerable_ = true;
        return &actor;
    }

    void free() override
    {
        delete this;
    }

    int findFreeIndex() override
    {
        return storage.findFreeIndex();
    }

    int claim() override
    {
        int res = storage.claim();
        return res;
    }

    int claim(int hint) override
    {
        int res = storage.claim(hint);
        return res;
    }

    bool valid(int index) const override
    {
        return storage.valid(index);
    }

    IActor& get(int index) override
    {
        return storage.get(index);
    }

    void release(int index) override
    {
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

    IEventDispatcher<ActorEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    /// Get a set of all the available labels
    const FlatPtrHashSet<IActor>& entries() override
    {
        return storage._entries();
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        if (streamConfigHelper.shouldStream(player.getID(), now)) {
            for (IActor* a : storage) {
                Actor* actor = static_cast<Actor*>(a);

                const PlayerState state = player.getState();
                const Vector2 dist2D = actor->pos_ - player.getPosition();
                const bool shouldBeStreamedIn = state != PlayerState_Spectating && state != PlayerState_None && player.getVirtualWorld() == actor->virtualWorld_ && glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = actor->isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    actor->streamInForPlayer(player);
                    ScopedPoolReleaseLock<IActor, Capacity> lock(*this, actor->getID());
                    eventDispatcher.dispatch(
                        &ActorEventHandler::onActorStreamIn,
                        *actor,
                        player);
                } else if (isStreamedIn && !shouldBeStreamedIn) {
                    actor->streamOutForPlayer(player);
                    ScopedPoolReleaseLock<IActor, Capacity> lock(*this, actor->getID());
                    eventDispatcher.dispatch(
                        &ActorEventHandler::onActorStreamOut,
                        lock.entry,
                        player);
                }
            }
        }

        return true;
    }
};

COMPONENT_ENTRY_POINT()
{
    return new ActorsComponent();
}
