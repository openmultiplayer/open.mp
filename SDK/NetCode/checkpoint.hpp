#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct SetCheckpoint final : NetworkPacketBase<107> {
        Vector3 position;
        float size;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC3(position));
            bs.write(NetworkBitStreamValue::FLOAT(size));
        }
    };

    struct DisableCheckpoint final : NetworkPacketBase<37> {
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const { }
    };

    struct SetRaceCheckpoint final : NetworkPacketBase<38> {
        uint8_t type;
        Vector3 position;
        Vector3 nextPosition;
        float size;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(type));
            bs.write(NetworkBitStreamValue::VEC3(position));
            bs.write(NetworkBitStreamValue::VEC3(nextPosition));
            bs.write(NetworkBitStreamValue::FLOAT(size));
        }
    };

    struct DisableRaceCheckpoint final : NetworkPacketBase<39> {
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const { }
    };
}
}