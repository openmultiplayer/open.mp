/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "actor.hpp"

class ActorsComponent final : public IActorsComponent, public PlayerEventHandler, public PlayerUpdateEventHandler, public PoolEventHandler<IPlayer> {
private:
    ICore* core = nullptr;
    MarkedPoolStorage<Actor, IActor, 0, ACTOR_POOL_SIZE> storage;
    DefaultEventDispatcher<ActorEventHandler> eventDispatcher;
    IPlayerPool* players;
    StreamConfigHelper streamConfigHelper;

    struct PlayerDamageActorEventHandler : public SingleNetworkInEventHandler {
        ActorsComponent& self;
        PlayerDamageActorEventHandler(ActorsComponent& self)
            : self(self)
        {
        }

        bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerDamageActor onPlayerDamageActorRPC;
            if (!onPlayerDamageActorRPC.read(bs)) {
                return false;
            }

            if (onPlayerDamageActorRPC.Damage <= 0.0f) {
                return false;
            }

            if (onPlayerDamageActorRPC.Bodypart < BodyPart_Torso || onPlayerDamageActorRPC.Bodypart > BodyPart_Head) {
                return false;
            }

            Actor* actorPtr = self.storage.get(onPlayerDamageActorRPC.ActorID);
            if (actorPtr) {
                Actor& actor = *actorPtr;
                if (actor.isStreamedInForPlayer(peer) && !actor.isInvulnerable()) {
                    ScopedPoolReleaseLock<IActor> lock(self, actor);
                    self.eventDispatcher.dispatch(
                        &ActorEventHandler::onPlayerGiveDamageActor,
                        peer,
                        *lock.entry,
                        onPlayerDamageActorRPC.Damage,
                        onPlayerDamageActorRPC.WeaponID,
                        BodyPart(onPlayerDamageActorRPC.Bodypart));
                }
            }
            return true;
        }
    } playerDamageActorEventHandler;

public:
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
        players->getPoolEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerDamageActor::addEventHandler(*core, &playerDamageActorEventHandler);
        streamConfigHelper = StreamConfigHelper(core->getConfig());
    }

    ~ActorsComponent()
    {
        if (core) {
            players->getPlayerUpdateDispatcher().removeEventHandler(this);
            players->getEventDispatcher().removeEventHandler(this);
            players->getPoolEventDispatcher().removeEventHandler(this);
            NetCode::RPC::OnPlayerDamageActor::removeEventHandler(*core, &playerDamageActorEventHandler);
        }
    }

    void onPlayerConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerActorData(), true);
    }

    void onPoolEntryDestroyed(IPlayer& player) override
    {
        const int pid = player.getID();
        for (IActor* a : storage) {
            static_cast<Actor*>(a)->removeFor(pid, player);
        }
    }

    IActor* create(int skin, Vector3 pos, float angle) override
    {
        return storage.emplace(skin, pos, angle, core->getConfig().getBool("game.use_all_animations"));
    }

    void free() override
    {
        delete this;
    }

    Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    IActor* get(int index) override
    {
        return storage.get(index);
    }

    void release(int index) override
    {
        auto ptr = storage.get(index);
        if (ptr) {
            static_cast<Actor*>(ptr)->destream();
            storage.release(index, false);
        }
    }

    void lock(int index) override
    {
        storage.lock(index);
    }

    bool unlock(int index) override
    {
        return storage.unlock(index);
    }

    IEventDispatcher<PoolEventHandler<IActor>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
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

    void reset() override
    {
        // Destroy all stored entity instances.
        storage.clear();
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        if (streamConfigHelper.shouldStream(player.getID(), now)) {
            for (IActor* a : storage) {
                Actor* actor = static_cast<Actor*>(a);

                const PlayerState state = player.getState();
                const Vector2 dist2D = actor->getPosition() - player.getPosition();
                const bool shouldBeStreamedIn = state != PlayerState_None && (player.getVirtualWorld() == actor->getVirtualWorld() || actor->getVirtualWorld() == -1) && glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = actor->isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    actor->streamInForPlayer(player);
                    ScopedPoolReleaseLock<IActor> lock(*this, *actor);
                    eventDispatcher.dispatch(
                        &ActorEventHandler::onActorStreamIn,
                        *lock.entry,
                        player);
                } else if (isStreamedIn && !shouldBeStreamedIn) {
                    actor->streamOutForPlayer(player);
                    ScopedPoolReleaseLock<IActor> lock(*this, *actor);
                    eventDispatcher.dispatch(
                        &ActorEventHandler::onActorStreamOut,
                        *lock.entry,
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
