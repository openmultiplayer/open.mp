#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct ShowDialog final : NetworkPacketBase<61> {
        int ID;
        uint8_t Style;
        NetworkString Title;
        NetworkString FirstButton;
        NetworkString SecondButton;
        NetworkString Info;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ID));
            bs.write(NetworkBitStreamValue::UINT8(Style));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Title));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(FirstButton));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(SecondButton));
            bs.write(NetworkBitStreamValue::COMPRESSED_STR(Info));
        }
    };

    struct OnPlayerDialogResponse final : NetworkPacketBase<62> {
        uint16_t ID;
        uint8_t Response;
        uint16_t ListItem;
        NetworkString Text;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(ID);
            bs.read<NetworkBitStreamValueType::UINT8>(Response);
            bs.read<NetworkBitStreamValueType::UINT16>(ListItem);
            return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Text);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };
}
}