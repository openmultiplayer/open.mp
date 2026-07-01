/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "../../Manager/Manager.hpp"
#include "sdk.hpp"

struct GangZoneEvents : public GangZoneEventHandler, public Singleton<GangZoneEvents>
{
	void onPlayerEnterGangZone(IPlayer& player, IGangZone& zone) override
	{
		auto pawn = PawnManager::Get();
		if (zone.getLegacyPlayer() == nullptr)
		{
			pawn->CallAllInEntryFirst("OnPlayerEnterGangZone", DefaultReturnValue_True, player.getID(), pawn->gangzones->toLegacyID(zone.getID()));
		}
		else if (auto data = queryExtension<IPlayerGangZoneData>(player))
		{
			pawn->CallAllInEntryFirst("OnPlayerEnterPlayerGangZone", DefaultReturnValue_True, player.getID(), data->toLegacyID(zone.getID()));
		}
	}

	void onPlayerLeaveGangZone(IPlayer& player, IGangZone& zone) override
	{
		auto pawn = PawnManager::Get();
		if (zone.getLegacyPlayer() == nullptr)
		{
			pawn->CallAllInEntryFirst("OnPlayerLeaveGangZone", DefaultReturnValue_True, player.getID(), pawn->gangzones->toLegacyID(zone.getID()));
		}
		else if (auto data = queryExtension<IPlayerGangZoneData>(player))
		{
			pawn->CallAllInEntryFirst("OnPlayerLeavePlayerGangZone", DefaultReturnValue_True, player.getID(), data->toLegacyID(zone.getID()));
		}
	}

	void onPlayerClickGangZone(IPlayer& player, IGangZone& zone) override
	{
		auto pawn = PawnManager::Get();
		if (zone.getLegacyPlayer() == nullptr)
		{
			pawn->CallAllInEntryFirst("OnPlayerClickGangZone", DefaultReturnValue_True, player.getID(), pawn->gangzones->toLegacyID(zone.getID()));
		}
		else if (auto data = queryExtension<IPlayerGangZoneData>(player))
		{
			pawn->CallAllInEntryFirst("OnPlayerClickPlayerGangZone", DefaultReturnValue_True, player.getID(), data->toLegacyID(zone.getID()));
		}
	}
};
