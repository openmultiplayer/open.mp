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
	struct PlayerInitMenu : NetworkPacketBase<76, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint8_t MenuID;
		bool HasTwoColumns;
		StaticString<MAX_MENU_TEXT_LENGTH> Title;
		Vector2 Position;
		float Col1Width;
		float Col2Width;
		bool MenuEnabled;
		StaticArray<bool, MAX_MENU_ITEMS> RowEnabled;
		StaticArray<StaticString<MAX_MENU_TEXT_LENGTH>, 2> ColumnHeaders;
		StaticArray<uint8_t, 2> ColumnItemCount;
		StaticArray<StaticArray<StaticString<MAX_MENU_TEXT_LENGTH>, MAX_MENU_ITEMS>, 2> MenuItems;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(MenuID);
			bs.writeUINT32(HasTwoColumns);

			// Menu title is a fixed size string
			bs.writeArray(Title.data());

			bs.writeVEC2(Position);
			bs.writeFLOAT(Col1Width);

			// Only send this when menu has two columns
			if (HasTwoColumns)
			{
				bs.writeFLOAT(Col2Width);
			}

			bs.writeUINT32(MenuEnabled);
			for (bool isRowEnabled : RowEnabled)
			{
				bs.writeUINT32(isRowEnabled);
			}

			// Get first column data
			uint8_t firstColumnItemCount = ColumnItemCount.at(0);
			auto& firstColumnHeader = ColumnHeaders.at(0);
			auto& firstColumnItems = MenuItems.at(0);

			// Send first column header as a fixed string
			bs.writeArray(firstColumnHeader.data());

			bs.writeUINT8(firstColumnItemCount);
			for (uint8_t i = 0; i < firstColumnItemCount; i++)
			{
				// Send items/rows of first column as fixed size string
				bs.writeArray(firstColumnItems.at(i).data());
			}

			if (HasTwoColumns)
			{
				// Get second column data
				uint8_t secondColumnItemCount = ColumnItemCount.at(1);
				auto& secondColumnHeader = ColumnHeaders.at(1);
				auto& secondColumnItems = MenuItems.at(1);

				// Send second second header as a fixed string
				bs.writeArray(secondColumnHeader.data());

				bs.writeUINT8(secondColumnItemCount);
				for (uint8_t i = 0; i < secondColumnItemCount; i++)
				{
					// Send items/rows of second column as fixed size string
					bs.writeArray(secondColumnItems.at(i).data());
				}
			}
		}
	};

	struct PlayerShowMenu : NetworkPacketBase<77, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint8_t MenuID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(MenuID);
		}
	};

	struct PlayerHideMenu : NetworkPacketBase<78, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint8_t MenuID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(MenuID);
		}
	};

	struct OnPlayerSelectedMenuRow : NetworkPacketBase<132, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint8_t MenuRow;

		bool read(NetworkBitStream& bs)
		{
			return bs.readUINT8(MenuRow);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct OnPlayerExitedMenu : NetworkPacketBase<140, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool read(NetworkBitStream& bs)
		{
			return true;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};
}
}
