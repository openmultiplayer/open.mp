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

struct ActorEvents : public ActorEventHandler, public Singleton<ActorEvents>
{
	void onPlayerGiveDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) override
	{
		PawnManager::Get()->CallInSidesWhile0("OnPlayerGiveDamageActor", player.getID(), actor.getID(), amount, weapon, int(part));
		PawnManager::Get()->CallInEntry("OnPlayerGiveDamageActor", DefaultReturnValue_False, player.getID(), actor.getID(), amount, weapon, int(part));
	}

	void onActorStreamIn(IActor& actor, IPlayer& forPlayer) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnActorStreamIn", DefaultReturnValue_True, actor.getID(), forPlayer.getID());
	}

	void onActorStreamOut(IActor& actor, IPlayer& forPlayer) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnActorStreamOut", DefaultReturnValue_True, actor.getID(), forPlayer.getID());
	}
};
