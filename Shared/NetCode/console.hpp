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
namespace Packet
{
	struct PlayerRconCommand : NetworkPacketBase<201, NetworkPacketType::Packet, OrderingChannel_Unordered>
	{
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
