#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct PlayerShowTextLabel final : NetworkPacketBase<36> {
        bool PlayerTextLabel;
        int TextLabelID;
        Colour Col;
        Vector3 Position;
        float DrawDistance;
        bool LOS;
        int PlayerAttachID;
        int VehicleAttachID;
        NetworkString Text;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerTextLabel ? TEXT_LABEL_POOL_SIZE + TextLabelID : TextLabelID));
            bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
            bs.write(NetworkBitStreamValue::VEC3(Position));
            bs.write(NetworkBitStreamValue::FLOAT(DrawDistance));
            bs.write(NetworkBitStreamValue::UINT8(LOS));
            bs.write(NetworkBitStreamValue::UINT16(PlayerAttachID));
            bs.write(NetworkBitStreamValue::UINT16(VehicleAttachID));
            bs.write(NetworkBitStreamValue::COMPRESSED_STR(Text));
        }
    };

    struct PlayerHideTextLabel final : NetworkPacketBase<58> {
        bool PlayerTextLabel;
        int TextLabelID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerTextLabel ? TEXT_LABEL_POOL_SIZE + TextLabelID : TextLabelID));
        }
    };
}
}