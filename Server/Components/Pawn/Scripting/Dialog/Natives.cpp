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
#include "../../format.hpp"
#include <iostream>

SCRIPT_API(ShowPlayerDialog, bool(IPlayer& player, int dialog, int style, const std::string& title, cell const* format, const std::string& button1, const std::string& button2))
{
	IPlayerDialogData* data = queryExtension<IPlayerDialogData>(player);

	if (!data)
	{
		return false;
	}

	// Put it back to `int` so we can detect and handle this special case.
	// Some old code uses invalid IDs to hide dialogs.
	if (dialog == INVALID_DIALOG_ID)
	{
		static bool warned = false;
		if (!warned)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Invalid dialog ID %d used.  Use `HidePlayerDialog()`.", dialog);
			warned = true;
		}

		data->hide(player);
		// Keep the return value true, some people rely on it.
		return true;
	}

	// And instead mask the ID here.
	AmxStringFormatter body(format, GetAMX(), GetParams(), 7);
	data->show(player, dialog & 0xFFFF, DialogStyle(style), title, body, button1, button2);
	return true;
}

/// Added for fixes.inc compatibility, but as `GetPlayerDialogID` from YSF also exists we don't need
/// two.This one is thus deprecated as then we have `ID` and `Data` suffixes for differentiation and
/// clarity.
///
/// TODO: Add a deprecation warning to this native.
SCRIPT_API_FAILRET(GetPlayerDialog, INVALID_DIALOG_ID, int(IPlayerDialogData& data))
{
	return data.getActiveID();
}

SCRIPT_API_FAILRET(GetPlayerDialogID, INVALID_DIALOG_ID, int(IPlayerDialogData& data))
{
	return data.getActiveID();
}

SCRIPT_API(GetPlayerDialogData, bool(IPlayerDialogData& data, int& style, OutputOnlyString& title, OutputOnlyString& body, OutputOnlyString& button1, OutputOnlyString& button2))
{
	DialogStyle styleVar {};
	StringView titleVar {};
	StringView bodyVar {};
	StringView button1Var {};
	StringView button2Var {};
	int dialogid;
	data.get(dialogid, styleVar, titleVar, bodyVar, button1Var, button2Var);
	style = int(styleVar);
	title = titleVar;
	body = bodyVar;
	button1 = button1Var;
	button2 = button2Var;
	return dialogid != INVALID_DIALOG_ID;
}

SCRIPT_API(HidePlayerDialog, bool(IPlayer& player))
{
	IPlayerDialogData* dialog = queryExtension<IPlayerDialogData>(player);
	if (dialog && dialog->getActiveID() != INVALID_DIALOG_ID)
	{
		dialog->hide(player);
		return true;
	}
	return false;
}
