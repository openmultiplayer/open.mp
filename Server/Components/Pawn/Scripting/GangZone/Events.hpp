/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

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

	void onPlayerEnterPlayerGangZone(IPlayer& player, IGangZone& zone) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerEnterPlayerGangZone", DefaultReturnValue_True, player.getID(), zone.getID());
    }

    void onPlayerLeavePlayerGangZone(IPlayer& player, IGangZone& zone) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerLeavePlayerGangZone", DefaultReturnValue_True, player.getID(), zone.getID());
    }
};
