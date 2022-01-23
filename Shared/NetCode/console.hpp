#pragma once

#include <network.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode {
namespace Packet {
    struct PlayerRconCommand : NetworkPacketBase<201, NetworkPacketType::Packet> {
        HybridString<64> cmd;

        bool read(NetworkBitStream& bs)
        {
            return bs.readDynStr32(cmd);
        }

        void write(NetworkBitStream& bs) const
        {
        }
    };
}
}
