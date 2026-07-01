/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#pragma once
#include "../ComponentManager.hpp"
#include "sdk.hpp"

template <EventPriorityType PRIORITY>
struct GangZoneEvents : public GangZoneEventHandler, public Singleton<GangZoneEvents<PRIORITY>>
{
	void onPlayerEnterGangZone(IPlayer& player, IGangZone& zone) override
	{
		if (zone.getLegacyPlayer() == nullptr)
		{
			ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerEnterGangZone", EventReturnHandler::None, &player, &zone);
		}
		else if (auto data = queryExtension<IPlayerGangZoneData>(player))
		{
		}
	}

	void onPlayerLeaveGangZone(IPlayer& player, IGangZone& zone) override
	{
		if (zone.getLegacyPlayer() == nullptr)
		{
			ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerLeaveGangZone", EventReturnHandler::None, &player, &zone);
		}
		else if (auto data = queryExtension<IPlayerGangZoneData>(player))
		{
		}
	}

	void onPlayerClickGangZone(IPlayer& player, IGangZone& zone) override
	{
		if (zone.getLegacyPlayer() == nullptr)
		{
			ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerClickGangZone", EventReturnHandler::None, &player, &zone);
		}
		else if (auto data = queryExtension<IPlayerGangZoneData>(player))
		{
		}
	}
};
