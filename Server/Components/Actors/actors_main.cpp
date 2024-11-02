/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "actor.hpp"
#include <Server/Components/Fixes/fixes.hpp>
#include <legacy_id_mapper.hpp>
#include <utils.hpp>

using namespace Impl;

class PlayerActorData final : public IPlayerActorData
{
private:
	FiniteLegacyIDMapper<ACTOR_POOL_SIZE>
		legacyIDs_,
		clientIDs_;

public:
	PlayerActorData()
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
		for (int i = 0; i != ACTOR_POOL_SIZE; ++i)
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

class ActorsComponent final : public IActorsComponent, public PlayerConnectEventHandler, public PlayerUpdateEventHandler, public PoolEventHandler<IPlayer>
{
private:
	ICore* core = nullptr;
	constexpr static const size_t Lower = 1;
	constexpr static const size_t Upper = ACTOR_POOL_SIZE * (PLAYER_POOL_SIZE + 1) + Lower;

	MarkedDynamicPoolStorage<Actor, IActor, Lower, Upper> storage;
	DefaultEventDispatcher<ActorEventHandler> eventDispatcher;
	IPlayerPool* players;
	StreamConfigHelper streamConfigHelper;
	FiniteLegacyIDMapper<ACTOR_POOL_SIZE> legacyIDs_;
	ICustomModelsComponent* modelsComponent = nullptr;
	IFixesComponent* fixesComponent_ = nullptr;

	struct PlayerDamageActorEventHandler : public SingleNetworkInEventHandler
	{
		ActorsComponent& self;
		PlayerDamageActorEventHandler(ActorsComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerDamageActor onPlayerDamageActorRPC;
			if (!onPlayerDamageActorRPC.read(bs))
			{
				return false;
			}

			auto data = queryExtension<IPlayerActorData>(peer);
			if (!data)
			{
				return false;
			}

			int id = data->fromClientID(onPlayerDamageActorRPC.ActorID);
			if (!id)
			{
				return false;
			}

			ScopedPoolReleaseLock lock(self, id);
			if (!lock.entry)
			{
				return false;
			}

			if (onPlayerDamageActorRPC.Damage < 0.0f)
			{
				return false;
			}

			if (!IsWeaponForTakenDamageValid(onPlayerDamageActorRPC.WeaponID))
			{
				return false;
			}

			if (onPlayerDamageActorRPC.Bodypart < BodyPart_Torso || onPlayerDamageActorRPC.Bodypart > BodyPart_Head)
			{
				return false;
			}

			IActor* actor = lock.entry;
			if (actor->isStreamedInForPlayer(peer) && !actor->isInvulnerable())
			{
				self.eventDispatcher.dispatch(
					&ActorEventHandler::onPlayerGiveDamageActor,
					peer,
					*actor,
					onPlayerDamageActorRPC.Damage,
					onPlayerDamageActorRPC.WeaponID,
					BodyPart(onPlayerDamageActorRPC.Bodypart));
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
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
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
		players->getPlayerConnectDispatcher().addEventHandler(this);
		players->getPlayerUpdateDispatcher().addEventHandler(this);
		players->getPoolEventDispatcher().addEventHandler(this);
		NetCode::RPC::OnPlayerDamageActor::addEventHandler(*core, &playerDamageActorEventHandler);
		streamConfigHelper = StreamConfigHelper(core->getConfig());
	}

	void onInit(IComponentList* components) override
	{
		modelsComponent = components->queryComponent<ICustomModelsComponent>();
		fixesComponent_ = components->queryComponent<IFixesComponent>();
	}

	void onFree(IComponent* component) override
	{
		if (component == modelsComponent)
		{
			modelsComponent = nullptr;
		}
		if (component == fixesComponent_)
		{
			fixesComponent_ = nullptr;
		}
	}

	~ActorsComponent()
	{
		if (core)
		{
			players->getPlayerUpdateDispatcher().removeEventHandler(this);
			players->getPlayerConnectDispatcher().removeEventHandler(this);
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
		for (IActor* p : storage)
		{
			Actor* actor = static_cast<Actor*>(p);
			// Release all the per-player (legacy) actors.
			if (actor->getLegacyPlayer() == &player)
			{
				release(actor->getID());
			}
			else
			{
				actor->removeFor(pid, player);
				actor->setActorHiddenForPlayer(player, false);
			}
		}
	}

	IActor* create(int skin, Vector3 pos, float angle) override
	{
		return storage.emplace(skin, pos, angle, core->getConfig().getBool("game.use_all_animations"), core->getConfig().getBool("game.validate_animations"), modelsComponent, fixesComponent_);
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
		if (ptr)
		{
			static_cast<Actor*>(ptr)->destream();
			storage.release(index, false);
			if (fixesComponent_)
			{
				fixesComponent_->clearAnimation(nullptr, ptr);
			}
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
		// Clear all the IDs.
		for (int i = 0; i != ACTOR_POOL_SIZE; ++i)
		{
			legacyIDs_.release(i);
		}
	}

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{
		const float maxDist = streamConfigHelper.getDistanceSqr();
		if (streamConfigHelper.shouldStream(player.getID(), now))
		{
			for (IActor* a : storage)
			{
				Actor* actor = static_cast<Actor*>(a);

				const PlayerState state = player.getState();
				const Vector2 dist2D = actor->getPosition() - player.getPosition();
				const bool shouldBeStreamedIn = !actor->isActorHiddenForPlayer(player) && state != PlayerState_None && (player.getVirtualWorld() == actor->getVirtualWorld() || actor->getVirtualWorld() == -1) && glm::dot(dist2D, dist2D) < maxDist;

				const bool isStreamedIn = actor->isStreamedInForPlayer(player);
				if (!isStreamedIn && shouldBeStreamedIn)
				{
					actor->streamInForPlayer(player);
					ScopedPoolReleaseLock<IActor> lock(*this, *actor);
					eventDispatcher.dispatch(
						&ActorEventHandler::onActorStreamIn,
						*lock.entry,
						player);
				}
				else if (isStreamedIn && !shouldBeStreamedIn)
				{
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
	return new ActorsComponent();
}
