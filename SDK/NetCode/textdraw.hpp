#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct PlayerShowTextDraw final : NetworkPacketBase<134> {
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
        NetworkString Text;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            uint8_t flags = UseBox | (Alignment << 1) | (Proportional << 4);
            bs.write(NetworkBitStreamValue::UINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID));
            bs.write(NetworkBitStreamValue::UINT8(flags));
            bs.write(NetworkBitStreamValue::VEC2(LetterSize));
            bs.write(NetworkBitStreamValue::UINT32(LetterColour.ABGR()));
            bs.write(NetworkBitStreamValue::VEC2(TextSize));
            bs.write(NetworkBitStreamValue::UINT32(BoxColour.ABGR()));
            bs.write(NetworkBitStreamValue::UINT8(Shadow));
            bs.write(NetworkBitStreamValue::UINT8(Outline));
            bs.write(NetworkBitStreamValue::UINT32(BackColour.ABGR()));
            bs.write(NetworkBitStreamValue::UINT8(Style));
            bs.write(NetworkBitStreamValue::UINT8(Selectable));
            bs.write(NetworkBitStreamValue::VEC2(Position));
            bs.write(NetworkBitStreamValue::UINT16(Model));
            bs.write(NetworkBitStreamValue::VEC3(Rotation));
            bs.write(NetworkBitStreamValue::FLOAT(Zoom));
            bs.write(NetworkBitStreamValue::INT16(Color1));
            bs.write(NetworkBitStreamValue::INT16(Color2));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_16(Text));
        }
    };

    struct PlayerHideTextDraw final : NetworkPacketBase<135> {
        bool PlayerTextDraw;
        int TextDrawID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID));
        }
    };

    struct PlayerTextDrawSetString final : NetworkPacketBase<105> {
        bool PlayerTextDraw;
        int TextDrawID;
        NetworkString Text;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_16(Text));
        }
    };

    struct PlayerBeginTextDrawSelect final : NetworkPacketBase<83> {
        Colour Col;
        bool Enable;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::BIT(Enable));
            bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
        }
    };

    struct OnPlayerSelectTextDraw final : NetworkPacketBase<83> {
        bool PlayerTextDraw;
        bool Invalid;
        int TextDrawID;

        bool read(INetworkBitStream& bs)
        {
            bool res = bs.read<NetworkBitStreamValueType::UINT16>(TextDrawID);
            Invalid = TextDrawID == INVALID_TEXTDRAW;
            if (!Invalid) {
                PlayerTextDraw = TextDrawID >= GLOBAL_TEXTDRAW_POOL_SIZE;
                if (PlayerTextDraw) {
                    TextDrawID -= GLOBAL_TEXTDRAW_POOL_SIZE;
                }
            }
            return res;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };
}
}