#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct PlayerInitMenu final : NetworkPacketBase<76> {
        uint8_t MenuID;
        bool HasTwoColumns;
        NetworkString Title;
        Vector2 Position;
        float Col1Width;
        float Col2Width;
        bool MenuEnabled;
        StaticArray<bool, MAX_MENU_ITEMS> RowEnabled;
        StaticArray<NetworkString, 2> ColumnHeaders;
        StaticArray<uint8_t, 2> ColumnItemCount;
        StaticArray<StaticArray<NetworkString, MAX_MENU_ITEMS>, 2> MenuItems;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(MenuID));
            bs.write(NetworkBitStreamValue::UINT32(HasTwoColumns));

            // Menu title is a fixed size string
            String menuTitleFixed(Title);
            menuTitleFixed.resize(MAX_MENU_TEXT_LENGTH);
            bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(menuTitleFixed)));

            bs.write(NetworkBitStreamValue::VEC2(Position));
            bs.write(NetworkBitStreamValue::FLOAT(Col1Width));

            // Only send this when menu has two columns
            if (HasTwoColumns) {
                bs.write(NetworkBitStreamValue::FLOAT(Col2Width));
            }

            bs.write(NetworkBitStreamValue::UINT32(MenuEnabled));
            for (bool isRowEnabled : RowEnabled) {
                bs.write(NetworkBitStreamValue::UINT32(isRowEnabled));
            }

            // Get first column data
            uint8_t firstColumnItemCount = ColumnItemCount.at(0);
            NetworkString firstColumnHeader = ColumnHeaders.at(0);
            StaticArray<NetworkString, MAX_MENU_ITEMS> firstColumnItems = MenuItems.at(0);

            // Send first column header as a fixed string
            String firstColumnHeaderFixed(firstColumnHeader);
            firstColumnHeaderFixed.resize(MAX_MENU_TEXT_LENGTH);
            bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(firstColumnHeaderFixed)));

            bs.write(NetworkBitStreamValue::UINT8(firstColumnItemCount));
            for (uint8_t i = 0; i < firstColumnItemCount; i++) {
                // Send items/rows of first column as fixed size string
                String itemTextFixed(firstColumnItems.at(i));
                itemTextFixed.resize(MAX_MENU_TEXT_LENGTH);
                bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(itemTextFixed)));
            }

            if (HasTwoColumns) {
                // Get second column data
                uint8_t secondColumnItemCount = ColumnItemCount.at(1);
                NetworkString secondColumnHeader = ColumnHeaders.at(1);
                StaticArray<NetworkString, MAX_MENU_ITEMS> secondColumnItems = MenuItems.at(1);

                // Send second second header as a fixed string
                String secondColumnHeaderFixed(secondColumnHeader);
                secondColumnHeaderFixed.resize(MAX_MENU_TEXT_LENGTH);
                bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(secondColumnHeaderFixed)));

                bs.write(NetworkBitStreamValue::UINT8(secondColumnItemCount));
                for (uint8_t i = 0; i < secondColumnItemCount; i++) {
                    // Send items/rows of second column as fixed size string
                    String itemTextFixed(secondColumnItems.at(i));
                    itemTextFixed.resize(MAX_MENU_TEXT_LENGTH);
                    bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(itemTextFixed)));
                }
            }
        }
    };

    struct PlayerShowMenu final : NetworkPacketBase<77> {
        uint8_t MenuID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(MenuID));
        }
    };

    struct PlayerHideMenu final : NetworkPacketBase<78> {
        uint8_t MenuID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(MenuID));
        }
    };

    struct OnPlayerSelectedMenuRow final : NetworkPacketBase<132> {
        uint8_t MenuRow;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::UINT8>(MenuRow);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct OnPlayerExitedMenu final : NetworkPacketBase<140> {
        bool read(INetworkBitStream& bs)
        {
            return true;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };
}
}