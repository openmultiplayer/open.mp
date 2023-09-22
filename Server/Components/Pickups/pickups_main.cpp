/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "pickup.hpp"
#include <Impl/events_impl.hpp>
#include <legacy_id_mapper.hpp>

using namespace Impl;

// TODO: This internal/external IDs mapping code should be extracted for other components to use.
class PlayerPickupData final : public IPlayerPickupData
{
private:
	FiniteLegacyIDMapper<PICKUP_POOL_SIZE>
		legacyIDs_,
		clientIDs_;

public:
	PlayerPickupData()
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
		for (int i = 0; i != PICKUP_POOL_SIZE; ++i)
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

class PickupsComponent final : public IPickupsComponent, public PlayerConnectEventHandler, public PlayerUpdateEventHandler, public PoolEventHandler<IPlayer>
{
private:
	ICore* core = nullptr;
	constexpr static const size_t Lower = 1;
	constexpr static const size_t Upper = PICKUP_POOL_SIZE * (PLAYER_POOL_SIZE + 1) + Lower;

	MarkedDynamicPoolStorage<Pickup, IPickup, Lower, Upper> storage;
	DefaultEventDispatcher<PickupEventHandler> eventDispatcher;
	IPlayerPool* players = nullptr;
	StreamConfigHelper streamConfigHelper;
	FiniteLegacyIDMapper<PICKUP_POOL_SIZE> legacyIDs_;

	struct PlayerPickUpPickupEventHandler : public SingleNetworkInEventHandler
	{
		PickupsComponent& self;
		PlayerPickUpPickupEventHandler(PickupsComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerPickUpPickup onPlayerPickUpPickupRPC;
			if (!onPlayerPickUpPickupRPC.read(bs))
			{
				return false;
			}

			auto data = queryExtension<IPlayerPickupData>(peer);
			if (!data)
			{
				return false;
			}

			int id = data->fromClientID(onPlayerPickUpPickupRPC.PickupID);
			if (!id)
			{
				return false;
			}

			ScopedPoolReleaseLock lock(self, id);
			if (!lock.entry)
			{
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
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
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
		players->getPlayerConnectDispatcher().addEventHandler(this);
		players->getPoolEventDispatcher().addEventHandler(this);
		NetCode::RPC::OnPlayerPickUpPickup::addEventHandler(*core, &playerPickUpPickupEventHandler);
		streamConfigHelper = StreamConfigHelper(core->getConfig());
	}

	~PickupsComponent()
	{
		if (core)
		{
			players->getPlayerUpdateDispatcher().removeEventHandler(this);
			players->getPlayerConnectDispatcher().removeEventHandler(this);
			players->getPoolEventDispatcher().removeEventHandler(this);
			NetCode::RPC::OnPlayerPickUpPickup::removeEventHandler(*core, &playerPickUpPickupEventHandler);
		}
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerPickupData(), true);
	}

	IPickup* create(int modelId, PickupType type, Vector3 pos, uint32_t virtualWorld, bool isStatic) override
	{
		return storage.emplace(modelId, type, pos, virtualWorld, isStatic);
	}

	void onPoolEntryDestroyed(IPlayer& player) override
	{
		const int pid = player.getID();
		for (IPickup* p : storage)
		{
			Pickup* pickup = static_cast<Pickup*>(p);
			// Release all the per-player (legacy) pickups.
			if (pickup->getLegacyPlayer() == &player)
			{
				release(pickup->getID());
			}
			else
			{
				pickup->removeFor(pid, player);
				pickup->setPickupHiddenForPlayer(player, false);
			}
		}
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Destroy all stored entity instances.
		storage.clear();
		// Clear all the IDs.
		for (int i = 0; i != PICKUP_POOL_SIZE; ++i)
		{
			legacyIDs_.release(i);
		}
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
		if (pickup && !pickup->isStatic())
		{
			static_cast<Pickup*>(pickup)->destream();
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

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{
		const float maxDist = streamConfigHelper.getDistanceSqr();
		if (streamConfigHelper.shouldStream(player.getID(), now))
		{
			const PlayerState state = player.getState();
			if (state == PlayerState_None)
			{
				return true;
			}
			Vector3 pos = player.getPosition();
			for (IPickup* p : storage)
			{
				Pickup* pickup = static_cast<Pickup*>(p);

				const Vector3 dist3D = pickup->getPosition() - pos;
				const bool shouldBeStreamedIn = !pickup->isPickupHiddenForPlayer(player) && (player.getVirtualWorld() == pickup->getVirtualWorld() || pickup->getVirtualWorld() == -1) && glm::dot(dist3D, dist3D) < maxDist;

				const bool isStreamedIn = pickup->isStreamedInForPlayer(player);
				if (!isStreamedIn && shouldBeStreamedIn)
				{
					pickup->streamInForPlayer(player);
				}
				else if (isStreamedIn && !shouldBeStreamedIn)
				{
					pickup->streamOutForPlayer(player);
				}
			}
		}

		return true;
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
	return new PickupsComponent();
}
