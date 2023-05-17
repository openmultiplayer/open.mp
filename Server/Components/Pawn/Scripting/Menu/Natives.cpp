/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>
#include "../../format.hpp"

SCRIPT_API(CreateMenu, int(cell const* format, uint32_t columns, Vector2 position, float column1Width, float column2Width))
{
	IMenusComponent* component = PawnManager::Get()->menus;
	if (component)
	{
		AmxStringFormatter title(format, GetAMX(), GetParams(), 6); // Not 5
		IMenu* menu = component->create(title, position, columns, column1Width, column2Width);
		if (menu)
		{
			return menu->getID();
		}
	}
	return -1; // INVALID_MENU_ID is 255 but samp returns -1 if CreateMenu fails
}

SCRIPT_API(DestroyMenu, bool(IMenu& menu))
{
	PawnManager::Get()->menus->release(menu.getID());
	return true;
}

SCRIPT_API(AddMenuItem, int(IMenu& menu, uint8_t column, cell const* format))
{
	AmxStringFormatter text(format, GetAMX(), GetParams(), 3);
	return menu.addCell(text, column);
}

SCRIPT_API(SetMenuColumnHeader, bool(IMenu& menu, uint8_t column, cell const* format))
{
	AmxStringFormatter headerTitle(format, GetAMX(), GetParams(), 3);
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
	menu.disable();
	return true;
}

SCRIPT_API(DisableMenuRow, bool(IMenu& menu, uint8_t row))
{
	menu.disableRow(row);
	return true;
}

SCRIPT_API_FAILRET(GetPlayerMenu, INVALID_MENU_ID, int(IPlayer& player))
{
	IPlayerMenuData* menuData = queryExtension<IPlayerMenuData>(player);
	if (menuData)
	{
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
	return menu.getRowCount(column);
}

SCRIPT_API(GetMenuPos, bool(IMenu& menu, Vector2& pos))
{
	pos = menu.getPosition();
	return true;
}

SCRIPT_API(GetMenuColumnWidth, bool(IMenu& menu, float& column1Width, float& column2Width))
{
	const Vector2 widths = menu.getColumnWidths();
	column1Width = widths.x;
	column2Width = widths.y;
	return true;
}

SCRIPT_API(GetMenuColumnHeader, bool(IMenu& menu, int column, OutputOnlyString& header))
{
	header = menu.getColumnHeader(column);
	return true;
}

SCRIPT_API(GetMenuItem, bool(IMenu& menu, int column, int row, OutputOnlyString& cell))
{
	cell = menu.getCell(column, row);
	return true;
}
