/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "gangzone.hpp"

using namespace Impl;

class GangZonesComponent final : public IGangZonesComponent, public PlayerEventHandler, public PlayerUpdateEventHandler, public PoolEventHandler<IPlayer> {
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
        this->core->getPlayers().getPoolEventDispatcher().addEventHandler(this);
    }

    ~GangZonesComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(this);
            core->getPlayers().getPoolEventDispatcher().removeEventHandler(this);
        }
    }

    void reset() override
    {
        storage.clear();
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        const Vector3& playerPos = player.getPosition();

        // only go through those that are added to our checking list using IGangZonesComponent::useGangZoneCheck
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

    void useGangZoneCheck(IGangZone& zone, bool enable) override
    {
        if (enable) {
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
        IGangZone* zone = get(index);
        if (zone) {
            if (checkingList.valid(index)) {
                checkingList.remove(index, *zone);
            }
            static_cast<GangZone*>(zone)->destream();
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

    void onPoolEntryDestroyed(IPlayer& player) override
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
