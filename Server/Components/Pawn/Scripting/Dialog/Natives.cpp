#include "../Types.hpp"
#include "Server/Components/Dialogs/dialogs.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(ShowPlayerDialog, bool(IPlayer& player, int dialogId, int style, const std::string& caption, const std::string& info, const std::string& button1, const std::string& button2))
{
    IPlayerDialogData* dialog = player.queryData<IPlayerDialogData>();
    if (dialog) {
        dialog->show(player, dialogId, DialogStyle(style), caption, info, button1, button2);
        return true;
    }
    return false;
}
