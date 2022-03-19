#include "gangzone.hpp"

using namespace Impl;

class GangZonesComponent final : public IGangZonesComponent, public PlayerEventHandler, public PlayerUpdateEventHandler {
private:
    ICore* core = nullptr;
    MarkedPoolStorage<GangZone, IGangZone, 0, GANG_ZONE_POOL_SIZE> storage;
    UniqueIDArray<IGangZone, GANG_ZONE_POOL_SIZE> checkingList;
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
        this->core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
    }

    ~GangZonesComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().addEventHandler(this);
            core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
        }
    }

    void onModeReset() override
    {
        for (IGangZone* a : storage)
		{
            static_cast<GangZone*>(a)->removeForAll();
        }
        storage.clear();
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        const Vector3& playerPos = player.getPosition();

        // only go through those that are added to our checking list using IGangZonesComponent::toggleGangZoneCheck
        for (auto gangzone : checkingList.entries()) {

            // only check visible gangzones
            if (!gangzone->isShownForPlayer(player)) {
                continue;
            }

            const GangZonePos& pos = gangzone->getPosition();
            bool isPlayerInInsideList = gangzone->isPlayerInside(player);
            bool isPlayerInZoneArea = playerPos.x >= pos.min.x && playerPos.x <= pos.max.x && playerPos.y >= pos.min.y && playerPos.y <= pos.max.y;

            if (isPlayerInZoneArea && !isPlayerInInsideList) {

                ScopedPoolReleaseLock<IGangZone> lock(*this, *gangzone);
                static_cast<GangZone*>(gangzone)->setPlayerInside(player, true);
                eventDispatcher.dispatch(
                    &GangZoneEventHandler::onPlayerEnterGangZone,
                    player,
                    *lock.entry);

            } else if (!isPlayerInZoneArea && isPlayerInInsideList) {

                ScopedPoolReleaseLock<IGangZone> lock(*this, *gangzone);
                static_cast<GangZone*>(gangzone)->setPlayerInside(player, false);
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

    const FlatHashSet<IGangZone*>& getCheckingGangZones() const override
    {
        return checkingList.entries();
    }

    void toggleGangZoneCheck(IGangZone& zone, bool toggle) override
    {
        if (toggle) {
            checkingList.add(zone.getID(), zone);
        } else {
            if (checkingList.valid(zone.getID())) {
                checkingList.remove(zone.getID(), zone);
            }
        }
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
        if (checkingList.valid(index)) {
            IGangZone* zone = get(index);
            checkingList.remove(index, *zone);
        }
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
            gangzone->removeFor(pid, player);
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new GangZonesComponent();
}
