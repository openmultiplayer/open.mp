/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "Query/query.hpp"
#include <Impl/network_impl.hpp>
#include <bitstream.hpp>
#include <core.hpp>
#include <glm/glm.hpp>
#include <map>
#include <network.hpp>
#include <raknet/BitStream.h>
#include <raknet/GetTime.h>
#include <raknet/RakNetworkFactory.h>
#include <raknet/RakServerInterface.h>
#include <raknet/StringCompressor.h>

using namespace Impl;

#define MAGNITUDE_EPSILON 0.00001f

#define MAX_MTU_037 576
#define MAX_MTU_DL MAXIMUM_MTU_SIZE

static const StaticArray<StringView, 2> ProtectedRules = {
	"version", "allowed_clients"
};

class Core;

class RakNetLegacyNetwork final : public Network, public CoreEventHandler, public PlayerConnectEventHandler, public PlayerChangeEventHandler, public INetworkQueryExtension
{
private:
	ICore* core = nullptr;
	Query query;
	RakNet::RakServerInterface& rakNetServer;
	StaticArray<IPlayer*, PLAYER_POOL_SIZE> playerFromRakIndex;
	StaticArray<RakNet::RakPeer::RemoteSystemStruct*, PLAYER_POOL_SIZE> playerRemoteSystem;
	Milliseconds cookieSeedTime;
	TimePoint lastCookieSeed;

public:
	inline void setQueryConsole(IConsoleComponent* console)
	{
		query.setConsole(console);
	}

	inline IConsoleComponent* getQueryConsole() const
	{
		return query.getConsole();
	}

	RakNetLegacyNetwork();
	~RakNetLegacyNetwork();

	IExtension* getExtension(UID id) override
	{
		if (id == INetworkQueryExtension::ExtensionIID)
		{
			return static_cast<INetworkQueryExtension*>(this);
		}
		return nullptr;
	}

	ENetworkType getNetworkType() const override
	{
		return ENetworkType_RakNetLegacy;
	}

	void disconnect(const IPlayer& peer) override
	{
		const PeerNetworkData& netData = peer.getNetworkData();
		if (netData.network != this)
		{
			return;
		}

		const PeerNetworkData::NetworkID& nid = netData.networkID;
		const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };

		const int playerIndex = rakNetServer.GetIndexFromPlayerID(rid);
		if (playerIndex >= 0 && playerIndex < PLAYER_POOL_SIZE)
		{
			playerFromRakIndex[playerIndex] = nullptr;
		}
		playerRemoteSystem[peer.getID()] = nullptr;
		rakNetServer.Kick(rid);
	}

	bool broadcastPacket(Span<uint8_t> data, int channel, const IPlayer* exceptPeer, bool dispatchEvents) override
	{
		// We want exact bits - set the write offset with bit granularity
		NetworkBitStream bs(data.data(), bitsToBytes(data.size()), false /* copyData */);
		bs.SetWriteOffset(data.size());

		if (dispatchEvents)
		{
			uint8_t type;
			if (bs.readUINT8(type))
			{
				if (!outEventDispatcher.stopAtFalse([type, &bs](NetworkOutEventHandler* handler)
						{
							bs.SetReadOffset(8); // Ignore packet ID
							return handler->onSendPacket(nullptr, type, bs);
						}))
				{
					return false;
				}

				if (!packetOutEventDispatcher.stopAtFalse(type, [&bs](SingleNetworkOutEventHandler* handler)
						{
							bs.SetReadOffset(8); // Ignore packet ID
							return handler->onSend(nullptr, bs);
						}))
				{
					return false;
				}
			}
		}

		const RakNet::PacketReliability reliability = (channel == OrderingChannel_Unordered) ? RakNet::RELIABLE : RakNet::RELIABLE_ORDERED;
		if (exceptPeer)
		{
			const PeerNetworkData& netData = exceptPeer->getNetworkData();
			if (netData.network == this)
			{
				const PeerNetworkData::NetworkID& nid = netData.networkID;
				const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };

				return rakNetServer.Send((const char*)bs.GetData(), bs.GetNumberOfBitsUsed(), RakNet::HIGH_PRIORITY, reliability, channel, rid, true);
			}
		}

		return rakNetServer.Send((const char*)bs.GetData(), bs.GetNumberOfBitsUsed(), RakNet::HIGH_PRIORITY, reliability, channel, RakNet::UNASSIGNED_PLAYER_ID, true);
	}

	bool sendPacket(IPlayer& peer, Span<uint8_t> data, int channel, bool dispatchEvents) override
	{
		const PeerNetworkData& netData = peer.getNetworkData();
		if (netData.network != this)
		{
			return false;
		}

		// We want exact bits - set the write offset with bit granularity
		NetworkBitStream bs(data.data(), bitsToBytes(data.size()), false /* copyData */);
		bs.SetWriteOffset(data.size());

		if (dispatchEvents)
		{
			uint8_t type;
			if (bs.readUINT8(type))
			{
				if (!outEventDispatcher.stopAtFalse([&peer, type, &bs](NetworkOutEventHandler* handler)
						{
							bs.SetReadOffset(8); // Ignore packet ID
							return handler->onSendPacket(&peer, type, bs);
						}))
				{
					return false;
				}

				if (!packetOutEventDispatcher.stopAtFalse(type, [&peer, &bs](SingleNetworkOutEventHandler* handler)
						{
							bs.SetReadOffset(8); // Ignore packet ID
							return handler->onSend(&peer, bs);
						}))
				{
					return false;
				}
			}
		}

		const PeerNetworkData::NetworkID& nid = netData.networkID;
		const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
		const RakNet::PacketReliability reliability = (channel == OrderingChannel_Reliable) ? RakNet::RELIABLE : ((channel == OrderingChannel_Unordered) ? RakNet::UNRELIABLE : RakNet::UNRELIABLE_SEQUENCED);
		return rakNetServer.Send((const char*)bs.GetData(), bs.GetNumberOfBitsUsed(), RakNet::HIGH_PRIORITY, reliability, channel, rid, false);
	}

	bool broadcastRPC(int id, Span<uint8_t> data, int channel, const IPlayer* exceptPeer, bool dispatchEvents) override
	{
		if (id == INVALID_PACKET_ID)
		{
			return false;
		}

		// We want exact bits - set the write offset with bit granularity
		NetworkBitStream bs(data.data(), bitsToBytes(data.size()), false /* copyData */);
		bs.SetWriteOffset(data.size());

		if (dispatchEvents)
		{
			if (!outEventDispatcher.stopAtFalse([id, &bs](NetworkOutEventHandler* handler)
					{
						bs.resetReadPointer();
						return handler->onSendRPC(nullptr, id, bs);
					}))
			{
				return false;
			}

			if (!rpcOutEventDispatcher.stopAtFalse(id, [&bs](SingleNetworkOutEventHandler* handler)
					{
						bs.resetReadPointer();
						return handler->onSend(nullptr, bs);
					}))
			{
				return false;
			}
		}

		const RakNet::PacketReliability reliability = (channel == OrderingChannel_Unordered) ? RakNet::RELIABLE : RakNet::RELIABLE_ORDERED;
		if (exceptPeer)
		{
			const PeerNetworkData& netData = exceptPeer->getNetworkData();
			if (netData.network == this)
			{
				const PeerNetworkData::NetworkID& nid = netData.networkID;
				const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };

				return rakNetServer.RPC(id, (const char*)bs.GetData(), bs.GetNumberOfUnreadBits(), RakNet::HIGH_PRIORITY, reliability, channel, rid, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
			}
		}

		return rakNetServer.RPC(id, (const char*)bs.GetData(), bs.GetNumberOfUnreadBits(), RakNet::HIGH_PRIORITY, reliability, channel, RakNet::UNASSIGNED_PLAYER_ID, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
	}

	bool sendRPC(IPlayer& peer, int id, Span<uint8_t> data, int channel, bool dispatchEvents) override
	{
		if (id == INVALID_PACKET_ID)
		{
			return false;
		}

		const PeerNetworkData& netData = peer.getNetworkData();
		if (netData.network != this)
		{
			return false;
		}

		// We want exact bits - set the write offset with bit granularity
		NetworkBitStream bs(data.data(), bitsToBytes(data.size()), false /* copyData */);
		bs.SetWriteOffset(data.size());

		if (dispatchEvents)
		{
			if (!outEventDispatcher.stopAtFalse([&peer, id, &bs](NetworkOutEventHandler* handler)
					{
						bs.resetReadPointer();
						return handler->onSendRPC(&peer, id, bs);
					}))
			{
				return false;
			}

			if (!rpcOutEventDispatcher.stopAtFalse(id, [&peer, &bs](SingleNetworkOutEventHandler* handler)
					{
						bs.resetReadPointer();
						return handler->onSend(&peer, bs);
					}))
			{
				return false;
			}
		}

		const PeerNetworkData::NetworkID& nid = netData.networkID;
		const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
		const RakNet::PacketReliability reliability = (channel == OrderingChannel_Unordered) ? RakNet::RELIABLE : RakNet::RELIABLE_ORDERED;
		return rakNetServer.RPC(id, (const char*)bs.GetData(), bs.GetNumberOfBitsUsed(), RakNet::HIGH_PRIORITY, reliability, channel, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
	}

	static void OnPlayerConnect(RakNet::RPCParameters* rpcParams, void* extra);
	static void OnNPCConnect(RakNet::RPCParameters* rpcParams, void* extra);

	IPlayer* OnPeerConnect(RakNet::RPCParameters* rpcParams, bool isNPC, StringView serial, uint32_t version, StringView versionName, uint32_t challenge, StringView name, bool isUsingOmp, bool isUsingOfficialClient = false);
	template <size_t ID>
	static void RPCHook(RakNet::RPCParameters* rpcParams, void* extra);
	void onTick(Microseconds elapsed, TimePoint now) override;
	void init(ICore* core);
	void start();

	void OnRakNetDisconnect(RakNet::PlayerIndex rid, PeerDisconnectReason reason);

	void onPlayerScoreChange(IPlayer& player, int score) override
	{
		query.buildPlayerDependentBuffers();
	}

	void onPlayerNameChange(IPlayer& player, StringView oldName) override
	{
		query.buildPlayerDependentBuffers();
	}

	void update() override;

	void onPlayerConnect(IPlayer& player) override
	{
		query.buildPlayerDependentBuffers();
	}

	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override
	{
		query.buildPlayerDependentBuffers(&player);
	}

	bool addRule(StringView rule, StringView value) override
	{
		if (isRuleProtected(rule))
		{
			return false;
		}

		query.setRuleValue<true>(String(rule), String(value));
		query.buildRulesBuffer();
		return true;
	}

	bool removeRule(StringView rule) override
	{
		if (isRuleProtected(rule))
		{
			return false;
		}

		query.removeRule(rule);
		query.buildRulesBuffer();
		return true;
	}

	bool isValidRule(StringView rule) override
	{
		return query.isValidRule(rule);
	}

	bool isRuleProtected(StringView rule)
	{
		for (StringView r : ProtectedRules)
		{
			if (r == rule)
			{
				return true;
			}
		}
		return false;
	}

	/// Synchronize players after banning an IP, kicking any that match the banned IP
	void synchronizeBans();

	NetworkStats getStatistics(IPlayer* player = nullptr) override;

	unsigned getPing(const IPlayer& peer) override
	{
		auto remoteSystem = playerRemoteSystem[peer.getID()];
		if (remoteSystem == nullptr)
		{
			return -1;
		}

		if (remoteSystem->pingAndClockDifferentialWriteIndex == 0)
		{
			return remoteSystem->pingAndClockDifferential[RakNet::PING_TIMES_ARRAY_SIZE - 1].pingTime;
		}
		else
		{
			return remoteSystem->pingAndClockDifferential[remoteSystem->pingAndClockDifferentialWriteIndex - 1].pingTime;
		}
	}

	void ban(const BanEntry& entry, Milliseconds expire = Milliseconds(0)) override;
	void unban(const BanEntry& entry) override;

	void reset() override
	{
	}
};

struct AnnounceHTTPResponseHandler final : HTTPResponseHandler
{
	ICore* core;

	AnnounceHTTPResponseHandler(ICore* core)
		: core(core)
	{
	}

	void onHTTPResponse(int status, StringView body) override
	{
		if (status != 200)
		{
			core->logLn(LogLevel::Warning, "Couldn't announce legacy network to open.mp list.");
			core->logLn(LogLevel::Warning, "\t Status: %d", status);
			core->logLn(LogLevel::Warning, "\t Message: %.*s", PRINT_VIEW(body));
			core->logLn(LogLevel::Warning, "This won't affect the server's behaviour.");
		}
		delete this;
	}
};
