/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "../../Manager/Manager.hpp"
#include "sdk.hpp"

struct MenuEvents : public MenuEventHandler, public Singleton<MenuEvents>
{
	void onPlayerSelectedMenuRow(IPlayer& player, MenuRow row) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerSelectedMenuRow", DefaultReturnValue_True, player.getID(), int(row));
	}

	void onPlayerExitedMenu(IPlayer& player) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerExitedMenu", DefaultReturnValue_True, player.getID());
	}
};
