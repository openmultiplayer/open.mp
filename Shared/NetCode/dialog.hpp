#pragma once

#include <network.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode {
namespace RPC {
    struct ShowDialog : NetworkPacketBase<61, NetworkPacketType::RPC> {
        int ID;
        uint8_t Style;
        HybridString<32> Title;
        HybridString<32> FirstButton;
        HybridString<32> SecondButton;
        HybridString<256> Info;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT16(ID);
            bs.writeUINT8(Style);
            bs.writeDynStr8(Title);
            bs.writeDynStr8(FirstButton);
            bs.writeDynStr8(SecondButton);
            bs.WriteCompressedStr(Info);
        }
    };

    struct OnPlayerDialogResponse : NetworkPacketBase<62, NetworkPacketType::RPC> {
        uint16_t ID;
        uint8_t Response;
        uint16_t ListItem;
        HybridString<256> Text;

        bool read(NetworkBitStream& bs)
        {
            bs.readUINT16(ID);
            bs.readUINT8(Response);
            bs.readUINT16(ListItem);
            return bs.readDynStr8(Text);
        }

        void write(NetworkBitStream& bs) const
        {
        }
    };
}
}
