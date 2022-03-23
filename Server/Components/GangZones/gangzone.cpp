#include "gangzone.hpp"

using namespace Impl;

struct GangZonesComponent final : public IGangZonesComponent, public PlayerUpdateEventHandler {
    ICore* core = nullptr;
    IPlayerPool* players = nullptr;
    MarkedPoolStorage<GangZone, IGangZone, 0, GANG_ZONE_POOL_SIZE> storage;
    DefaultEventDispatcher<GangZoneEventHandler> eventDispatcher;

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
        players = &core->getPlayers();
        players->getPlayerUpdateDispatcher().addEventHandler(this);
    }

    ~GangZonesComponent()
    {
        if (core) {
            players->getPlayerUpdateDispatcher().removeEventHandler(this);
        }
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        for (auto gangzone : storage._entries()) {

            // getting the list from GangZone implementation instead of interface because
            // we need the modifiable version
            FlatHashSet<IPlayer*>& playersInside = reinterpret_cast<GangZone*>(gangzone)->getPlayersInside();
            const GangZonePos& pos = gangzone->getPosition();
            const Vector3& playerPos = player.getPosition();

            if (playerPos.x >= pos.min.x && playerPos.x <= pos.max.x && playerPos.y >= pos.min.y && playerPos.y <= pos.max.y && playersInside.find(&player) == playersInside.end()) {

                ScopedPoolReleaseLock<IGangZone> lock(*this, *gangzone);
                playersInside.insert(&player);
                eventDispatcher.dispatch(
                    &GangZoneEventHandler::onPlayerEnterGangZone,
                    player,
                    *lock.entry);

            } else if (!(playerPos.x >= pos.min.x && playerPos.x <= pos.max.x && playerPos.y >= pos.min.y && playerPos.y <= pos.max.y) && playersInside.find(&player) != playersInside.end()) {

                ScopedPoolReleaseLock<IGangZone> lock(*this, *gangzone);
                playersInside.erase(&player);
                eventDispatcher.dispatch(
                    &GangZoneEventHandler::onPlayerLeaveGangZone,
                    player,
                    *lock.entry);
            }
        }

        return true;
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
};

COMPONENT_ENTRY_POINT()
{
    return new GangZonesComponent();
}
