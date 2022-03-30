#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(CreateMenu, int(const std::string& title, uint32_t columns, Vector2 position, float col1Width, float col2Width))
{
    IMenusComponent* component = PawnManager::Get()->menus;
    if (component) {
        IMenu* menu = component->create(title, position, columns, col1Width, col2Width);
        if (menu) {
            return menu->getID();
        }
    }
    return -1;
}

SCRIPT_API(DestroyMenu, bool(IMenu& menu))
{
    PawnManager::Get()->menus->release(menu.getID());
    return true;
}

SCRIPT_API(AddMenuItem, int(IMenu& menu, uint8_t column, const std::string& text))
{
    return menu.addMenuItem(text, column);
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

SCRIPT_API_FAILRET(GetPlayerMenu, INVALID_MENU_ID, int(IPlayer& player))
{
    IPlayerMenuData* menuData = queryExtension<IPlayerMenuData>(player);
    if (menuData) {
        return menuData->getMenuID();
    }
    return FailRet;
}

SCRIPT_API(IsValidMenu, bool(IMenu* menu))
{
    return menu != nullptr;
}

SCRIPT_API(IsMenuDisabled, bool(IMenu& menu))
{
    return !menu.isEnabled();
}

SCRIPT_API(IsMenuRowDisabled, bool(IMenu& menu, int row))
{
    return !menu.isRowEnabled(row);
}

SCRIPT_API(GetMenuColumns, int(IMenu& menu))
{
    return menu.getColumnCount();
}

SCRIPT_API(GetMenuItems, int(IMenu& menu, int column))
{
    return menu.getItemCount(column);
}

SCRIPT_API(GetMenuPos, bool(IMenu& menu, Vector2& pos))
{
    pos = menu.getPosition();
    return true;
}

SCRIPT_API(GetMenuColumnWidth, bool(IMenu& menu, float& Column1, float& Column2))
{
    const Vector2 widths = menu.getColumnsWidth();
    Column1 = widths.x;
    Column2 = widths.y;
    return true;
}

SCRIPT_API(GetMenuColumnHeader, bool(IMenu& menu, int column, OutputOnlyString& header))
{
    header = menu.getColumnHeader(column);
    return true;
}

SCRIPT_API(GetMenuItem, bool(IMenu& menu, int column, int row, OutputOnlyString& item))
{
    item = menu.getItem(column, row);
    return true;
}
