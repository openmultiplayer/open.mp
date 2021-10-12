#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct TextDrawEvents : public TextDrawEventHandler, public Singleton<TextDrawEvents> {
    void onTextDrawClick(IPlayer& player, ITextDraw& td) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerClickTextDraw", player.getID(), td.getID());
        if (!ret) {
            PawnManager::Get()->CallInEntry("OnPlayerClickTextDraw", DefaultReturnValue_False, player.getID(), td.getID());
        }
    }

    void onPlayerTextDrawClick(IPlayer& player, IPlayerTextDraw& td) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerClickPlayerTextDraw", player.getID(), td.getID());
        if (!ret) {
            PawnManager::Get()->CallInEntry("OnPlayerClickPlayerTextDraw", DefaultReturnValue_False, player.getID(), td.getID());
        }
    }
};
