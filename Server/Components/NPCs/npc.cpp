/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "npc.hpp"
#include <netcode.hpp>
#include "npcs_main.hpp"

NPC::NPC(NPCComponent* component, IPlayer* playerPtr)
{
	npcComponent = component;
	player = playerPtr;
	currentPosition = { 0.0f, 0.0f, 3.5f }; // TODO: remove me, only used for basic sync/move testing!
	moving = false;

	// Fake a spawn
	// TODO: REMOVE
	NetworkBitStream bs;
	bs.writeUINT16(0);

	npcComponent->npcNetwork.rpcInEventDispatcher.stopAtFalse(128, [this, &bs](SingleNetworkInEventHandler* handler)
	{
		bs.resetReadPointer();
		return handler->onReceive(*player, bs);
	});
}

bool NPC::moveTo(Vector3 position, NPCMoveSpeed moveSpeed)
{
	// TODO: handle moveSpeed here and set appropriate speed, currently hardcoded to jogging.
	float distance = glm::distance(currentPosition, position);
	Vector3 newVelocity;

	if (!(distance <= 0.0f))
	{
		newVelocity = (position - currentPosition) / distance;
	}
	
	newVelocity *= (0.56444f / 100.0f);
	velocity = newVelocity;

	targetPosition = position;
	moving = true;
	moveStart = Time::now();
	lastMove = Time::now();
	return true;
}

void NPC::sendFootSync()
{
	NetworkBitStream foot;

	foot.writeUINT16(0);
	foot.writeUINT16(moving ? static_cast<uint16_t>(Key::ANALOG_UP) : 0); // TODO: remove
	foot.writeUINT16(0);
	foot.writeVEC3(currentPosition);
	foot.writeVEC4(Vector4(0.960891485f, 0.0f, 0.0f, 0.276925147f)); // writeGTAQuat won't work on bitstream read, so just use this value for now (got from a basic foot packet)
	foot.writeCompressedPercentPair(Vector2(100, 0));
	foot.writeUINT8(0);
	foot.writeUINT8(0);
	foot.writeVEC3(Vector3(0.0f, 0.0f, 0.0f));
	foot.writeVEC3(Vector3(0.0f, 0.0f, 0.0f));
	foot.writeUINT16(0);
	foot.writeUINT16(0);
	foot.writeUINT16(0);
	foot.writeUINT16(0);

	uint8_t type = 207;
	npcComponent->getNetwork();
	const bool res = npcComponent->npcNetwork.inEventDispatcher.stopAtFalse([this, type, &foot](NetworkInEventHandler* handler)
		{
			return handler->onReceivePacket(*player, type, foot);
		});

	if (res)
	{
		npcComponent->npcNetwork.packetInEventDispatcher.stopAtFalse(type, [this, &foot](SingleNetworkInEventHandler* handler)
			{
				return handler->onReceive(*player, foot);
			});
	}
}

void NPC::move(TimePoint now)
{
	int difference = (duration_cast<Milliseconds>(now.time_since_epoch()) - duration_cast<Milliseconds>(lastMove.time_since_epoch())).count();
	if (difference > 0)
	{
		currentPosition += velocity * static_cast<float>(difference);
		lastMove = now;
	}
}

void NPC::tick(Microseconds elapsed, TimePoint now)
{
	// TODO: move the sync rates to vars
	// TODO: only update if we have an actual state and are spawned (ATM we're automatically spawned for testing)
	// TODO: handle reaching the destination
	if (moving)
	{
		move(now);
	}

	if ((now - lastUpdate).count() > *npcComponent->core->getConfig().getInt("network.on_foot_sync_rate"))
	{
		sendFootSync();
		lastUpdate = now;
	}
}