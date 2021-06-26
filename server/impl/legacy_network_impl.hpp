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

    bool write(const NetworkBitStreamValue& input) override {
        switch (input.type) {
        case NetworkBitStreamValueType::BIT:
            bs.Write(input.b); break;
        case NetworkBitStreamValueType::UINT8:
            bs.Write(input.u8); break;
        case NetworkBitStreamValueType::UINT16:
            bs.Write(input.u16); break;
        case NetworkBitStreamValueType::UINT32:
            bs.Write(input.u32); break;
        case NetworkBitStreamValueType::INT8:
            bs.Write(input.i8); break;
        case NetworkBitStreamValueType::INT16:
            bs.Write(input.i16); break;
        case NetworkBitStreamValueType::INT32:
            bs.Write(input.i32); break;
        case NetworkBitStreamValueType::DOUBLE:
            bs.Write(input.d); break;
        case NetworkBitStreamValueType::FLOAT:
            bs.Write(input.f); break;
        case NetworkBitStreamValueType::VEC2:
            bs.Write(input.v2); break;
        case NetworkBitStreamValueType::VEC3:
            bs.Write(input.v3); break;
        case NetworkBitStreamValueType::VEC4:
            bs.Write(input.v4); break;
        case NetworkBitStreamValueType::FIXED_LEN_STR:
            writeConstString(input.s); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_8:
            writeDynamicString<uint8_t>(input.s); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_16:
            writeDynamicString<uint16_t>(input.s); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_32:
            writeDynamicString<uint32_t>(input.s); break;
        case NetworkBitStreamValueType::FIXED_LEN_UINT8_ARR:
            writeConstArray<uint8_t>(input.au8); break;
            // todo more cases
        }
        return true;
    }

    NetworkBitStreamValue read(const NetworkBitStreamValue& input) override {
        NetworkBitStreamValue res{ input.type };
        read(res);
        return res;
    }

    bool read(NetworkBitStreamValueList input) override {
        for (size_t i = 0; i < input.len; ++i) {
            read(input.data[i]);
            if (input.data[i].type == NetworkBitStreamValueType::NONE) {
                for (size_t j = 0; j < i; ++j) {
                    tryFree(input.data[j]);
                }
                return false;
            }
        }
        return true;
    }

    template <typename LenType>
    void writeDynamicString(const NetworkBitStreamValue::String& input) {
        bs.Write(static_cast<LenType>(input.len));
        bs.Write(input.str, input.len);
    }

    template <typename LenType>
    bool readDynamicString(NetworkBitStreamValue::String& input) {
        LenType len;
        if (!bs.Read(len)) {
            return false;
        }

        input.len = len;
        if (input.len > unsigned(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.str = new char[input.len];
        return bs.Read(input.str, input.len);
    }

    template <typename T>
    void writeConstArray(const NetworkBitStreamValue::Array<T>& input) {
        bs.Write(reinterpret_cast<const char*>(input.data), input.len * sizeof(T));
    }

    template <typename T>
    bool readConstArray(NetworkBitStreamValue::Array<T>& input) {
        if (input.len * sizeof(T) > unsigned(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.data = new T[input.len];
        return bs.Read(reinterpret_cast<char*>(input.data), input.len * sizeof(T));
    }

    void writeConstString(const NetworkBitStreamValue::String& input) {
        bs.Write(input.str, input.len);
    }

    bool readConstString(NetworkBitStreamValue::String& input) {
        if (input.len > (unsigned int)(BITS_TO_BYTES(bs.GetNumberOfUnreadBits()))) {
            return false;
        }

        input.str = new char[input.len];
        return bs.Read(input.str, input.len);
    }

    void read(NetworkBitStreamValue& input) override {
        bool success = false;
        switch (input.type) {
        case NetworkBitStreamValueType::BIT:
            success = bs.Read(input.b); break;
        case NetworkBitStreamValueType::UINT8:
            success = bs.Read(input.u8); break;
        case NetworkBitStreamValueType::UINT16:
            success = bs.Read(input.u16); break;
        case NetworkBitStreamValueType::UINT32:
            success = bs.Read(input.u32); break;
        case NetworkBitStreamValueType::INT8:
            success = bs.Read(input.i8); break;
        case NetworkBitStreamValueType::INT16:
            success = bs.Read(input.i16); break;
        case NetworkBitStreamValueType::INT32:
            success = bs.Read(input.i32); break;
        case NetworkBitStreamValueType::DOUBLE:
            success = bs.Read(input.d); break;
        case NetworkBitStreamValueType::FLOAT:
            success = bs.Read(input.f); break;
        case NetworkBitStreamValueType::VEC2:
            success = bs.Read(input.v2); break;
        case NetworkBitStreamValueType::VEC3:
            success = bs.Read(input.v3); break;
        case NetworkBitStreamValueType::VEC4:
            success = bs.Read(input.v4); break;
        case NetworkBitStreamValueType::FIXED_LEN_STR:
            success = readConstString(input.s); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_8:
            success = readDynamicString<uint8_t>(input.s); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_16:
            success = readDynamicString<uint16_t>(input.s); break;
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_32:
            success = readDynamicString<uint32_t>(input.s); break;
        case NetworkBitStreamValueType::FIXED_LEN_UINT8_ARR:
            success = readConstArray<uint8_t>(input.au8); break;
            // todo more cases
        }
        if (!success) {
            input.type = NetworkBitStreamValueType::NONE;
        }
    }

    void tryFree(NetworkBitStreamValue& input) {
        switch (input.type) {
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_8:
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_16:
        case NetworkBitStreamValueType::DYNAMIC_LEN_STR_32:
        case NetworkBitStreamValueType::FIXED_LEN_STR:
            delete input.s.str;
            break;
        case NetworkBitStreamValueType::FIXED_LEN_UINT8_ARR:
            delete input.au8.data;
            break;
        }
        input.type = NetworkBitStreamValueType::NONE;
    }

    void free(NetworkBitStreamValueList input) override {
        for (size_t i = 0; i < input.len; ++i) {
            tryFree(input.data[i]);
        }
    }
};

struct RakNetLegacyNetwork final : public Network<256, 256>, public CoreEventHandler, public RakNet::PluginInterface {
    RakNetLegacyNetwork(Core& core);
    ~RakNetLegacyNetwork();

    ENetworkType getNetworkType() override {
        return ENetworkType::RakNetLegacy;
    }

    bool sendPacket(IPlayer& player, int id, NetworkBitStreamValueList params) override {
        RakNet::BitStream bs;
        RakNetLegacyBitStream lbs(bs);
        lbs.write(NetworkBitStreamValue::INT16(id));
        for (size_t i = 0; i < params.len; ++i) {
            lbs.write(params.data[i]);
        }

        return rakNetServer.Send(&bs, RakNet::HIGH_PRIORITY, RakNet::UNRELIABLE_SEQUENCED, 0, ridFromPID[player.getID()], false);
    }

    bool sendRPC(int id, NetworkBitStreamValueList params) override {
        RakNet::BitStream bs;
        RakNetLegacyBitStream lbs(bs);
        for (size_t i = 0; i < params.len; ++i) {
            lbs.write(params.data[i]);
        }

        return rakNetServer.RPC(id, &bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_PLAYER_ID, true, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    bool sendRPC(IPlayer& player, int id, NetworkBitStreamValueList params) override {
        RakNet::BitStream bs;
        RakNetLegacyBitStream lbs(bs);
        for (size_t i = 0; i < params.len; ++i) {
            lbs.write(params.data[i]);
        }

        return rakNetServer.RPC(id, &bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, ridFromPID[player.getID()], false, false, RakNet::UNASSIGNED_NETWORK_ID, nullptr);
    }

    static void OnPlayerConnect(RakNet::RPCParameters* rpcParams, void* extra);
    template <size_t ID>
    static void RPCHook(RakNet::RPCParameters* rpcParams, void* extra);

    void onTick(uint64_t tick) override;

    void onInit() override {
        ridFromPID.fill(RakNet::UNASSIGNED_PLAYER_ID);
        rakNetServer.Start(MAX_PLAYERS, 0, 5, 7777);
        rakNetServer.StartOccasionalPing();
    }

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
    std::array<RakNet::PlayerID, MAX_PLAYERS> ridFromPID;
};
