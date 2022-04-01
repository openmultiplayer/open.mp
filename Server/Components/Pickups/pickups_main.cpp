#include "pickup.hpp"
#include <Impl/events_impl.hpp>

using namespace Impl;

class PickupsComponent final : public IPickupsComponent, public PlayerEventHandler, public PlayerUpdateEventHandler, public ModeResetEventHandler {
private:
    ICore* core = nullptr;
    MarkedPoolStorage<Pickup, IPickup, 0, PICKUP_POOL_SIZE> storage;
    DefaultEventDispatcher<PickupEventHandler> eventDispatcher;
    IPlayerPool* players = nullptr;
    StreamConfigHelper streamConfigHelper;

    struct PlayerPickUpPickupEventHandler : public SingleNetworkInEventHandler {
        PickupsComponent& self;
        PlayerPickUpPickupEventHandler(PickupsComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerPickUpPickup onPlayerPickUpPickupRPC;
            if (!onPlayerPickUpPickupRPC.read(bs)) {
                return false;
            }

            ScopedPoolReleaseLock lock(self, onPlayerPickUpPickupRPC.PickupID);
            if (!lock.entry) {
                return false;
            }

            self.eventDispatcher.dispatch(
                &PickupEventHandler::onPlayerPickUpPickup,
                peer,
                *lock.entry);
            return true;
        }
    } playerPickUpPickupEventHandler;

public:
    StringView componentName() const override
    {
        return "Pickups";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    PickupsComponent()
        : playerPickUpPickupEventHandler(*this)
    {
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        players = &core->getPlayers();
        players->getPlayerUpdateDispatcher().addEventHandler(this);
        players->getEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerPickUpPickup::addEventHandler(*core, &playerPickUpPickupEventHandler);
        streamConfigHelper = StreamConfigHelper(core->getConfig());
    }

    ~PickupsComponent()
    {
        if (core) {
            players->getPlayerUpdateDispatcher().removeEventHandler(this);
            players->getEventDispatcher().removeEventHandler(this);
            NetCode::RPC::OnPlayerPickUpPickup::removeEventHandler(*core, &playerPickUpPickupEventHandler);
        }
    }

    IPickup* create(int modelId, PickupType type, Vector3 pos, uint32_t virtualWorld, bool isStatic) override
    {
        return storage.emplace(modelId, type, pos, virtualWorld, isStatic);
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (IPickup* p : storage)
		{
            static_cast<Pickup*>(p)->removeFor(pid, player);
            static_cast<Pickup*>(p)->setPickupHiddenForPlayer(player, false);
        }
    }

    void free() override
    {
        delete this;
    }

    void onModeReset() override
    {
        // Destroy all stored entity instances.
        storage.clear();
    }

    Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    IPickup* get(int index) override
    {
        return storage.get(index);
    }

    void release(int index) override
    {
        Pickup* pickup = storage.get(index);
        if (pickup && !pickup->isStatic()) {
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

    IEventDispatcher<PoolEventHandler<IPickup>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }

    IEventDispatcher<PickupEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    /// Get a set of all the available labels
    const FlatPtrHashSet<IPickup>& entries() override
    {
        return storage._entries();
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        if (streamConfigHelper.shouldStream(player.getID(), now)) {
            for (IPickup* p : storage) {
                Pickup* pickup = static_cast<Pickup*>(p);

                const PlayerState state = player.getState();
                const Vector3 dist3D = pickup->getPosition() - player.getPosition();
                const bool shouldBeStreamedIn = !pickup->isPickupHiddenForPlayer(player) && state != PlayerState_None && (player.getVirtualWorld() == pickup->getVirtualWorld() || pickup->getVirtualWorld() == -1) && glm::dot(dist3D, dist3D) < maxDist;

                const bool isStreamedIn = pickup->isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    pickup->streamInForPlayer(player);
                } else if (isStreamedIn && !shouldBeStreamedIn) {
                    pickup->streamOutForPlayer(player);
                }
            }
        }

        return true;
    }
};

COMPONENT_ENTRY_POINT()
{
    return new PickupsComponent();
}

