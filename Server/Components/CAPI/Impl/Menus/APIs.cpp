/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Menu_Create, objectPtr(StringCharPtr title, uint32_t columns, float x, float y, float column1Width, float column2Width, int* id))
{
	IMenusComponent* component = ComponentManager::Get()->menus;
	if (component)
	{
		IMenu* menu = component->create(title, { x, y }, uint8_t(columns), column1Width, column2Width);
		if (menu)
		{
			*id = menu->getID();
			return menu;
		}
	}
	return nullptr;
}

OMP_CAPI(Menu_Destroy, bool(objectPtr menu))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	ComponentManager::Get()->menus->release(menu_->getID());
	return true;
}

OMP_CAPI(Menu_FromID, objectPtr(int menuid))
{
	IMenusComponent* component = ComponentManager::Get()->menus;
	if (component)
	{
		return component->get(menuid);
	}
	return nullptr;
}

OMP_CAPI(Menu_GetID, int(objectPtr menu))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, INVALID_MENU_ID);
	return menu_->getID();
}

OMP_CAPI(Menu_AddItem, int(objectPtr menu, uint8_t column, StringCharPtr text))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, INVALID_MENU_ID);
	auto index = menu_->addCell(text, column);
	return index;
}

OMP_CAPI(Menu_SetColumnHeader, bool(objectPtr menu, uint8_t column, StringCharPtr headerTitle))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	menu_->setColumnHeader(headerTitle, column);
	return true;
}

OMP_CAPI(Menu_ShowForPlayer, bool(objectPtr menu, objectPtr player))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	menu_->showForPlayer(*player_);
	return true;
}

OMP_CAPI(Menu_HideForPlayer, bool(objectPtr menu, objectPtr player))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	menu_->hideForPlayer(*player_);
	return true;
}

OMP_CAPI(Menu_Disable, bool(objectPtr menu))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	menu_->disable();
	return true;
}

OMP_CAPI(Menu_DisableRow, bool(objectPtr menu, uint8_t row))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	menu_->disableRow(row);
	return true;
}

OMP_CAPI(Player_GetMenu, objectPtr(objectPtr player))
{
	IMenusComponent* component = ComponentManager::Get()->menus;
	if (component)
	{
		POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
		IPlayerMenuData* menuData = queryExtension<IPlayerMenuData>(player_);
		if (menuData)
		{
			return component->get(menuData->getMenuID());
		}
	}
	return nullptr;
}

OMP_CAPI(Menu_IsValid, bool(objectPtr menu))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	if (!menus->get(menu_->getID()))
		return false;
	return true;
}

OMP_CAPI(Menu_IsDisabled, bool(objectPtr menu))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	auto disabled = !menu_->isEnabled();
	return disabled;
}

OMP_CAPI(Menu_IsRowDisabled, bool(objectPtr menu, int row))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	auto disabled = !menu_->isRowEnabled(MenuRow(row));
	return disabled;
}

OMP_CAPI(Menu_GetColumns, int(objectPtr menu))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, 0);
	auto columns = menu_->getColumnCount();
	return columns;
}

OMP_CAPI(Menu_GetItems, int(objectPtr menu, int column))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, 0);
	auto rows = menu_->getRowCount(MenuColumn(column));
	return rows;
}

OMP_CAPI(Menu_GetPos, bool(objectPtr menu, float* x, float* y))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	auto pos = menu_->getPosition();
	*x = pos.x;
	*y = pos.y;
	return true;
}

OMP_CAPI(Menu_GetColumnWidth, bool(objectPtr menu, float* column1Width, float* column2Width))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	auto widths = menu_->getColumnWidths();
	*column1Width = widths.x;
	*column2Width = widths.y;
	return true;
}

OMP_CAPI(Menu_GetColumnHeader, bool(objectPtr menu, int column, OutputStringViewPtr header))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	auto result = menu_->getColumnHeader(MenuColumn(column));
	SET_CAPI_STRING_VIEW(header, result);
	return true;
}

OMP_CAPI(Menu_GetItem, bool(objectPtr menu, int column, int row, OutputStringViewPtr item))
{
	POOL_ENTITY_RET(menus, IMenu, menu, menu_, false);
	auto result = menu_->getCell(MenuColumn(column), MenuRow(row));
	SET_CAPI_STRING_VIEW(item, result);
	return true;
}
