#pragma once

#include <network.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode {
namespace RPC {
    struct ShowGangZone : NetworkPacketBase<108, NetworkPacketType::RPC> {
        int ID;
        Vector2 Min;
        Vector2 Max;
        Colour Col;

        void write(NetworkBitStream& bs) const
        {
            bs.writeINT16(ID);
            bs.writeVEC2(Min);
            bs.writeVEC2(Max);
            bs.writeUINT32(Col.ABGR());
        }
    };

    struct HideGangZone : NetworkPacketBase<120, NetworkPacketType::RPC> {
        int ID;

        void write(NetworkBitStream& bs) const
        {
            bs.writeINT16(ID);
        }
    };

    struct FlashGangZone : NetworkPacketBase<121, NetworkPacketType::RPC> {
        int ID;
        Colour Col;

        void write(NetworkBitStream& bs) const
        {
            bs.writeINT16(ID);
            bs.writeUINT32(Col.ABGR());
        }
    };

    struct StopFlashGangZone : NetworkPacketBase<85, NetworkPacketType::RPC> {
        int ID;

        void write(NetworkBitStream& bs) const
        {
            bs.writeINT16(ID);
        }
    };
}
}
