#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct GangZoneEvents : public GangZoneEventHandler, public Singleton<GangZoneEvents> {
    void onPlayerEnterGangZone(IPlayer& player, IGangZone& zone) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerEnterGangZone", DefaultReturnValue_True, player.getID(), zone.getID());
    }

    void onPlayerLeaveGangZone(IPlayer& player, IGangZone& zone) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerLeaveGangZone", DefaultReturnValue_True, player.getID(), zone.getID());
    }
};
