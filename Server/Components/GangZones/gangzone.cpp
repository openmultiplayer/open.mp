/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "gangzone.hpp"
#include <legacy_id_mapper.hpp>

using namespace Impl;

// TODO: This internal/external IDs mapping code should be extracted for other components to use.
class PlayerGangZoneData final : public IPlayerGangZoneData
{
private:
	FiniteLegacyIDMapper<GANG_ZONE_POOL_SIZE>
		legacyIDs_,
		clientIDs_;

public:
	PlayerGangZoneData()
	{
		reset();
	}

	virtual void freeExtension() override
	{
		delete this;
	}

	virtual void reset() override
	{
		// Clear all the IDs.
		for (int i = 0; i != GANG_ZONE_POOL_SIZE; ++i)
		{
			legacyIDs_.release(i);
			clientIDs_.release(i);
		}
	}

	virtual int toLegacyID(int zoneid) const override
	{
		return legacyIDs_.toLegacy(zoneid);
	}

	virtual int fromLegacyID(int legacy) const override
	{
		return legacyIDs_.fromLegacy(legacy);
	}

	virtual int reserveLegacyID() override
	{
		return legacyIDs_.reserve();
	}

	virtual void releaseLegacyID(int legacy) override
	{
		legacyIDs_.release(legacy);
	}

	virtual void setLegacyID(int legacy, int zoneid) override
	{
		return legacyIDs_.set(legacy, zoneid);
	}

	virtual int toClientID(int zoneid) const override
	{
		return clientIDs_.toLegacy(zoneid);
	}

	virtual int fromClientID(int client) const override
	{
		return clientIDs_.fromLegacy(client);
	}

	virtual int reserveClientID() override
	{
		return clientIDs_.reserve();
	}

	virtual void releaseClientID(int client) override
	{
		clientIDs_.release(client);
	}

	virtual void setClientID(int client, int zoneid) override
	{
		return clientIDs_.set(client, zoneid);
	}
};

class GangZonesComponent final : public IGangZonesComponent, public PlayerConnectEventHandler, public PlayerClickEventHandler, public PlayerUpdateEventHandler, public PoolEventHandler<IPlayer>
{
private:
	ICore* core = nullptr;
	constexpr static const size_t Lower = 1;
	constexpr static const size_t Upper = GANG_ZONE_POOL_SIZE * (PLAYER_POOL_SIZE + 1) + Lower;

	MarkedDynamicPoolStorage<GangZone, IGangZone, Lower, Upper> storage;
	UniqueIDArray<IGangZone, Upper> checkingList;
	DefaultEventDispatcher<GangZoneEventHandler> eventDispatcher;
	FiniteLegacyIDMapper<GANG_ZONE_POOL_SIZE> legacyIDs_;

public:
	StringView componentName() const override
	{
		return "GangZones";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		this->core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
		this->core->getPlayers().getPlayerClickDispatcher().addEventHandler(this);
		this->core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
		this->core->getPlayers().getPoolEventDispatcher().addEventHandler(this);
	}

	~GangZonesComponent()
	{
		if (core)
		{
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
			core->getPlayers().getPlayerClickDispatcher().removeEventHandler(this);
			core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(this);
			core->getPlayers().getPoolEventDispatcher().removeEventHandler(this);
		}
	}

	void reset() override
	{
		checkingList.clear();
		storage.clear();
		// Clear all the IDs.
		for (int i = 0; i != GANG_ZONE_POOL_SIZE; ++i)
		{
			legacyIDs_.release(i);
		}
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerGangZoneData(), true);
	}

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{
		// Only go through those that are added to our checking list using IGangZonesComponent::useGangZoneCheck
		if (checkingList.entries().size())
		{
			const Vector3& playerPos = player.getPosition();
			DynamicArray<IGangZone*> enteredList;
			// Guarantee a single allocation
			enteredList.reserve(checkingList.entries().size());

			for (auto gangzone : checkingList.entries())
			{
				// Only check visible gangzones
				if (!gangzone->isShownForPlayer(player))
				{
					continue;
				}

				const GangZonePos& pos = gangzone->getPosition();
				bool isPlayerInInsideList = gangzone->isPlayerInside(player);
				bool isPlayerInZoneArea = playerPos.x >= pos.min.x && playerPos.x < pos.max.x && playerPos.y >= pos.min.y && playerPos.y < pos.max.y;

				if (isPlayerInZoneArea && !isPlayerInInsideList)
				{
					// Collect entered gangzones to call events with them later after exit events
					enteredList.push_back(gangzone);
				}
				else if (!isPlayerInZoneArea && isPlayerInInsideList)
				{
					// Call leave gangzone events
					ScopedPoolReleaseLock<IGangZone> lock(*this, *gangzone);
					static_cast<GangZone*>(gangzone)->setPlayerInside(player, false);
					eventDispatcher.dispatch(
						&GangZoneEventHandler::onPlayerLeaveGangZone,
						player,
						*lock.entry);
				}
			}

			// Call enter gangzone events for all the gangzones in entered gangzone list
			for (auto gangzone : enteredList)
			{
				ScopedPoolReleaseLock<IGangZone> lock(*this, *gangzone);
				static_cast<GangZone*>(gangzone)->setPlayerInside(player, true);
				eventDispatcher.dispatch(
					&GangZoneEventHandler::onPlayerEnterGangZone,
					player,
					*lock.entry);
			}
		}

		return true;
	}

	IGangZone* create(GangZonePos pos) override
	{
		if (pos.max.x < pos.min.x)
		{
			core->logLn(LogLevel::Warning, "Gangzone X co-ordinates %.2f and %.2f out of order, inverting.", pos.min.x, pos.max.x);
			auto tmp = pos.max.x;
			pos.max.x = pos.min.x;
			pos.min.x = tmp;
		}
		if (pos.max.y < pos.min.y)
		{
			core->logLn(LogLevel::Warning, "Gangzone Y co-ordinates %.2f and %.2f out of order, inverting.", pos.min.y, pos.max.y);
			auto tmp = pos.max.y;
			pos.max.y = pos.min.y;
			pos.min.y = tmp;
		}
		return storage.emplace(pos);
	}

	const FlatHashSet<IGangZone*>& getCheckingGangZones() const override
	{
		return checkingList.entries();
	}

	void useGangZoneCheck(IGangZone& zone, bool enable) override
	{
		if (enable)
		{
			checkingList.add(zone.getID(), zone);
		}
		else
		{
			if (checkingList.valid(zone.getID()))
			{
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
		if (zone)
		{
			if (checkingList.valid(index))
			{
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
		for (IGangZone* g : storage)
		{
			GangZone* gangzone = static_cast<GangZone*>(g);
			// Release all the per-player (legacy) gangzones.
			if (gangzone->getLegacyPlayer() == &player)
			{
				release(gangzone->getID());
			}
			else
			{
				gangzone->removeFor(pid, player);
			}
		}
	}

	void onPlayerClickMap(IPlayer& player, Vector3 clickPos) override
	{
		// Only go through those that are added to our checking list using IGangZonesComponent::toggleGangZoneCheck
		for (auto gangzone : checkingList.entries())
		{
			// only check visible gangzones
			if (!gangzone->isShownForPlayer(player))
			{
				continue;
			}

			const GangZonePos& pos = gangzone->getPosition();
			bool isClickInZoneArea = clickPos.x >= pos.min.x && clickPos.x < pos.max.x && clickPos.y >= pos.min.y && clickPos.y < pos.max.y;

			if (isClickInZoneArea)
			{
				ScopedPoolReleaseLock<IGangZone> lock(*this, *gangzone);
				eventDispatcher.dispatch(
					&GangZoneEventHandler::onPlayerClickGangZone,
					player,
					*lock.entry);
			}
		}
	}

	virtual int toLegacyID(int zoneid) const override
	{
		return legacyIDs_.toLegacy(zoneid);
	}

	virtual int fromLegacyID(int legacy) const override
	{
		return legacyIDs_.fromLegacy(legacy);
	}

	virtual int reserveLegacyID() override
	{
		return legacyIDs_.reserve();
	}

	virtual void releaseLegacyID(int legacy) override
	{
		legacyIDs_.release(legacy);
	}

	virtual void setLegacyID(int legacy, int zoneid) override
	{
		return legacyIDs_.set(legacy, zoneid);
	}
};

COMPONENT_ENTRY_POINT()
{
	return new GangZonesComponent();
}
