#pragma once

#include <network.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode {
namespace RPC {
    struct PlayerCreatePickup : NetworkPacketBase<95, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        int PickupID;
        int Model;
        int Type;
        Vector3 Position;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeINT32(PickupID);
            bs.writeINT32(Model);
            bs.writeINT32(Type);
            bs.writeVEC3(Position);
        }
    };

    struct PlayerDestroyPickup : NetworkPacketBase<63, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        int PickupID;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeINT32(PickupID);
        }
    };

    struct OnPlayerPickUpPickup : NetworkPacketBase<131, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        int PickupID;

        bool read(NetworkBitStream& bs)
        {
            return bs.readINT32(PickupID);
        }

        void write(NetworkBitStream& bs) const
        {
        }
    };
}
}
