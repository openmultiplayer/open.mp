/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Server/Components/Pawn/pawn.hpp>
#include <Impl/network_impl.hpp>
#include <netcode.hpp>
#include <Impl/pool_impl.hpp>
#include <Server/Components/NPCs/npcs.hpp>
#include "npc.hpp"

struct NPCNetwork : public Impl::Network
{
	ICore* core;
	void init(ICore* core)
	{
		this->core = core;
	}

	/// Get the network type of the network
	/// @return The network type of the network
	ENetworkType getNetworkType() const override
	{
		return ENetworkType_NPC;
	}

	/// Attempt to send a packet to a network peer
	/// @param peer The network peer to send the packet to
	/// @param data The data span with the length in BITS
	/// @param dispatchEvents If calling sendPacket should dispatch send events or not
	bool sendPacket(IPlayer& peer, Span<uint8_t> data, int channel, bool dispatchEvents = true) override
	{
		//core->logLn(LogLevel::Error, "[npc network] sendPacket(\"%.*s\", data, %i, %i)\n", peer.getName().length(), peer.getName().data(), channel, dispatchEvents);
		return true;
	}

	/// Attempt to broadcast a packet to everyone on this network
	/// @param data The data span with the length in BITS
	/// @param exceptPeer send packet to everyone except this peer
	/// @param dispatchEvents dispatch packet related events
	bool broadcastPacket(Span<uint8_t> data, int channel, const IPlayer* exceptPeer = nullptr, bool dispatchEvents = true) override
	{
		//core->logLn(LogLevel::Error, "[npc network] broadcastPacket(data, %i, \"%.*s\", %i)\n", channel, exceptPeer == nullptr ? 0 : exceptPeer->getName().length(), exceptPeer == nullptr ? "" : exceptPeer->getName().data(), dispatchEvents);
		return true;
	}

	/// Attempt to send an RPC to a network peer
	/// @param peer The network peer to send the RPC to
	/// @param id The RPC ID for the current network
	/// @param data The data span with the length in BITS
	/// @param dispatchEvents If calling sendRPC should dispatch send events or not
	bool sendRPC(IPlayer& peer, int id, Span<uint8_t> data, int channel, bool dispatchEvents = true) override
	{
		//core->logLn(LogLevel::Error, "[npc network] sendRpc(\"%.*s\", %i, data, %i, %i)\n", peer.getName().length(), peer.getName().data(), id, channel, dispatchEvents);
		return true;
	}

	/// Attempt to broadcast an RPC to everyone on this network
	/// @param id The RPC ID for the current network
	/// @param data The data span with the length in BITS
	/// @param exceptPeer send RPC to everyone except this peer
	/// @param dispatchEvents dispatch RPC related events
	bool broadcastRPC(int id, Span<uint8_t> data, int channel, const IPlayer* exceptPeer = nullptr, bool dispatchEvents = true) override
	{
		//core->logLn(LogLevel::Error, "[npc network] broadcastRPC(%i, data, %i, \"%.*s\", %i)\n", id, channel, exceptPeer == nullptr ? 0 : exceptPeer->getName().length(), exceptPeer == nullptr ? "" : exceptPeer->getName().data(), dispatchEvents);
		return true;
	}

	/// Get netowrk statistics
	NetworkStats getStatistics(IPlayer* player = nullptr) override
	{
		return NetworkStats();
	}

	/// Get the last ping for a peer on this network or 0 if the peer isn't on this network
	unsigned getPing(const IPlayer& peer) override
	{
		return 0;
	}

	/// Disconnect the peer from the network
	void disconnect(const IPlayer& peer) override
	{
	}

	/// Ban a peer from the network
	void ban(const BanEntry& entry, Milliseconds expire = Milliseconds(0)) override
	{
	}

	/// Unban a peer from the network
	void unban(const BanEntry& entry) override
	{
	}

	/// Update server parameters
	void update() override
	{
	}

	NPCNetwork()
		: Network(256, 256)
	{
	}
	~NPCNetwork() { }
};

struct NPCComponent final : public INPCComponent, public CoreEventHandler
{
	NPCNetwork npcNetwork;
	DefaultEventDispatcher<NPCEventHandler> eventDispatcher;
	MarkedDynamicPoolStorage<NPC, INPC, 0, 1000> storage;

	StringView componentName() const override
	{
		return "Controllable NPCs";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(0, 0, 1, 0);
	}

	void onLoad(ICore* c) override
	{
		core = c;
	}

	UID getUID() override
	{
		return 0x1126fdb4780f11ec;
	}

	void onInit(IComponentList* components) override
	{
		npcNetwork.init(core);
		core->getEventDispatcher().addEventHandler(this);
	}

	void onReady() override { }

	void free() override
	{
		core->getEventDispatcher().removeEventHandler(this);
		delete this;
	}

	void reset() override
	{
	}

	INetwork* getNetwork() override
	{
		return &npcNetwork;
	}

	IEventDispatcher<NPCEventHandler>& getEventDispatcher() override
	{
		return eventDispatcher;
	}

	IEventDispatcher<PoolEventHandler<INPC>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}

	const FlatPtrHashSet<INPC>& entries() override
	{
		return storage._entries();
	}

	Pair<size_t, size_t> bounds() const override
	{
		return std::make_pair(storage.Lower, storage.Upper);
	}

	INPC* get(int index) override
	{
		if (index == -1)
		{
			return nullptr;
		}
		return storage.get(index);
	}

	void release(int index) override
	{
		auto ptr = storage.get(index);
		if (ptr)
		{
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

	void onTick(Microseconds elapsed, TimePoint now) override
	{
		for (auto & npc : storage)
		{
			static_cast<NPC*>(npc)->tick(elapsed, now);
		}
	}

	// Create a controllable NPC
	INPC* create(StringView name) override
	{
		PeerNetworkData data;
		data.network = getNetwork();
		PeerRequestParams request;
		request.bot = false;
		request.name = name;
		Pair<NewConnectionResult, IPlayer*> newConnectionResult { NewConnectionResult_Ignore, nullptr };
		newConnectionResult = core->getPlayers().requestPlayer(data, request);
		if (newConnectionResult.first == NewConnectionResult_NoPlayerSlot)
		{
			core->logLn(LogLevel::Error, "[npc] Server is full!");
			return nullptr;
		}
		else if (newConnectionResult.first == NewConnectionResult_BadName)
		{
			core->logLn(LogLevel::Error, "[npc] NPC has a bad name!");
			return nullptr;
		}
		return storage.emplace(this, newConnectionResult.second);
	}

	ICore* core = nullptr;
	IPawnComponent* pawnComponent;
};
