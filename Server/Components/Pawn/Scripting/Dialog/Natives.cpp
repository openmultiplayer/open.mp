/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "Server/Components/Dialogs/dialogs.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(ShowPlayerDialog, bool(IPlayer& player, int dialog, int style, const std::string& title, const std::string& body, const std::string& button1, const std::string& button2))
{
    IPlayerDialogData* dialog = queryExtension<IPlayerDialogData>(player);
	// Put it back to `int` so we can detect and handle this special case.
	if (dialogId == INVALID_DIALOG_ID)
	{
		// Some old code uses invalid IDs to hide dialogs.
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Invalid dialog ID %d used.  Use `HidePlayerDialog()`.", dialog);
		if (dialog)
		{
			dialog->hide(player);
		}
		return false;
	}
    if (dialog) {
		// And instead mask the ID here.
        dialog->show(player, dialogId & 0xFFFF, DialogStyle(style), title, body, button1, button2);
        return true;
    }
    return false;
}

/// Added for fixes.inc compatibility, but as `GetPlayerDialogID` from YSF also exists we don't need
/// two.This one is thus deprecated as then we have `ID` and `Data` suffixes for differentiation and
/// clarity.
/// 
/// TODO: Add a deprecation warning to this native.
SCRIPT_API_FAILRET(GetPlayerDialog, INVALID_DIALOG_ID, int(IPlayer & player))
{
    IPlayerDialogData* dialog = queryExtension<IPlayerDialogData>(player);
    if (dialog) {
        return dialog->getActiveID();
    }
	return INVALID_DIALOG_ID;
}

SCRIPT_API_FAILRET(GetPlayerDialogID, INVALID_DIALOG_ID, int(IPlayer & player))
{
	IPlayerDialogData * data = queryExtension<IPlayerDialogData>(player);
	if (data) {
		return data->getActiveID();
	}
	return INVALID_DIALOG_ID;
}

SCRIPT_API(GetPlayerDialogData, bool(IPlayer& player, int& style, OutputOnlyString& title, OutputOnlyString& body, OutputOnlyString& button1, OutputOnlyString& button2))
{
    IPlayerDialogData* dialog = queryExtension<IPlayerDialogData>(player);
    if (dialog) {
		DialogStyle styleVar {};
		StringView titleVar {};
		StringView bodyVar {};
		StringView button1Var{};
		StringView button2Var{};
		int dialogid;
        dialog->get(dialogid, styleVar, titleVar, bodyVar, button1Var, button2Var);
		style = int(styleVar);
		title = titleVar;
		body = bodyVar;
		button1 = button1Var;
		button2 = button2Var;
		return dialogid != INVALID_DIALOG_ID;
    }
    return false;
}

SCRIPT_API(HidePlayerDialog, bool(IPlayer& player))
{
    IPlayerDialogData* dialog = queryExtension<IPlayerDialogData>(player);
    if (dialog && dialog->getActiveID() != INVALID_DIALOG_ID) {
        dialog->hide(player);
        return true;
	}
    return false;
}

