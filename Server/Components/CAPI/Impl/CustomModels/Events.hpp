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
struct CustomModelsEvents : public PlayerModelsEventHandler, public Singleton<CustomModelsEvents<PRIORITY>>
{
	virtual void onPlayerFinishedDownloading(IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerFinishedDownloading", EventReturnHandler::None, &player, player.getVirtualWorld());
	}

	virtual bool onPlayerRequestDownload(IPlayer& player, ModelDownloadType type, uint32_t checksum) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerRequestDownload", EventReturnHandler::StopAtFalse, &player, int(type), int(checksum));
	}
};
