#include <sdk.hpp>
#include <Server/Components/Menus/menus.hpp>
#include <netcode.hpp>

struct PlayerMenuData final : IPlayerMenuData {
	uint8_t menuId = INVALID_MENU_ID;

	uint8_t getMenuId() const override {
		return menuId;
	}

	void setMenuId(uint8_t id) override {
		menuId = id;
	}

	void free() override {
		delete this;
	}
};

struct Menu final : public IMenu, public PoolIDProvider, public NoCopy {
	String title;
	uint8_t columnCount;
	Vector2 pos;
	float col1Width;
	float col2Width;
	bool menuEnabled;
	std::array<bool, MAX_MENU_ITEMS> rowEnabled;
	std::array<String, 2> columnHeaders;
	std::array<uint8_t, 2> columnItemCount;
	std::array<std::array<String, MAX_MENU_ITEMS>, 2> columnMenuItems;

	UniqueIDArray<IPlayer, IPlayerPool::Cnt> initedFor_;

	void setColumnHeader(const String & header, MenuColumn column) override {
		if (column > columnCount) {
			return;
		}

		columnHeaders.at(column) = header;
	}

	void addMenuItem(const String & itemText, MenuColumn column) override {
		if (column > columnCount)
		{
			return;
		}

		uint8_t itemCount = columnItemCount.at(column);

		if (itemCount >= MAX_MENU_ITEMS)
		{
			return;
		}

		columnMenuItems.at(column).at(itemCount) = itemText;
		itemCount++;
		columnItemCount.at(column) = itemCount;

		initedFor_.clear();
	}

	void disableMenuRow(MenuRow row) override {
		if (row >= MAX_MENU_ITEMS)
		{
			return;
		}

		rowEnabled.at(row) = false;
		initedFor_.clear();
	}

	void disableMenu() override {
		menuEnabled = 0;
		initedFor_.clear();
	}

	void initForPlayer(IPlayer & player) override {
		NetCode::RPC::PlayerInitMenu playerInitMenu;
		playerInitMenu.MenuID = poolID;
		playerInitMenu.HasTwoColumns = columnCount >= 1;
		playerInitMenu.Title = title;
		playerInitMenu.Position = pos;
		playerInitMenu.Col1Width = col1Width;
		playerInitMenu.Col2Width = col2Width;
		playerInitMenu.MenuEnabled = menuEnabled;
		playerInitMenu.RowEnabled = rowEnabled;

		for (int i = 0; i < columnHeaders.size(); i++) {
			playerInitMenu.ColumnHeaders.at(i) = columnHeaders.at(i);
		}

		playerInitMenu.ColumnItemCount = columnItemCount;

		for (int column = 0; column < columnMenuItems.size(); column++) {
			std::array<String, MAX_MENU_ITEMS> singleColumnItems = columnMenuItems.at(column);
			for (int row = 0; row < singleColumnItems.size(); row++) {
				playerInitMenu.MenuItems.at(column).at(row) = singleColumnItems.at(row);
			}
		}

		playerInitMenu.MenuItems[0] = {
			columnMenuItems[0][0], columnMenuItems[0][1], columnMenuItems[0][2],
			columnMenuItems[0][3], columnMenuItems[0][4], columnMenuItems[0][5],
			columnMenuItems[0][6], columnMenuItems[0][7], columnMenuItems[0][8],
			columnMenuItems[0][9], columnMenuItems[0][10], columnMenuItems[0][11]
		};

		playerInitMenu.MenuItems[1] = {
			columnMenuItems[1][0], columnMenuItems[1][1], columnMenuItems[1][2],
			columnMenuItems[1][3], columnMenuItems[1][4], columnMenuItems[1][5],
			columnMenuItems[1][6], columnMenuItems[1][7], columnMenuItems[1][8],
			columnMenuItems[1][9], columnMenuItems[1][10], columnMenuItems[1][11]
		};

		player.sendRPC(playerInitMenu);
		initedFor_.add(player.getID(), player);
	}

	void showForPlayer(IPlayer & player) override {
		if (!initedFor_.valid(player.getID())) {
			initForPlayer(player);
		}
		NetCode::RPC::PlayerShowMenu playerShowMenu;
		playerShowMenu.MenuID = poolID;
		player.sendRPC(playerShowMenu);

		IPlayerMenuData * data = player.queryData<IPlayerMenuData>();
		data->setMenuId(poolID);
	}

	void hideForPlayer(IPlayer & player) override {
		NetCode::RPC::PlayerHideMenu playerHideMenu;
		playerHideMenu.MenuID = poolID;
		player.sendRPC(playerHideMenu);

		IPlayerMenuData * data = player.queryData<IPlayerMenuData>();
		data->setMenuId(INVALID_MENU_ID);
	}

	void resetForPlayer(IPlayer & player) override {
		if (initedFor_.valid(player.getID())) {
			initedFor_.remove(player.getID(), player);
		}
	}

	int getID() const override {
		return poolID;
	}

	~Menu() {

	}
};
