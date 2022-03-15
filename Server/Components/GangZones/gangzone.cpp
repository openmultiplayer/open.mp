#include "gangzone.hpp"

using namespace Impl;

class GangZonesComponent final : public IGangZonesComponent, public PlayerEventHandler {
private:
    ICore* core = nullptr;
    MarkedPoolStorage<GangZone, IGangZone, 0, GANG_ZONE_POOL_SIZE> storage;
    DefaultEventDispatcher<GangZoneEventHandler> eventDispatcher;

public:
    StringView componentName() const override
    {
        return "GangZones";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        this->core->getPlayers().getEventDispatcher().addEventHandler(this);
    }

    ~GangZonesComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().addEventHandler(this);
        }
    }

    IGangZone* create(GangZonePos pos) override
    {
        return storage.emplace(pos);
    }

    void free() override
    {
        delete this;
    }

    virtual Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    IGangZone* get(int index) override
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

    IEventDispatcher<PoolEventHandler<IGangZone>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }

    IEventDispatcher<GangZoneEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    /// Get a set of all the available labels
    const FlatPtrHashSet<IGangZone>& entries() override
    {
        return storage._entries();
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (IGangZone* g : storage) {
            GangZone* gangzone = static_cast<GangZone*>(g);
            if (gangzone->shownFor_.valid(pid)) {
                gangzone->shownFor_.remove(pid, player);
            }
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new GangZonesComponent();
}

