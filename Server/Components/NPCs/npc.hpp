/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Server/Components/NPCs/npcs.hpp>

struct NPCComponent;

class NPC : public INPC
{
public:
	NPC(NPCComponent* npcComponent, IPlayer* playerPtr);

	bool moveTo(Vector3 position, NPCMoveSpeed moveSpeed) override;

	void sendFootSync();

	void tick(Microseconds elapsed, TimePoint now);

	int getID() override
	{
		if (player)
		{
			return player->getID();
		}
		return 65535;
	}

	void move(TimePoint now);

private:
	NPCComponent* npcComponent;
	IPlayer* player; // The NPC's player pointer.
	TimePoint lastUpdate;
	TimePoint lastMove;
	TimePoint moveStart;
	Vector3 currentPosition;
	Vector3 targetPosition;
	Vector3 velocity;
	bool moving;
};