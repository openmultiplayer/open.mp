#pragma once

#include <network.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode {
namespace RPC {
    struct PlayerShowTextDraw : NetworkPacketBase<134, NetworkPacketType::RPC> {
        bool PlayerTextDraw;
        int TextDrawID;
        bool UseBox;
        int Alignment;
        bool Proportional;
        Vector2 LetterSize;
        Colour LetterColour;
        Vector2 TextSize;
        Colour BoxColour;
        int Shadow;
        int Outline;
        Colour BackColour;
        int Style;
        bool Selectable;
        Vector2 Position;
        int Model;
        Vector3 Rotation;
        float Zoom;
        int Color1;
        int Color2;
        HybridString<256> Text;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            uint8_t flags = UseBox | (Alignment << 1) | (Proportional << 4);
            bs.writeUINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID);
            bs.writeUINT8(flags);
            bs.writeVEC2(LetterSize);
            bs.writeUINT32(LetterColour.ABGR());
            bs.writeVEC2(TextSize);
            bs.writeUINT32(BoxColour.ABGR());
            bs.writeUINT8(Shadow);
            bs.writeUINT8(Outline);
            bs.writeUINT32(BackColour.ABGR());
            bs.writeUINT8(Style);
            bs.writeUINT8(Selectable);
            bs.writeVEC2(Position);
            bs.writeUINT16(Model);
            bs.writeVEC3(Rotation);
            bs.writeFLOAT(Zoom);
            bs.writeINT16(Color1);
            bs.writeINT16(Color2);
            bs.writeDynStr16(Text);
        }
    };

    struct PlayerHideTextDraw : NetworkPacketBase<135, NetworkPacketType::RPC> {
        bool PlayerTextDraw;
        int TextDrawID;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID);
        }
    };

    struct PlayerTextDrawSetString : NetworkPacketBase<105, NetworkPacketType::RPC> {
        bool PlayerTextDraw;
        int TextDrawID;
        HybridString<256> Text;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID);
            bs.writeDynStr16(Text);
        }
    };

    struct PlayerBeginTextDrawSelect : NetworkPacketBase<83, NetworkPacketType::RPC> {
        Colour Col;
        bool Enable;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeBIT(Enable);
            bs.writeUINT32(Col.RGBA());
        }
    };

    struct OnPlayerSelectTextDraw : NetworkPacketBase<83, NetworkPacketType::RPC> {
        bool PlayerTextDraw;
        bool Invalid;
        int TextDrawID;

        bool read(NetworkBitStream& bs)
        {
            bool res = bs.readUINT16(TextDrawID);
            Invalid = TextDrawID == INVALID_TEXTDRAW;
            if (!Invalid) {
                PlayerTextDraw = TextDrawID >= GLOBAL_TEXTDRAW_POOL_SIZE;
                if (PlayerTextDraw) {
                    TextDrawID -= GLOBAL_TEXTDRAW_POOL_SIZE;
                }
            }
            return res;
        }

        void write(NetworkBitStream& bs) const
        {
        }
    };
}
}
