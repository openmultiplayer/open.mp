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

struct TextDrawEvents : public TextDrawEventHandler, public Singleton<TextDrawEvents>
{
	virtual bool onPlayerCancelTextDrawSelection(IPlayer& player) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerClickTextDraw", player.getID(), INVALID_TEXTDRAW);
		if (!ret)
		{
			PawnManager::Get()->CallInEntry("OnPlayerClickTextDraw", DefaultReturnValue_False, player.getID(), INVALID_TEXTDRAW);
		}
		// TODO: New callback?
		return true;
	}

	virtual bool onPlayerCancelPlayerTextDrawSelection(IPlayer& player) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerClickPlayerTextDraw", player.getID(), INVALID_TEXTDRAW);
		if (!ret)
		{
			PawnManager::Get()->CallInEntry("OnPlayerClickPlayerTextDraw", DefaultReturnValue_False, player.getID(), INVALID_TEXTDRAW);
		}
		// TODO: New callback?
		return true;
	}

	void onPlayerClickTextDraw(IPlayer& player, ITextDraw& td) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerClickTextDraw", player.getID(), td.getID());
		if (!ret)
		{
			PawnManager::Get()->CallInEntry("OnPlayerClickTextDraw", DefaultReturnValue_False, player.getID(), td.getID());
		}
	}

	void onPlayerClickPlayerTextDraw(IPlayer& player, IPlayerTextDraw& td) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerClickPlayerTextDraw", player.getID(), td.getID());
		if (!ret)
		{
			PawnManager::Get()->CallInEntry("OnPlayerClickPlayerTextDraw", DefaultReturnValue_False, player.getID(), td.getID());
		}
	}
};
