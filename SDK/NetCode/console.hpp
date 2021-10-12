#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace Packet {
    struct PlayerRconCommand final : NetworkPacketBase<201> {
        NetworkString cmd;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_32>(cmd);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };
}
}
