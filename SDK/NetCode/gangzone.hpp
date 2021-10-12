#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct ShowGangZone final : NetworkPacketBase<108> {
        int ID;
        Vector2 Min;
        Vector2 Max;
        Colour Col;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ID));
            bs.write(NetworkBitStreamValue::VEC2(Min));
            bs.write(NetworkBitStreamValue::VEC2(Max));
            bs.write(NetworkBitStreamValue::UINT32(Col.ABGR()));
        }
    };

    struct HideGangZone final : NetworkPacketBase<120> {
        int ID;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ID));
        }
    };

    struct FlashGangZone final : NetworkPacketBase<121> {
        int ID;
        Colour Col;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ID));
            bs.write(NetworkBitStreamValue::UINT32(Col.ABGR()));
        }
    };

    struct StopFlashGangZone final : NetworkPacketBase<85> {
        int ID;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ID));
        }
    };
}
}