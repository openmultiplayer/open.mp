#pragma once

#include <map>
#include <network.hpp>
#include <core.hpp>
#include <raknet/BitStream.h>
#include <raknet/RakNetworkFactory.h>
#include <raknet/RakServerInterface.h>
#include <raknet/PluginInterface.h>
#include "network_impl.hpp"

struct Core;

struct RakNetLegacyBitStream final : public INetworkBitStream {
    RakNet::BitStream& bs;

    RakNetLegacyBitStream(RakNet::BitStream& bs) : bs(bs) {}

    ENetworkType getNetworkType() override {
        return ENetworkType_RakNetLegacy;
    }

    void reset(ENetworkBitStreamReset reset) override {
        if (reset & BSResetRead) {
            bs.ResetReadPointer();
        }
        if (reset & BSResetWrite) {
            bs.ResetWritePointer();
        }
    }

    bool write(const NetworkBitStreamValue& input) override {
        switch (input.type) {
        case NetworkBitStreamValueType::BIT:
            bs.Write(std::get<bool>(input.data)); break;
        case NetworkBitStreamValueType::UINT8:
            bs.Write(std::get<uint8_t>(input.data)); break;
        case NetworkBitStreamValueType::UINT16:
            bs.Write(std::get<uint16_t>(input.data)); break;
        case NetworkBitStreamValueType::UINT32:
            bs.Write(std::get<uint32_t>(input.data)); break;
        case NetworkBitStreamValueType::UINT64:
            bs.Write(std::get<uint64_t>(input.data)); break;
        case NetworkBitStreamValueType::INT8:
            bs.Write(std::get<int8_t>(input.data)); break;
        case NetworkBitStreamValueType::INT16:
            bs.Write(std::get<int16_t>(input.data)); break;
        case NetworkBitStreamValueType::INT32:
            bs.Write(std::get<int32_t>(input.data)); break;
        case NetworkBitStreamValueType::INT64:
            bs.Write(std::get<int64_t>(input.data)); break;
        case NetworkBitStreamValueType::DOUBLE:
            bs.Write(std::get<double>(input.data)); break;
        case NetworkBitStreamValueType::FLOAT:
            bs.Write(std::get<float>(input.data)); break;
        case NetworkBitStreamValueType::VEC2:
            bs.Write(std::get<vector2>(input.data)); break;
        case NetworkBitStreamValueType::VEC3:
            bs.Write(std::get<vector3>(input.data)); break;
        case NetworkBitStreamValueType::VEC4:
            bs.Write(std::get<vector4>(input.data)); break;
        case NetworkBitStreamValueType::FIXED_LEN_STR:
            writeFixedString(std::get<NetworkString>(input.data)); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_8:
            writeDynamicString<uint8_t>(std::get<NetworkString>(input.data)); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_16:
            writeDynamicString<uint16_t>(std::get<NetworkString>(input.data)); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_32:
            writeDynamicString<uint32_t>(std::get<NetworkString>(input.data)); break;
        case NetworkBitStreamValueType::FIXED_LEN_UINT8_ARR:
            writeFixedArray<uint8_t>(std::get<NetworkArray<uint8_t>>(input.data)); break;
        case NetworkBitStreamValueType::NONE:
            assert(false); break;
        }
        return true;
    }

    template <typename LenType>
    void writeDynamicString(const NetworkString& input) {
        bs.Write(static_cast<LenType>(input.count));
        bs.Write(input.data, input.count);
    }

    template <typename LenType>
    bool readDynamicString(NetworkString& input) {
        LenType len;
        if (!bs.Read(len)) {
            return false;
        }

        if (len > unsigned(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.allocate(len);
        return bs.Read(input.data, input.count);
    }

    template <typename T>
    void writeFixedArray(const NetworkArray<T>& input) {
        bs.Write(reinterpret_cast<const char*>(input.data), input.count * sizeof(T));
    }

    template <typename T>
    bool readFixedArray(NetworkArray<T>& input) {
        if (input.count * sizeof(T) > unsigned(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.allocate(input.count);
        return bs.Read(reinterpret_cast<char*>(input.data), input.count * sizeof(T));
    }

    void writeFixedString(const NetworkString& input) {
        bs.Write(input.data, input.count);
    }

    bool readFixedString(NetworkString& input) {
        if (input.count * sizeof(char) > (unsigned int)(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.allocate(input.count);
        return bs.Read(input.data, input.count);
    }

    bool read(NetworkBitStreamValue& input) override {
        bool success = false;
        switch (input.type) {
        case NetworkBitStreamValueType::BIT:
            success = bs.Read(input.data.emplace<bool>()); break;
        case NetworkBitStreamValueType::UINT8:
            success = bs.Read(input.data.emplace<uint8_t>()); break;
        case NetworkBitStreamValueType::UINT16:
            success = bs.Read(input.data.emplace<uint16_t>()); break;
        case NetworkBitStreamValueType::UINT32:
            success = bs.Read(input.data.emplace<uint32_t>()); break;
        case NetworkBitStreamValueType::UINT64:
            success = bs.Read(input.data.emplace<uint64_t>()); break;
        case NetworkBitStreamValueType::INT8:
            success = bs.Read(input.data.emplace<int8_t>()); break;
        case NetworkBitStreamValueType::INT16:
            success = bs.Read(input.data.emplace<int16_t>()); break;
        case NetworkBitStreamValueType::INT32:
            success = bs.Read(input.data.emplace<int32_t>()); break;
        case NetworkBitStreamValueType::INT64:
            success = bs.Read(input.data.emplace<int64_t>()); break;
        case NetworkBitStreamValueType::DOUBLE:
            success = bs.Read(input.data.emplace<double>()); break;
        case NetworkBitStreamValueType::FLOAT:
            success = bs.Read(input.data.emplace<float>()); break;
        case NetworkBitStreamValueType::VEC2:
            success = bs.Read(input.data.emplace<vector2>()); break;
        case NetworkBitStreamValueType::VEC3:
            success = bs.Read(input.data.emplace<vector3>()); break;
        case NetworkBitStreamValueType::VEC4:
            success = bs.Read(input.data.emplace<vector4>()); break;
        case NetworkBitStreamValueType::FIXED_LEN_STR:
            success = readFixedString(std::get<NetworkString>(input.data)); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_8:
            success = readDynamicString<uint8_t>(input.data.emplace<NetworkString>()); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_16:
            success = readDynamicString<uint16_t>(input.data.emplace<NetworkString>()); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_32:
            success = readDynamicString<uint32_t>(input.data.emplace<NetworkString>()); break;
        case NetworkBitStreamValueType::FIXED_LEN_UINT8_ARR:
            success = readFixedArray<uint8_t>(input.data.emplace<NetworkArray<uint8_t>>()); break;
        case NetworkBitStreamValueType::NONE:
            assert(false); break;
        }
        return success;
    }
};

struct RakNetLegacyNetwork final : public Network<256, 256>, public CoreEventHandler, public RakNet::PluginInterface {
    RakNetLegacyNetwork(Core& core);
    ~RakNetLegacyNetwork();

    ENetworkType getNetworkType() override {
        return ENetworkType_RakNetLegacy;
    }

    bool sendPacket(INetworkPeer& peer, INetworkBitStream& bs) override {
        if (bs.getNetworkType() != ENetworkType_RakNetLegacy || peer.getNetwork().getNetworkType() != ENetworkType_RakNetLegacy) {
            return false;
        }

        RakNetLegacyBitStream& lbs = static_cast<RakNetLegacyBitStream&>(bs);
        const INetworkPeer::NetworkID nid = peer.getNetworkID();
        const RakNet::PlayerID rid{ unsigned(nid.address), nid.port };
        return rakNetServer.Send(&lbs.bs, RakNet::HIGH_PRIORITY, RakNet::UNRELIABLE_SEQUENCED, 0, rid, false);
    }

    bool broadcastRPC(int id, INetworkBitStream& bs) override {
        if (bs.getNetworkType() != ENetworkType_RakNetLegacy) {
            return false;
        }

        RakNetLegacyBitStream& lbs = static_cast<RakNetLegacyBitStream&>(bs);
        return rakNetServer.RPC(id, &lbs.bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_PLAYER_ID, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    bool sendRPC(INetworkPeer& peer, int id, INetworkBitStream& bs) override {
        if (bs.getNetworkType() != ENetworkType_RakNetLegacy || peer.getNetwork().getNetworkType() != ENetworkType_RakNetLegacy) {
            return false;
        }

        RakNetLegacyBitStream& lbs = static_cast<RakNetLegacyBitStream&>(bs);
        const INetworkPeer::NetworkID nid = peer.getNetworkID();
        const RakNet::PlayerID rid{ unsigned(nid.address), nid.port };
        return rakNetServer.RPC(id, &lbs.bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, rid, false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    INetworkBitStream& writeBitStream() override {
        wlbs.bs.Reset();
        return wlbs;
    }

    static void OnPlayerConnect(RakNet::RPCParameters* rpcParams, void* extra);
    template <size_t ID>
    static void RPCHook(RakNet::RPCParameters* rpcParams, void* extra);

    void onTick(uint64_t tick) override;
    void onInit() override;

    void OnRakNetDisconnect(RakNet::PlayerID rid);

    void OnDisconnect(RakNet::RakPeerInterface* peer) override {
        return OnRakNetDisconnect(peer->GetInternalID());
    }

    void OnCloseConnection(RakNet::RakPeerInterface* peer, RakNet::PlayerID playerId) override {
        return OnRakNetDisconnect(playerId);
    }

    Core& core;
    RakNet::RakServerInterface& rakNetServer;
    std::map<RakNet::PlayerID, int> pidFromRID;
    RakNet::BitStream wbs;
    RakNetLegacyBitStream wlbs;
};
