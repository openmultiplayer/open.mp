/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/pool_impl.hpp>
#include <Server/Components/Menus/menus.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

struct PlayerMenuData final : public IPlayerMenuData
{
private:
	uint8_t menuId = INVALID_MENU_ID;

public:
	uint8_t getMenuID() const override
	{
		return menuId;
	}

	void setMenuID(uint8_t id) override
	{
		menuId = id;
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
		menuId = INVALID_MENU_ID;
	}
};

class Menu final : public IMenu, public PoolIDProvider, public NoCopy
{
private:
	String title;
	uint8_t columnCount;
	Vector2 pos;
	float column1Width;
	float column2Width;
	bool enabled_;
	StaticArray<bool, MAX_MENU_ITEMS> rowEnabled;
	StaticArray<String, 2> columnHeaders;
	StaticArray<uint8_t, 2> columnItemCount;
	StaticArray<StaticArray<String, MAX_MENU_ITEMS>, 2> columnMenuItems;

	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> initedFor_;

public:
	void removeFor(int pid, IPlayer& player)
	{
		if (initedFor_.valid(pid))
		{
			initedFor_.remove(pid, player);
		}
	}

	Menu(StringView title, Vector2 position, uint8_t columns, float col1Width, float col2Width)
		: title(String(title))
		, columnCount(columns)
		, pos(position)
		, column1Width(col1Width)
		, column2Width(col2Width)
		, enabled_(true)
		, columnHeaders { "" }
		, columnItemCount { 0 }
		, columnMenuItems { { { "" } } }
	{
		rowEnabled.fill(true);
	}

	void setColumnHeader(StringView header, MenuColumn column) override
	{
		if (column > columnCount)
		{
			return;
		}

		columnHeaders.at(column) = String(header);
	}

	int addCell(StringView itemText, MenuColumn column) override
	{
		if (column > columnCount)
		{
			return INVALID_MENU_ITEM_ID;
		}

		uint8_t itemCount = columnItemCount.at(column);

		if (itemCount >= MAX_MENU_ITEMS)
		{
			return INVALID_MENU_ITEM_ID;
		}

		const int res = itemCount;

		columnMenuItems.at(column).at(itemCount) = String(itemText);
		itemCount++;
		columnItemCount.at(column) = itemCount;

		initedFor_.clear();
		return res;
	}

	void disableRow(MenuRow row) override
	{
		if (row >= MAX_MENU_ITEMS)
		{
			return;
		}

		rowEnabled.at(row) = false;
		initedFor_.clear();
	}

	bool isRowEnabled(MenuRow row) const override
	{
		if (row >= MAX_MENU_ITEMS)
		{
			return false;
		}

		return rowEnabled.at(row);
	}

	void disable() override
	{
		enabled_ = false;
		initedFor_.clear();
	}

	bool isEnabled() const override
	{
		return enabled_;
	}

	const Vector2& getPosition() const override
	{
		return pos;
	}

	int getColumnCount() const override
	{
		return columnCount;
	}

	int getRowCount(MenuColumn column) const override
	{
		if (column > columnCount)
		{
			return 0;
		}
		return columnItemCount.at(column);
	}

	Vector2 getColumnWidths() const override
	{
		return { column1Width, column2Width };
	}

	const StringView getColumnHeader(MenuColumn column) const override
	{
		if (column > columnCount)
		{
			return StringView();
		}

		return columnHeaders[column];
	}

	const StringView getCell(MenuColumn column, MenuRow row) const override
	{
		if (column > columnCount)
		{
			return StringView();
		}

		if (row >= MAX_MENU_ITEMS)
		{
			return StringView();
		}

		return columnMenuItems[column][row];
	}

	void initForPlayer(IPlayer& player) override
	{
		NetCode::RPC::PlayerInitMenu playerInitMenu;
		playerInitMenu.MenuID = poolID;
		playerInitMenu.HasTwoColumns = columnCount > 1;
		playerInitMenu.Title = title;
		playerInitMenu.Position = pos;
		playerInitMenu.Col1Width = column1Width;
		playerInitMenu.Col2Width = column2Width;
		playerInitMenu.MenuEnabled = enabled_;
		playerInitMenu.RowEnabled = rowEnabled;

		for (int i = 0; i < columnHeaders.size(); i++)
		{
			playerInitMenu.ColumnHeaders.at(i) = StringView(columnHeaders.at(i));
		}

		playerInitMenu.ColumnItemCount = columnItemCount;

		for (int column = 0; column < columnMenuItems.size(); column++)
		{
			StaticArray<String, MAX_MENU_ITEMS> singleColumnItems = columnMenuItems.at(column);
			for (int row = 0; row < singleColumnItems.size(); row++)
			{
				playerInitMenu.MenuItems.at(column).at(row) = StringView(singleColumnItems.at(row));
			}
		}

		playerInitMenu.MenuItems[0] = {
			StringView(columnMenuItems[0][0]), StringView(columnMenuItems[0][1]), StringView(columnMenuItems[0][2]),
			StringView(columnMenuItems[0][3]), StringView(columnMenuItems[0][4]), StringView(columnMenuItems[0][5]),
			StringView(columnMenuItems[0][6]), StringView(columnMenuItems[0][7]), StringView(columnMenuItems[0][8]),
			StringView(columnMenuItems[0][9]), StringView(columnMenuItems[0][10]), StringView(columnMenuItems[0][11])
		};

		playerInitMenu.MenuItems[1] = {
			StringView(columnMenuItems[1][0]), StringView(columnMenuItems[1][1]), StringView(columnMenuItems[1][2]),
			StringView(columnMenuItems[1][3]), StringView(columnMenuItems[1][4]), StringView(columnMenuItems[1][5]),
			StringView(columnMenuItems[1][6]), StringView(columnMenuItems[1][7]), StringView(columnMenuItems[1][8]),
			StringView(columnMenuItems[1][9]), StringView(columnMenuItems[1][10]), StringView(columnMenuItems[1][11])
		};

		PacketHelper::send(playerInitMenu, player);

		initedFor_.add(player.getID(), player);
	}

	void showForPlayer(IPlayer& player) override
	{
		if (!initedFor_.valid(player.getID()))
		{
			initForPlayer(player);
		}
		NetCode::RPC::PlayerShowMenu playerShowMenu;
		playerShowMenu.MenuID = poolID;
		PacketHelper::send(playerShowMenu, player);

		IPlayerMenuData* data = queryExtension<IPlayerMenuData>(player);
		if (data)
		{
			data->setMenuID(poolID);
		}
	}

	void hideForPlayer(IPlayer& player) override
	{
		IPlayerMenuData* data = queryExtension<IPlayerMenuData>(player);

		if (!data || data->getMenuID() != poolID)
		{
			return;
		}

		NetCode::RPC::PlayerHideMenu playerHideMenu;
		playerHideMenu.MenuID = poolID;
		PacketHelper::send(playerHideMenu, player);

		data->setMenuID(INVALID_MENU_ID);
	}

	int getID() const override
	{
		return poolID;
	}

	~Menu()
	{
	}
};
