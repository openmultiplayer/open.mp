#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct DialogEvents : public PlayerDialogEventHandler, public Singleton<DialogEvents> {
    void onDialogResponse(IPlayer& player, uint16_t dialogId, DialogResponse response, uint16_t listItem, StringView inputText) override
    {
        PawnManager::Get()->CallInSidesWhile0("OnDialogResponse", player.getID(), dialogId, int(response), listItem, inputText);
        PawnManager::Get()->CallInEntry("OnDialogResponse", DefaultReturnValue_False, player.getID(), dialogId, int(response), listItem, inputText);
    }
};
