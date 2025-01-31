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

struct NPCEvents : public NPCEventHandler, public Singleton<NPCEvents>
{
	void onNPCFinishMove(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCFinishMove", DefaultReturnValue_True, npc.getID());
	}

	void onNPCCreate(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCCreate", DefaultReturnValue_True, npc.getID());
	}

	void onNPCDestroy(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCDestroy", DefaultReturnValue_True, npc.getID());
	}

	void onNPCWeaponStateChange(INPC& npc, PlayerWeaponState newState, PlayerWeaponState oldState) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCWeaponStateChange", DefaultReturnValue_True, npc.getID(), int(newState), int(oldState));
	}

	bool onNPCTakeDamage(INPC& npc, IPlayer& damager, float damage, uint8_t weapon, BodyPart bodyPart) override
	{
		auto result = !!PawnManager::Get()->CallAllInEntryFirst("OnNPCTakeDamage", DefaultReturnValue_True, npc.getID(), damager.getID(), damage, weapon, int(bodyPart));
		return result;
	}

	void onNPCDeath(INPC& npc, IPlayer* killer, int reason) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCDeath", DefaultReturnValue_True, npc.getID(), killer ? killer->getID() : INVALID_PLAYER_ID, reason);
	}
};
