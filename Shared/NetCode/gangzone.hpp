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

namespace NetCode
{
namespace RPC
{
	struct ShowGangZone : NetworkPacketBase<108, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
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

	struct HideGangZone : NetworkPacketBase<120, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ID;

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT16(ID);
		}
	};

	struct FlashGangZone : NetworkPacketBase<121, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ID;
		Colour Col;

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT16(ID);
			bs.writeUINT32(Col.ABGR());
		}
	};

	struct StopFlashGangZone : NetworkPacketBase<85, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ID;

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT16(ID);
		}
	};
}
}
