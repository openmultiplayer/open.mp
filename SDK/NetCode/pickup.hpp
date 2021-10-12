#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct PlayerCreatePickup final : NetworkPacketBase<95> {
        int PickupID;
        int Model;
        int Type;
        Vector3 Position;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::INT32(PickupID));
            bs.write(NetworkBitStreamValue::INT32(Model));
            bs.write(NetworkBitStreamValue::INT32(Type));
            bs.write(NetworkBitStreamValue::VEC3(Position));
        }
    };

    struct PlayerDestroyPickup final : NetworkPacketBase<63> {
        int PickupID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::INT32(PickupID));
        }
    };

    struct OnPlayerPickUpPickup final : NetworkPacketBase<131> {
        int PickupID;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::INT32>(PickupID);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };
}
}