#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct MenuEvents : public MenuEventHandler, public Singleton<MenuEvents> {
    void onPlayerSelectedMenuRow(IPlayer& player, MenuRow row) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerSelectedMenuRow", DefaultReturnValue_True, player.getID(), int(row));
    }

    void onPlayerExitedMenu(IPlayer& player) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerExitedMenu", DefaultReturnValue_True, player.getID());
    }
};
