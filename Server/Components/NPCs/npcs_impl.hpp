/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <sdk.hpp>
#include <Server/Components/NPCs/npcs.hpp>
#include <Impl/pool_impl.hpp>
#include <Impl/events_impl.hpp>
#include <netcode.hpp>
#include "./Network/npcs_network.hpp"
#include "./NPC/npc.hpp"

using namespace Impl;

class NPCComponent final : public INPCComponent, public CoreEventHandler
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

	INPC* create(StringView name) override;

	void emulateRPCIn(INPC* npc, int rpcId, NetworkBitStream& bs);

	void emulatePacketIn(INPC* npc, int type, NetworkBitStream& bs);

	ICore* getCore()
	{
		return core;
	}

	DefaultEventDispatcher<NPCEventHandler>& getEventDispatcher_internal()
	{
		return eventDispatcher;
	}

private:
	ICore* core = nullptr;
	NPCNetwork npcNetwork;
	DefaultEventDispatcher<NPCEventHandler> eventDispatcher;
	MarkedDynamicPoolStorage<NPC, INPC, 0, NPC_POOL_SIZE> storage;
};
