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

	void onNPCConnect(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCConnect", DefaultReturnValue_True, npc.getID());
	}

	void onNPCDisconnect(INPC& npc) override
	{
		PawnManager::Get()->CallAllInEntryFirst("OnNPCDisconnect", DefaultReturnValue_True, npc.getID());
	}
};
