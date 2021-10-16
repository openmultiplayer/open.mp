#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct PlayerRequestClass final : NetworkPacketBase<128> {
        int Classid;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::UINT16>(Classid);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(Classid));
        }
    };

    struct PlayerRequestClassResponse final : NetworkPacketBase<128> {
        uint8_t Selectable;
        uint8_t TeamID;
        uint32_t ModelID;
        uint8_t Unknown1;
        Vector3 Spawn;
        float ZAngle;
        NetworkArray<uint32_t> Weapons;
        NetworkArray<uint32_t> Ammos;

        /// Default constructor
        PlayerRequestClassResponse() { }

        /// Construction from a IClass
        PlayerRequestClassResponse(int team, int model, Vector3 spawn, float angle)
        {
            TeamID = team;
            ModelID = model;
            Spawn = spawn;
            ZAngle = angle;
        }

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT8>(Selectable);
            bs.read<NetworkBitStreamValueType::UINT8>(TeamID);
            bs.read<NetworkBitStreamValueType::UINT32>(ModelID);
            bs.read<NetworkBitStreamValueType::UINT8>(Unknown1);
            bs.read<NetworkBitStreamValueType::VEC3>(Spawn);
            bs.read<NetworkBitStreamValueType::FLOAT>(ZAngle);
            bs.read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32>(Weapons);
            return bs.read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32>(Ammos);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Selectable));
            bs.write(NetworkBitStreamValue::UINT8(TeamID));
            bs.write(NetworkBitStreamValue::UINT32(ModelID));
            bs.write(NetworkBitStreamValue::UINT8(Unknown1));
            bs.write(NetworkBitStreamValue::VEC3(Spawn));
            bs.write(NetworkBitStreamValue::FLOAT(ZAngle));
            bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Weapons));
            bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Ammos));
        }
    };

    struct SetSpawnInfo final : NetworkPacketBase<68> {
        uint8_t TeamID;
        uint32_t ModelID;
        uint8_t Unknown1;
        Vector3 Spawn;
        float ZAngle;
        NetworkArray<uint32_t> Weapons;
        NetworkArray<uint32_t> Ammos;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(TeamID));
            bs.write(NetworkBitStreamValue::UINT32(ModelID));
            bs.write(NetworkBitStreamValue::UINT8(Unknown1));
            bs.write(NetworkBitStreamValue::VEC3(Spawn));
            bs.write(NetworkBitStreamValue::FLOAT(ZAngle));
            bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Weapons));
            bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Ammos));
        }
    };

    struct PlayerRequestSpawn final : NetworkPacketBase<129> {
        bool read(INetworkBitStream& bs)
        {
            return true;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct PlayerRequestSpawnResponse final : NetworkPacketBase<129> {
        uint32_t Allow;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::UINT32>(Allow);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(Allow));
        }
    };

    struct ImmediatelySpawnPlayer final : NetworkPacketBase<129> {
        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(2));
        }
    };
}
}
