/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <network.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

namespace NetCode
{
namespace RPC
{
	struct PlayerShowTextDraw : NetworkPacketBase<134, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool PlayerTextDraw;
		int TextDrawID;
		int GlobalTextDrawPoolSize = GLOBAL_TEXTDRAW_POOL_SIZE;
		bool UseBox;
		int Alignment;
		bool Proportional;
		Vector2 LetterSize;
		Colour LetterColour;
		Vector2 TextSize;
		Colour BoxColour;
		int Shadow;
		int Outline;
		Colour BackgroundColour;
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
			bs.writeUINT16(PlayerTextDraw ? GlobalTextDrawPoolSize + TextDrawID : TextDrawID);
			bs.writeUINT8(flags);
			bs.writeVEC2(LetterSize);
			bs.writeUINT32(LetterColour.ABGR());
			bs.writeVEC2(TextSize);
			bs.writeUINT32(BoxColour.ABGR());
			bs.writeUINT8(Shadow);
			bs.writeUINT8(Outline);
			bs.writeUINT32(BackgroundColour.ABGR());
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

	struct PlayerHideTextDraw : NetworkPacketBase<135, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool PlayerTextDraw;
		int TextDrawID;
		int GlobalTextDrawPoolSize = GLOBAL_TEXTDRAW_POOL_SIZE;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerTextDraw ? GlobalTextDrawPoolSize + TextDrawID : TextDrawID);
		}
	};

	struct PlayerTextDrawSetString : NetworkPacketBase<105, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool PlayerTextDraw;
		int TextDrawID;
		int GlobalTextDrawPoolSize = GLOBAL_TEXTDRAW_POOL_SIZE;
		HybridString<256> Text;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerTextDraw ? GlobalTextDrawPoolSize + TextDrawID : TextDrawID);
			bs.writeDynStr16(Text);
		}
	};

	struct PlayerBeginTextDrawSelect : NetworkPacketBase<83, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
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

	struct OnPlayerSelectTextDraw : NetworkPacketBase<83, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool PlayerTextDraw;
		bool Invalid;
		int TextDrawID;
		int GlobalTextDrawPoolSize = GLOBAL_TEXTDRAW_POOL_SIZE;

		bool read(NetworkBitStream& bs)
		{
			bool res = bs.readUINT16(TextDrawID);
			Invalid = TextDrawID == INVALID_TEXTDRAW;
			if (!Invalid)
			{
				PlayerTextDraw = TextDrawID >= GlobalTextDrawPoolSize;
				if (PlayerTextDraw)
				{
					TextDrawID -= GlobalTextDrawPoolSize;
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
