/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"

SCRIPT_API(NPC_Create, int(const std::string& name))
{
	if (PawnManager::Get()->npcs == nullptr)
	{
		return 65535;
	}
	return PawnManager::Get()->npcs->create(name)->getID();
}

SCRIPT_API(NPC_GoTo, int(int npcID, Vector3 pos))
{
	auto npcComponent = PawnManager::Get()->npcs;

	if (npcComponent == nullptr)
	{
		return 0;
	}

	INPC* npc = npcComponent->get(npcID);
	npc->moveTo(pos, NPCMoveSpeed_Jog);
	return 1;
}