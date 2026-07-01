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

struct PickupEvents : public PickupEventHandler, public Singleton<PickupEvents>
{
	void onPlayerPickUpPickup(IPlayer& player, IPickup& pickup) override
	{
		auto pawn = PawnManager::Get();
		if (pickup.getLegacyPlayer() == nullptr)
		{
			pawn->CallAllInEntryFirst("OnPlayerPickUpPickup", DefaultReturnValue_True, player.getID(), pawn->pickups->toLegacyID(pickup.getID()));
		}
		else if (auto data = queryExtension<IPlayerPickupData>(player))
		{
			pawn->CallAllInEntryFirst("OnPlayerPickUpPlayerPickup", DefaultReturnValue_True, player.getID(), data->toLegacyID(pickup.getID()));
		}
	}
};
