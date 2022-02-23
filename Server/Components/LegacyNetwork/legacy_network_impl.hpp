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

struct Core;

struct RakNetLegacyNetwork final : public Network, public CoreEventHandler, public PlayerEventHandler, public INetworkQueryExtension {
    RakNetLegacyNetwork();
    ~RakNetLegacyNetwork();

    const IExtension* getExtension(UID id) const override
    {
        if (id == INetworkQueryExtension::ExtensionIID) {
            return static_cast<const INetworkQueryExtension*>(this);
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
        if (netData.network != this) {
            return;
        }

        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        rakNetServer.Kick(rid);
    }

    bool sendPacket(IPlayer& peer, Span<uint8_t> data, int channel) override
    {
        const PeerNetworkData& netData = peer.getNetworkData();
        if (netData.network != this) {
            return false;
        }

        // Don't use constructor because it takes bytes; we want bits
        NetworkBitStream bs;
        bs.SetData(data.data());
        bs.SetWriteOffset(data.size());
        bs.SetReadOffset(0);

        uint8_t type;
        if (bs.readUINT8(type)) {
            if (!outEventDispatcher.stopAtFalse([&peer, type, &bs](NetworkOutEventHandler* handler) {
                    bs.SetReadOffset(8); // Ignore packet ID
                    return handler->sentPacket(&peer, type, bs);
                })) {
                return false;
            }

            if (!packetOutEventDispatcher.stopAtFalse(type, [&peer, &bs](SingleNetworkOutEventHandler* handler) {
                    bs.SetReadOffset(8); // Ignore packet ID
                    return handler->sent(&peer, bs);
                })) {
                return false;
            }
        }

        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        const RakNet::PacketReliability reliability = (channel == OrderingChannel_ReliablePacket) ? RakNet::RELIABLE : ((channel == OrderingChannel_Unordered) ? RakNet::UNRELIABLE : RakNet::UNRELIABLE_SEQUENCED);
        return rakNetServer.Send((const char*)bs.GetData(), bs.GetNumberOfBytesUsed(), RakNet::HIGH_PRIORITY, reliability, channel, rid, false);
    }

    bool broadcastRPC(int id, Span<uint8_t> data, int channel, const IPlayer* exceptPeer) override
    {
        if (id == INVALID_PACKET_ID) {
            return false;
        }

        // Don't use constructor because it takes bytes; we want bits
        NetworkBitStream bs;
        bs.SetData(data.data());
        bs.SetWriteOffset(data.size());
        bs.SetReadOffset(0);

        if (!outEventDispatcher.stopAtFalse([id, &bs](NetworkOutEventHandler* handler) {
                bs.resetReadPointer();
                return handler->sentRPC(nullptr, id, bs);
            })) {
            return false;
        }

        if (!rpcOutEventDispatcher.stopAtFalse(id, [&bs](SingleNetworkOutEventHandler* handler) {
                bs.resetReadPointer();
                return handler->sent(nullptr, bs);
            })) {
            return false;
        }

        const RakNet::PacketReliability reliability = (channel == OrderingChannel_Unordered) ? RakNet::RELIABLE : RakNet::RELIABLE_ORDERED;
        if (exceptPeer) {
            const PeerNetworkData& netData = exceptPeer->getNetworkData();
            if (netData.network == this) {
                const PeerNetworkData::NetworkID& nid = netData.networkID;
                const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };

                return rakNetServer.RPC(id, (const char*)bs.GetData(), bs.GetNumberOfUnreadBits(), RakNet::HIGH_PRIORITY, reliability, channel, rid, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
            }
        }

        return rakNetServer.RPC(id, (const char*)bs.GetData(), bs.GetNumberOfUnreadBits(), RakNet::HIGH_PRIORITY, reliability, channel, RakNet::UNASSIGNED_PLAYER_ID, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    bool sendRPC(IPlayer& peer, int id, Span<uint8_t> data, int channel) override
    {
        if (id == INVALID_PACKET_ID) {
            return false;
        }

        const PeerNetworkData& netData = peer.getNetworkData();
        if (netData.network != this) {
            return false;
        }

        // Don't use constructor because it takes bytes; we want bits
        NetworkBitStream bs;
        bs.SetData(data.data());
        bs.SetWriteOffset(data.size());
        bs.SetReadOffset(0);

        if (!outEventDispatcher.stopAtFalse([&peer, id, &bs](NetworkOutEventHandler* handler) {
                bs.resetReadPointer();
                return handler->sentRPC(&peer, id, bs);
            })) {
            return false;
        }

        if (!rpcOutEventDispatcher.stopAtFalse(id, [&peer, &bs](SingleNetworkOutEventHandler* handler) {
                bs.resetReadPointer();
                return handler->sent(&peer, bs);
            })) {
            return false;
        }

        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        const RakNet::PacketReliability reliability = (channel == OrderingChannel_Unordered) ? RakNet::RELIABLE : RakNet::RELIABLE_ORDERED;
        return rakNetServer.RPC(id, (const char*)bs.GetData(), bs.GetNumberOfBitsUsed(), RakNet::HIGH_PRIORITY, reliability, channel, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    static void OnPlayerConnect(RakNet::RPCParameters* rpcParams, void* extra);
    static void OnNPCConnect(RakNet::RPCParameters* rpcParams, void* extra);

    IPlayer* OnPeerConnect(RakNet::RPCParameters* rpcParams, bool isNPC, StringView serial, uint32_t version, StringView versionName, uint32_t challenge, StringView name);
    template <size_t ID>
    static void RPCHook(RakNet::RPCParameters* rpcParams, void* extra);
    void onTick(Microseconds elapsed, TimePoint now) override;
    void init(ICore* core);

    void OnRakNetDisconnect(RakNet::PlayerIndex rid, PeerDisconnectReason reason);

    void onScoreChange(IPlayer& player, int score) override
    {
        query.buildPlayerDependentBuffers();
    }

    void onNameChange(IPlayer& player, StringView oldName) override
    {
        query.buildPlayerDependentBuffers();
    }

    void update() override;

    void onConnect(IPlayer& player) override
    {
        query.buildPlayerDependentBuffers();
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        query.buildPlayerDependentBuffers(&player);
    }

    void addRule(StringView rule, StringView value) override
    {
        query.setRuleValue(String(rule), String(value));
        query.buildRulesBuffer();
    }

    void removeRule(StringView rule) override
    {
        query.removeRule(rule);
        query.buildRulesBuffer();
    }

    NetworkStats getStatistics(int playerIndex = -1) override;

    unsigned getPing(const IPlayer& peer) override
    {
        const PeerNetworkData& netData = peer.getNetworkData();
        if (netData.network != this) {
            return 0;
        }

        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        return rakNetServer.GetLastPing(rid);
    }

    void ban(const BanEntry& entry, Milliseconds expire = Milliseconds(0)) override;
    void unban(const BanEntry& entry) override;

    ICore* core = nullptr;
    Query query;
    RakNet::RakServerInterface& rakNetServer;
    std::array<IPlayer*, PLAYER_POOL_SIZE> playerFromRakIndex;
    Milliseconds cookieSeedTime;
    TimePoint lastCookieSeed;
};
