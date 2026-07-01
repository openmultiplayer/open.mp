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

struct CheckpointEvents : public PlayerCheckpointEventHandler, public Singleton<CheckpointEvents>
{
	void onPlayerEnterCheckpoint(IPlayer& player) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerEnterCheckpoint", DefaultReturnValue_True, player.getID());
	}

	void onPlayerLeaveCheckpoint(IPlayer& player) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerLeaveCheckpoint", DefaultReturnValue_True, player.getID());
	}

	void onPlayerEnterRaceCheckpoint(IPlayer& player) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerEnterRaceCheckpoint", DefaultReturnValue_True, player.getID());
	}

	void onPlayerLeaveRaceCheckpoint(IPlayer& player) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerLeaveRaceCheckpoint", DefaultReturnValue_True, player.getID());
	}
};
