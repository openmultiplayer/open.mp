/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include <sdk.hpp>
#include <Server/Components/NPCs/npcs.hpp>
#include <Impl/network_impl.hpp>
#include "../NPC/npc.hpp"

using namespace Impl;

class NPCNetwork : public Impl::Network
{
private:
	ICore* core;
	INPCComponent* npcComponent;
	DynamicArray<int> markedToBeKicked;

public:
	void init(ICore* c, INPCComponent* comp)
	{
		core = c;
		npcComponent = comp;
	}

	DynamicArray<int>& getMarkedForKickNPCs()
	{
		return markedToBeKicked;
	}

	ENetworkType getNetworkType() const override
	{
		return ENetworkType(3);
	}

	bool sendPacket(IPlayer& peer, Span<uint8_t> data, int channel, bool dispatchEvents = true) override
	{
		// core->logLn(LogLevel::Error, "[npc network] sendPacket(\"%.*s\", data, %i, %i)\n", peer.getName().length(), peer.getName().data(), channel, dispatchEvents);
		return true;
	}

	bool broadcastPacket(Span<uint8_t> data, int channel, const IPlayer* exceptPeer = nullptr, bool dispatchEvents = true) override
	{
		// core->logLn(LogLevel::Error, "[npc network] broadcastPacket(data, %i, \"%.*s\", %i)\n", channel, exceptPeer == nullptr ? 0 : exceptPeer->getName().length(), exceptPeer == nullptr ? "" : exceptPeer->getName().data(), dispatchEvents);
		return true;
	}

	bool sendRPC(IPlayer& peer, int id, Span<uint8_t> data, int channel, bool dispatchEvents = true) override
	{
		// core->logLn(LogLevel::Error, "[npc network] sendRpc(\"%.*s\", %i, data, %i, %i)\n", peer.getName().length(), peer.getName().data(), id, channel, dispatchEvents);
		return true;
	}

	bool broadcastRPC(int id, Span<uint8_t> data, int channel, const IPlayer* exceptPeer = nullptr, bool dispatchEvents = true) override
	{
		// core->logLn(LogLevel::Error, "[npc network] broadcastRPC(%i, data, %i, \"%.*s\", %i)\n", id, channel, exceptPeer == nullptr ? 0 : exceptPeer->getName().length(), exceptPeer == nullptr ? "" : exceptPeer->getName().data(), dispatchEvents);
		return true;
	}

	NetworkStats getStatistics(IPlayer* player = nullptr) override
	{
		return NetworkStats();
	}

	unsigned getPing(const IPlayer& peer) override
	{
		return 0;
	}

	void disconnect(const IPlayer& peer) override
	{
		auto id = peer.getID();
		auto npc = npcComponent->get(id);
		if (npc)
		{
			reinterpret_cast<NPC*>(npc)->setKickMarkState(true);
			markedToBeKicked.push_back(npc->getID());
		}
	}

	void ban(const BanEntry& entry, Milliseconds expire = Milliseconds(0)) override
	{
	}

	void unban(const BanEntry& entry) override
	{
	}

	void update() override
	{
	}

	NPCNetwork()
		: Network(256, 256)
	{
	}

	~NPCNetwork() { }
};
