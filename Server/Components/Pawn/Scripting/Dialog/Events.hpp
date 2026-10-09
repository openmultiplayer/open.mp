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

struct DialogEvents : public PlayerDialogEventHandler, public Singleton<DialogEvents>
{
	void onDialogResponse(IPlayer& player, int dialogId, DialogResponse response, int listItem, StringView inputText) override
	{
		PawnManager::Get()->CallInSidesWhile0("OnDialogResponse", player.getID(), dialogId, int(response), listItem, inputText);
		PawnManager::Get()->CallInEntry("OnDialogResponse", DefaultReturnValue_False, player.getID(), dialogId, int(response), listItem, inputText);
	}
};
