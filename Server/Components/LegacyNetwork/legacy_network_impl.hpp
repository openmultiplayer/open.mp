#pragma once

#include "Query/query.hpp"
#include <Impl/network_impl.hpp>
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

    void disconnect(const INetworkPeer& peer) override
    {
        const PeerNetworkData& netData = peer.getNetworkData();
        if (netData.network != this) {
            return;
        }

        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        rakNetServer.Kick(rid);
    }

    bool sendPacket(const INetworkPeer& peer, Span<uint8_t> data) override
    {
        const PeerNetworkData& netData = peer.getNetworkData();
        if (netData.network != this) {
            return false;
        }

        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        return rakNetServer.Send((const char*)data.data(), BITS_TO_BYTES(data.length()), RakNet::HIGH_PRIORITY, RakNet::UNRELIABLE_SEQUENCED, 0, rid, false);
    }

    bool broadcastRPC(int id, Span<uint8_t> data, const INetworkPeer* exceptPeer) override
    {
        if (id == INVALID_PACKET_ID) {
            return false;
        }

        if (exceptPeer) {
            const PeerNetworkData& netData = exceptPeer->getNetworkData();
            if (netData.network == this) {
                const PeerNetworkData::NetworkID& nid = netData.networkID;
                const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };

                return rakNetServer.RPC(id, (const char*)data.data(), data.length(), RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, rid, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
            }
        }

        return rakNetServer.RPC(id, (const char*)data.data(), data.length(), RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_PLAYER_ID, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    bool sendRPC(const INetworkPeer& peer, int id, Span<uint8_t> data) override
    {
        if (id == INVALID_PACKET_ID) {
            return false;
        }

        const PeerNetworkData& netData = peer.getNetworkData();
        if (netData.network != this) {
            return false;
        }

        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        return rakNetServer.RPC(id, (const char*)data.data(), data.length(), RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    static void OnPlayerConnect(RakNet::RPCParameters* rpcParams, void* extra);
    static void OnNPCConnect(RakNet::RPCParameters* rpcParams, void* extra);

    IPlayer* OnPeerConnect(RakNet::RPCParameters* rpcParams, bool isNPC, StringView serial, uint32_t version, StringView versionName, uint32_t challenge, StringView name);
    template <size_t ID>
    static void RPCHook(RakNet::RPCParameters* rpcParams, void* extra);
    void onTick(Microseconds elapsed, TimePoint now) override;
    void init(ICore* core);

    void OnRakNetDisconnect(RakNet::PlayerID rid, PeerDisconnectReason reason);

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

    unsigned getPing(const INetworkPeer& peer) override
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

    typedef std::map<RakNet::PlayerID, std::reference_wrapper<IPlayer>> PlayerFromRIDMap;

    ICore* core;
    Query query;
    RakNet::RakServerInterface& rakNetServer;
    PlayerFromRIDMap playerFromRID;
    Milliseconds cookieSeedTime;
    TimePoint lastCookieSeed;
};
