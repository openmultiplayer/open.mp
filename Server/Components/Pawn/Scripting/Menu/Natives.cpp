#include "sdk.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(CreateMenu, int(const std::string& title, uint32_t columns, const Vector2& position, float col1Width, float col2Width))
{
	IMenusComponent* component = PawnManager::Get()->menus;
	if (component) {
		IMenu* menu = component->create(title, position, columns, col1Width, col2Width);
		if (menu) {
			return menu->getID();
		}
	}
	return INVALID_MENU_ID;
}

SCRIPT_API(DestroyMenu, bool(IMenu& menu))
{
	PawnManager::Get()->menus->release(menu.getID());
	return true;
}

SCRIPT_API(AddMenuItem, bool(IMenu& menu, uint8_t column, const std::string& text))
{
	menu.addMenuItem(text, column);
	return true;
}

SCRIPT_API(SetMenuColumnHeader, bool(IMenu& menu, uint8_t column, const std::string& headerTitle))
{
	menu.setColumnHeader(headerTitle, column);
	return true;
}

SCRIPT_API(ShowMenuForPlayer, bool(IMenu& menu, IPlayer& player))
{
	menu.showForPlayer(player);
	return true;
}

SCRIPT_API(HideMenuForPlayer, bool(IMenu& menu, IPlayer& player))
{
	menu.hideForPlayer(player);
	return true;
}

SCRIPT_API(DisableMenu, bool(IMenu& menu))
{
	menu.disableMenu();
	return true;
}

SCRIPT_API(DisableMenuRow, bool(IMenu& menu, uint8_t row))
{
	menu.disableMenuRow(row);
	return true;
}

SCRIPT_API(GetPlayerMenu, int(IPlayer& player))
{
	IPlayerMenuData* menuData = player.queryData<IPlayerMenuData>();
	return menuData->getMenuID();
}

SCRIPT_API(IsValidMenu, bool(IMenu& menu))
{
	return true;
}
