#include "sdk.hpp"
#include <iostream>
#include "Server/Components/Dialogs/dialogs.hpp"
#include "../Types.hpp"

SCRIPT_API(ShowPlayerDialog, int(IPlayer& player, int dialogId, int style, const std::string& caption, const std::string& info, const std::string& button1, const std::string& button2))
{
	IPlayerDialogData* dialog = player.queryData<IPlayerDialogData>();
	if (dialog) {
		dialog->show(player, dialogId, DialogStyle(style), caption, info, button1, button2);
		return true;
	}
	return false;
}
