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
struct CheckpointEvents : public PlayerCheckpointEventHandler, public Singleton<CheckpointEvents<PRIORITY>>
{
	void onPlayerEnterCheckpoint(IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerEnterCheckpoint", EventReturnHandler::None, &player);
	}

	void onPlayerLeaveCheckpoint(IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerLeaveCheckpoint", EventReturnHandler::None, &player);
	}

	void onPlayerEnterRaceCheckpoint(IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerEnterRaceCheckpoint", EventReturnHandler::None, &player);
	}

	void onPlayerLeaveRaceCheckpoint(IPlayer& player) override
	{

		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerLeaveRaceCheckpoint", EventReturnHandler::None, &player);
	}
};
