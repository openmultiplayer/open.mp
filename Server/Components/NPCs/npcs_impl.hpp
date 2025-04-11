/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */
#pragma once
#include <sdk.hpp>
#include <Server/Components/Actors/actors.hpp>

#include <Server/Components/NPCs/npcs.hpp>
#include <Impl/pool_impl.hpp>
#include <Impl/events_impl.hpp>
#include <netcode.hpp>
#include "./Network/npcs_network.hpp"
#include "./NPC/npc.hpp"

using namespace Impl;

class NPCComponent final : public INPCComponent, public CoreEventHandler, public PlayerDamageEventHandler
{
public:
	StringView componentName() const override
	{
		return "Controllable NPCs";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(0, 0, 1, 0);
	}

	void onLoad(ICore* c) override;

	void onInit(IComponentList* components) override;

	void onReady() override { }

	void free() override;

	void reset() override
	{
	}

	INetwork* getNetwork() override;

	IEventDispatcher<NPCEventHandler>& getEventDispatcher() override;

	IEventDispatcher<PoolEventHandler<INPC>>& getPoolEventDispatcher() override;

	const FlatPtrHashSet<INPC>& entries() override;

	Pair<size_t, size_t> bounds() const override;

	INPC* get(int index) override;

	void release(int index) override;

	void lock(int index) override;

	bool unlock(int index) override;

	void onTick(Microseconds elapsed, TimePoint now) override;

	void onPlayerGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part) override;

	void onPlayerTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part) override;

	INPC* create(StringView name) override;

	void destroy(INPC& npc) override;

	bool emulatePlayerGiveDamageToNPCEvent(IPlayer& player, INPC& npc, float amount, unsigned weapon, BodyPart part, bool callOriginalEvents);

	bool emulatePlayerTakeDamageFromNPCEvent(IPlayer& player, INPC& npc, float amount, unsigned weapon, BodyPart part, bool callOriginalEvents);

	void emulateRPCIn(IPlayer& player, int rpcId, NetworkBitStream& bs);

	void emulatePacketIn(IPlayer& player, int type, NetworkBitStream& bs);

	ICore* getCore()
	{
		return core;
	}

	IVehiclesComponent* getVehiclesPool()
	{
		return vehicles;
	}

	IObjectsComponent* getObjectsPool()
	{
		return objects;
	}

	IActorsComponent* getActorsPool()
	{
		return actors;
	}

	DefaultEventDispatcher<NPCEventHandler>& getEventDispatcher_internal()
	{
		return eventDispatcher;
	}

	int getFootSyncRate() const
	{
		return *footSyncRate;
	}

	int getGeneralNPCUpdateRate() const
	{
		return *generalNPCUpdateRateMS;
	}

	void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) override
	{
		int defaultGeneralNPCUpdateRateMS = 50;
		if (defaults)
		{
			config.setInt("npc.globalUpdateRate", defaultGeneralNPCUpdateRateMS);
		}
		else
		{
			// Set default values if options are not set.
			if (config.getType("npc.globalUpdateRate") == ConfigOptionType_None)
			{
				config.setInt("npc.globalUpdateRate", defaultGeneralNPCUpdateRateMS);
			}
		}

		generalNPCUpdateRateMS = config.getInt("npc.globalUpdateRate");
	}

private:
	ICore* core = nullptr;
	NPCNetwork npcNetwork;
	DefaultEventDispatcher<NPCEventHandler> eventDispatcher;
	MarkedDynamicPoolStorage<NPC, INPC, 0, NPC_POOL_SIZE> storage;
	bool shouldCallCustomEvents = true;

	// Update rates
	int* generalNPCUpdateRateMS = nullptr;
	int* footSyncRate = nullptr;

	// Components
	IVehiclesComponent* vehicles = nullptr;
	IObjectsComponent* objects = nullptr;
	IActorsComponent* actors = nullptr;
};
