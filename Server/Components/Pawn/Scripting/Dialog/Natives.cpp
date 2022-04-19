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

SCRIPT_API(ShowPlayerDialog, bool(IPlayer& player, int dialogId, int style, const std::string& caption, const std::string& info, const std::string& button1, const std::string& button2))
{
    IPlayerDialogData* dialog = queryExtension<IPlayerDialogData>(player);
    if (dialog) {
        dialog->show(player, dialogId, DialogStyle(style), caption, info, button1, button2);
        return true;
    }
    return false;
}

SCRIPT_API_FAILRET(GetPlayerDialog, INVALID_DIALOG_ID, int(IPlayer& player))
{
    IPlayerDialogData* dialog = queryExtension<IPlayerDialogData>(player);
    if (dialog) {
        return dialog->getActiveID();
    }
    return FailRet;
}

SCRIPT_API(GetPlayerDialogID, int(IPlayer& player))
{
    IPlayerDialogData* data = queryExtension<IPlayerDialogData>(player);
    if (data) {
        return data->getActiveID();
    }
    return -1;
}
