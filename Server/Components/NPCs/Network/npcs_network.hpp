/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <sdk.hpp>
#include <Impl/network_impl.hpp>

struct NPCNetwork : public Impl::Network
{
	ICore* core;
	void init(ICore* core)
	{
		this->core = core;
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