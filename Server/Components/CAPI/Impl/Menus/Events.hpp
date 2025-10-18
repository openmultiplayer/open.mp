/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#pragma once
#include "../ComponentManager.hpp"
#include "sdk.hpp"

template <EventPriorityType PRIORITY>
struct MenuEvents : public MenuEventHandler, public Singleton<MenuEvents<PRIORITY>>
{
	void onPlayerSelectedMenuRow(IPlayer& player, MenuRow row) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerSelectedMenuRow", EventReturnHandler::None, &player, int(row));
	}

	void onPlayerExitedMenu(IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerExitedMenu", EventReturnHandler::None, &player);
	}
};
