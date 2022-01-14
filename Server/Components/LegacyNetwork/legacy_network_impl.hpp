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

struct RakNetLegacyBitStream final : public INetworkBitStream {
    RakNet::BitStream& bs;

    RakNetLegacyBitStream(RakNet::BitStream& bs)
        : bs(bs)
    {
    }

    ENetworkType getNetworkType() const override
    {
        return ENetworkType_RakNetLegacy;
    }

    void reset(ENetworkBitStreamReset reset) override
    {
        if (reset & BSResetRead) {
            bs.ResetReadPointer();
        }
        if (reset & BSResetWrite) {
            bs.ResetWritePointer();
        }
    }

    bool write(const NetworkBitStreamValue& input) override
    {
        switch (input.type) {
        case NetworkBitStreamValueType::BIT:
            bs.Write(absl::get<bool>(input.data));
            break;
        case NetworkBitStreamValueType::UINT8:
            bs.Write(absl::get<uint8_t>(input.data));
            break;
        case NetworkBitStreamValueType::UINT16:
            bs.Write(absl::get<uint16_t>(input.data));
            break;
        case NetworkBitStreamValueType::UINT32:
            bs.Write(absl::get<uint32_t>(input.data));
            break;
        case NetworkBitStreamValueType::UINT64:
            bs.Write(absl::get<uint64_t>(input.data));
            break;
        case NetworkBitStreamValueType::INT8:
            bs.Write(absl::get<int8_t>(input.data));
            break;
        case NetworkBitStreamValueType::INT16:
            bs.Write(absl::get<int16_t>(input.data));
            break;
        case NetworkBitStreamValueType::INT32:
            bs.Write(absl::get<int32_t>(input.data));
            break;
        case NetworkBitStreamValueType::INT64:
            bs.Write(absl::get<int64_t>(input.data));
            break;
        case NetworkBitStreamValueType::DOUBLE:
            bs.Write(absl::get<double>(input.data));
            break;
        case NetworkBitStreamValueType::FLOAT:
            bs.Write(absl::get<float>(input.data));
            break;
        case NetworkBitStreamValueType::VEC2:
            bs.Write(absl::get<Vector2>(input.data));
            break;
        case NetworkBitStreamValueType::VEC3:
            bs.Write(absl::get<Vector3>(input.data));
            break;
        case NetworkBitStreamValueType::VEC4:
            bs.Write(absl::get<Vector4>(input.data));
            break;
        case NetworkBitStreamValueType::FIXED_LEN_STR:
            writeFixedString(absl::get<NetworkString>(input.data));
            break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_8:
            writeDynamicString<uint8_t>(absl::get<NetworkString>(input.data));
            break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_16:
            writeDynamicString<uint16_t>(absl::get<NetworkString>(input.data));
            break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_32:
            writeDynamicString<uint32_t>(absl::get<NetworkString>(input.data));
            break;
        case NetworkBitStreamValueType::FIXED_LEN_ARR_UINT8:
            writeFixedArray<uint8_t>(absl::get<NetworkArray<uint8_t>>(input.data));
            break;
        case NetworkBitStreamValueType::FIXED_LEN_ARR_UINT16:
            writeFixedArray<uint16_t>(absl::get<NetworkArray<uint16_t>>(input.data));
            break;
        case NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32:
            writeFixedArray<uint32_t>(absl::get<NetworkArray<uint32_t>>(input.data));
            break;
        case NetworkBitStreamValueType::HP_ARMOR_COMPRESSED: {
            uint8_t ha = (absl::get<Vector2>(input.data).x >= 100 ? 0x0F : (uint8_t)CEILDIV((int)absl::get<Vector2>(input.data).x, 7)) << 4 | (absl::get<Vector2>(input.data).y >= 100 ? 0x0F : (uint8_t)CEILDIV((int)absl::get<Vector2>(input.data).y, 7));
            bs.Write(ha);
            break;
        }
        case NetworkBitStreamValueType::VEC3_SAMP: {
            Vector3 vector = absl::get<Vector3>(input.data);
            float magnitude = glm::length(vector);
            bs.Write(magnitude);
            if (magnitude > MAGNITUDE_EPSILON) {
                vector /= magnitude;
                bs.WriteCompressed(vector.x);
                bs.WriteCompressed(vector.y);
                bs.WriteCompressed(vector.z);
            }
            break;
        }
        case NetworkBitStreamValueType::GTA_QUAT: {
            const GTAQuat& quat = absl::get<GTAQuat>(input.data);
            bs.WriteNormQuat(quat.q.w, quat.q.x, quat.q.y, quat.q.z);
            break;
        }
        case NetworkBitStreamValueType::COMPRESSED_STR: {
            const NetworkString& str = absl::get<NetworkString>(input.data);
            RakNet::StringCompressor::Instance()->EncodeString(str.data, str.count + 1, &bs);
            break;
        }
        case NetworkBitStreamValueType::NONE:
            assert(false);
            break;
        }
        return true;
    }

    template <typename LenType>
    void writeDynamicString(const NetworkString& input)
    {
        bs.Write(static_cast<LenType>(input.count));
        bs.Write(input.data, input.count);
    }

    template <typename LenType>
    bool readDynamicString(NetworkString& input)
    {
        LenType len;
        if (!bs.Read(len)) {
            return false;
        }

        if (len > unsigned(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.allocate(len);
        return bs.Read(input.data, len);
    }

    template <typename T>
    void writeFixedArray(const NetworkArray<T>& input)
    {
        bs.Write(reinterpret_cast<const char*>(input.data), input.count * sizeof(T));
    }

    template <typename T>
    bool readFixedArray(NetworkArray<T>& input)
    {
        if (input.count * sizeof(T) > unsigned(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.allocate(input.count);
        return bs.Read(reinterpret_cast<char*>(input.data), input.count * sizeof(T));
    }

    void writeFixedString(const NetworkString& input)
    {
        bs.Write(input.data, input.count);
    }

    bool readFixedString(NetworkString& input)
    {
        if (input.count * sizeof(char) > (unsigned int)(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        const unsigned int len = input.count;
        input.allocate(len);
        return bs.Read(input.data, len);
    }

    bool read(NetworkBitStreamValue& input) override
    {
        bool success = false;
        switch (input.type) {
        case NetworkBitStreamValueType::BIT:
            success = bs.Read(input.data.emplace<bool>());
            break;
        case NetworkBitStreamValueType::UINT8:
            success = bs.Read(input.data.emplace<uint8_t>());
            break;
        case NetworkBitStreamValueType::UINT16:
            success = bs.Read(input.data.emplace<uint16_t>());
            break;
        case NetworkBitStreamValueType::UINT32:
            success = bs.Read(input.data.emplace<uint32_t>());
            break;
        case NetworkBitStreamValueType::UINT64:
            success = bs.Read(input.data.emplace<uint64_t>());
            break;
        case NetworkBitStreamValueType::INT8:
            success = bs.Read(input.data.emplace<int8_t>());
            break;
        case NetworkBitStreamValueType::INT16:
            success = bs.Read(input.data.emplace<int16_t>());
            break;
        case NetworkBitStreamValueType::INT32:
            success = bs.Read(input.data.emplace<int32_t>());
            break;
        case NetworkBitStreamValueType::INT64:
            success = bs.Read(input.data.emplace<int64_t>());
            break;
        case NetworkBitStreamValueType::DOUBLE:
            success = bs.Read(input.data.emplace<double>());
            break;
        case NetworkBitStreamValueType::FLOAT:
            success = bs.Read(input.data.emplace<float>());
            break;
        case NetworkBitStreamValueType::VEC2:
            success = bs.Read(input.data.emplace<Vector2>());
            break;
        case NetworkBitStreamValueType::VEC3:
            success = bs.Read(input.data.emplace<Vector3>());
            break;
        case NetworkBitStreamValueType::VEC4:
            success = bs.Read(input.data.emplace<Vector4>());
            break;
        case NetworkBitStreamValueType::FIXED_LEN_STR:
            success = readFixedString(absl::get<NetworkString>(input.data));
            break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_8:
            success = readDynamicString<uint8_t>(input.data.emplace<NetworkString>());
            break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_16:
            success = readDynamicString<uint16_t>(input.data.emplace<NetworkString>());
            break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_32:
            success = readDynamicString<uint32_t>(input.data.emplace<NetworkString>());
            break;
        case NetworkBitStreamValueType::FIXED_LEN_ARR_UINT8:
            success = readFixedArray<uint8_t>(input.data.emplace<NetworkArray<uint8_t>>());
            break;
        case NetworkBitStreamValueType::FIXED_LEN_ARR_UINT16:
            success = readFixedArray<uint16_t>(input.data.emplace<NetworkArray<uint16_t>>());
            break;
        case NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32:
            success = readFixedArray<uint32_t>(input.data.emplace<NetworkArray<uint32_t>>());
            break;
        case NetworkBitStreamValueType::HP_ARMOR_COMPRESSED: {
            uint8_t
                health,
                armour;
            if (!(success = bs.Read(health))) {
                break;
            } else if (!(success = bs.Read(armour))) {
                break;
            }
            input.data.emplace<Vector2>(health, armour);
            break;
        }
        case NetworkBitStreamValueType::GTA_QUAT:
            success = bs.Read(input.data.emplace<GTAQuat>());
            break;
        case NetworkBitStreamValueType::NONE:
            assert(false);
            break;
        }
        return success;
    }
};

struct RakNetLegacyNetwork final : public Network, public CoreEventHandler, public PlayerEventHandler, public INetworkQueryExtension {
    RakNetLegacyNetwork();
    ~RakNetLegacyNetwork();

    const IExtension* getExtension(UUID id) const override
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

    bool sendPacket(const INetworkPeer& peer, const INetworkBitStream& bs) override
    {
        const PeerNetworkData& netData = peer.getNetworkData();
        if (bs.getNetworkType() != ENetworkType_RakNetLegacy || netData.network != this) {
            return false;
        }

        const RakNetLegacyBitStream& lbs = static_cast<const RakNetLegacyBitStream&>(bs);
        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        return rakNetServer.Send(&lbs.bs, RakNet::HIGH_PRIORITY, RakNet::UNRELIABLE_SEQUENCED, 0, rid, false);
    }

    bool broadcastRPC(int id, const INetworkBitStream& bs, const INetworkPeer* exceptPeer) override
    {
        if (id == INVALID_PACKET_ID) {
            return false;
        }

        if (bs.getNetworkType() != ENetworkType_RakNetLegacy) {
            return false;
        }

        const RakNetLegacyBitStream& lbs = static_cast<const RakNetLegacyBitStream&>(bs);

        if (exceptPeer) {
            const PeerNetworkData& netData = exceptPeer->getNetworkData();
            if (netData.network == this) {
                const PeerNetworkData::NetworkID& nid = netData.networkID;
                const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };

                return rakNetServer.RPC(id, &lbs.bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, rid, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
            }
        }

        return rakNetServer.RPC(id, &lbs.bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_PLAYER_ID, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    bool sendRPC(const INetworkPeer& peer, int id, const INetworkBitStream& bs) override
    {
        if (id == INVALID_PACKET_ID) {
            return false;
        }

        const PeerNetworkData& netData = peer.getNetworkData();
        if (bs.getNetworkType() != ENetworkType_RakNetLegacy || netData.network != this) {
            return false;
        }

        const RakNetLegacyBitStream& lbs = static_cast<const RakNetLegacyBitStream&>(bs);
        const PeerNetworkData::NetworkID& nid = netData.networkID;
        const RakNet::PlayerID rid { unsigned(nid.address.v4), nid.port };
        return rakNetServer.RPC(id, &lbs.bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    INetworkBitStream& writeBitStream() override
    {
        wlbs.bs.Reset();
        return wlbs;
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
    RakNet::BitStream wbs;
    RakNetLegacyBitStream wlbs;
    Milliseconds cookieSeedTime;
    TimePoint lastCookieSeed;
};
