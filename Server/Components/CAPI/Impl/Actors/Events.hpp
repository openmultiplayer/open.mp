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
struct ActorEvents : public ActorEventHandler, public Singleton<ActorEvents<PRIORITY>>
{
	void onPlayerGiveDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerGiveDamageActor", EventReturnHandler::None, &player, &actor, amount, int(weapon), int(part));
	}

	void onActorStreamIn(IActor& actor, IPlayer& forPlayer) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onActorStreamIn", EventReturnHandler::None, &actor, &forPlayer);
	}

	void onActorStreamOut(IActor& actor, IPlayer& forPlayer) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onActorStreamOut", EventReturnHandler::None, &actor, &forPlayer);
	}
};
