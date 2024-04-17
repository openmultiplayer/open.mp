/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "npc.hpp"
#include <netcode.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include "../npcs_impl.hpp"

namespace utils
{
float getAngleOfLine(float x, float y)
{
	float angle = atan2(y, x) * (180.0f / M_PI) + 270.0f;
	if (angle >= 360.0f)
	{
		angle -= 360.0f;
	}
	else if (angle < 0.0f)
	{
		angle += 360.0f;
	}
	return angle;
}
}

NPC::NPC(NPCComponent* component, IPlayer* playerPtr)
{
	// Keep a handle of NPC copmonent instance internally
	npcComponent = component;
	// We created a player instance for it, let's keep a handle of it internally
	player = playerPtr;

	// Initial entity values
	Vector3 initialPosition = { 0.0f, 0.0f, 3.5f };
	GTAQuat initialRotation = { 0.960891485f, 0.0f, 0.0f, 0.276925147f };
	moving = false;
	velocity = { 0.0f, 0.0f, 0.0f };
	moveType = NPCMoveType_None;

	// Initial values for foot sync values
	footSync.LeftRight = 0;
	footSync.UpDown = 0;
	footSync.Keys = 0;
	footSync.Position = initialPosition;
	footSync.Velocity = velocity;
	footSync.Rotation = initialRotation;
	footSync.WeaponAdditionalKey = 0;
	footSync.HealthArmour = { 100.0f, 0.0f };
	footSync.SpecialAction = 0;
	footSync.AnimationID = 0;
	footSync.AnimationFlags = 0;
	footSync.SurfingData.type = PlayerSurfingData::Type::None;
	footSync.SurfingData.ID = 0;
	footSync.SurfingData.offset = { 0.0f, 0.0f, 0.0f };
}

Vector3 NPC::getPosition() const
{
	return player->getPosition();
}

void NPC::setPosition(Vector3 pos)
{
	footSync.Position = pos;

	// Let it update for all players and internally in open.mp
	sendFootSync();

	if (moving)
	{
		move(targetPosition, moveType);
	}
}

GTAQuat NPC::getRotation() const
{
	return player->getRotation();
}

void NPC::setRotation(GTAQuat rot)
{
	footSync.Rotation = rot;

	// Let it update for all players and internally in open.mp
	sendFootSync();

	if (moving)
	{
		move(targetPosition, moveType);
	}
}

int NPC::getVirtualWorld() const
{
	return player->getVirtualWorld();
}

void NPC::setVirtualWorld(int vw)
{
	player->setVirtualWorld(vw);
}

void NPC::spawn()
{
	NetworkBitStream requestClassBS;
	NetworkBitStream emptyBS;

	requestClassBS.writeUINT16(0);
	npcComponent->emulateRPCIn(this, NetCode::RPC::PlayerRequestClass::PacketID, requestClassBS);

	npcComponent->emulateRPCIn(this, NetCode::RPC::PlayerRequestSpawn::PacketID, emptyBS);
	npcComponent->emulateRPCIn(this, NetCode::RPC::PlayerSpawn::PacketID, emptyBS);
}

bool NPC::move(Vector3 pos, NPCMoveType moveType)
{
	if (moveType_ == NPCMoveType_None)
	{
		return false;
	}

	// Set up everything to start moving in next tick
	auto position = getPosition();
	float distance = glm::distance(position, pos);
	Vector3 newVelocity;

	if (!(distance <= 0.0f))
	{
		newVelocity = (pos - position) / distance;
	}

	// Determine which speed to use based on moving type
	float speed = 0.0f;
	if (moveType != NPCMoveType_None)
	{
		if (moveType == NPCMoveType_Sprint)
		{
			speed = NPC_MOVE_SPEED_SPRINT;
			footSync.Keys |= Key::SPRINT;
		}
		else if (moveType == NPCMoveType_Jog)
		{
			speed = NPC_MOVE_SPEED_JOG;
		}
		else
		{
			speed = NPC_MOVE_SPEED_WALK;
			footSync.Keys |= Key::WALK;
		}

		footSync.UpDown = static_cast<uint16_t>(Key::ANALOG_UP);
	}

	// Calculate velocity to use on tick
	newVelocity *= (speed / 100.0f);
	velocity = newVelocity;

	// Calculate front vector and player's facing angle:
	Vector3 front;
	if (!(std::fabs(distance) < DBL_EPSILON))
	{
		front = (pos - position) / distance;
	}

	auto rotation = getRotation().ToEuler();
	rotation.z = utils::getAngleOfLine(front.x, front.y);
	footSync.Rotation = rotation; // Do this directly, if you use NPC::setRotation it's going to cause recursion

	// Set internal variables
	moveSpeed = speed;
	targetPosition = pos;
	moving = true;
	lastMove = Time::now();
	return true;
}

void NPC::stopMove()
{
	moving = false;
	moveSpeed = 0.0f;
	targetPosition = { 0.0f, 0.0f, 0.0f };
	velocity = { 0.0f, 0.0f, 0.0f };
	moveType = NPCMoveType_None;

	footSync.Keys &= Key::SPRINT;
	footSync.Keys &= Key::WALK;
	footSync.UpDown = 0;
}

void NPC::sendFootSync()
{
	NetworkBitStream bs;

	auto& quat = footSync.Rotation.q;

	bs.writeUINT16(footSync.LeftRight);
	bs.writeUINT16(footSync.UpDown);
	bs.writeUINT16(footSync.Keys);
	bs.writeVEC3(footSync.Position);
	bs.writeVEC4(Vector4(quat.w, quat.x, quat.y, quat.z));
	bs.writeUINT8(uint8_t(footSync.HealthArmour.x));
	bs.writeUINT8(uint8_t(footSync.HealthArmour.y));
	bs.writeUINT8(footSync.WeaponAdditionalKey);
	bs.writeUINT8(footSync.SpecialAction);
	bs.writeVEC3(footSync.Velocity);
	bs.writeVEC3(footSync.SurfingData.offset);
	bs.writeUINT16(footSync.SurfingData.ID);
	bs.writeUINT16(footSync.AnimationID);
	bs.writeUINT16(footSync.AnimationFlags);

	npcComponent->emulatePacketIn(this, footSync.PacketID, bs);
}

void NPC::advance(TimePoint now)
{
	auto position = getPosition();
	Milliseconds difference = duration_cast<Milliseconds>(now.time_since_epoch()) - duration_cast<Milliseconds>(lastMove.time_since_epoch());
	float remainingDistance = glm::distance(position, targetPosition);
	Vector3 travelled = velocity * static_cast<float>(difference.count());

	if (glm::length(travelled) >= remainingDistance)
	{
		moving = false;
		moveSpeed = 0.0f;
		targetPosition = { 0.0f, 0.0f, 0.0f };

		footSync.Keys &= Key::SPRINT;
		footSync.Keys &= Key::WALK;
		footSync.UpDown = 0;
		npcComponent->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMove, *this);
	}
	else
	{
		position += travelled;
		footSync.Velocity = velocity;
	}

	lastMove = Time::now();
	footSync.Position = position; // Do this directly, if you use NPC::setPosition it's going to cause recursion
}

void NPC::tick(Microseconds elapsed, TimePoint now)
{
	static auto footSyncRate = npcComponent->getCore()->getConfig().getInt("network.on_foot_sync_rate");

	// Only process the NPC if it is spawned
	if (player && (player->getState() == PlayerState_OnFoot || player->getState() == PlayerState_Driver || player->getState() == PlayerState_Passenger || player->getState() == PlayerState_Spawned))
	{
		if (moving)
		{
			advance(now);
		}

		if ((now - lastUpdate).count() > *footSyncRate)
		{
			sendFootSync();
			lastUpdate = now;
		}
	}
}
