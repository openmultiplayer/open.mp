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
	npcComponent_ = component;
	// We created a player instance for it, let's keep a handle of it internally
	player_ = playerPtr;

	// Initial entity values
	Vector3 initialPosition = { 0.0f, 0.0f, 3.5f };
	GTAQuat initialRotation = { 0.960891485f, 0.0f, 0.0f, 0.276925147f };
	moving_ = false;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	moveType_ = NPCMoveType_None;

	// Initial values for foot sync values
	footSync_.LeftRight = 0;
	footSync_.UpDown = 0;
	footSync_.Keys = 0;
	footSync_.Position = initialPosition;
	footSync_.Velocity = velocity_;
	footSync_.Rotation = initialRotation;
	footSync_.WeaponAdditionalKey = 0;
	footSync_.HealthArmour = { 100.0f, 0.0f };
	footSync_.SpecialAction = 0;
	footSync_.AnimationID = 0;
	footSync_.AnimationFlags = 0;
	footSync_.SurfingData.type = PlayerSurfingData::Type::None;
	footSync_.SurfingData.ID = 0;
	footSync_.SurfingData.offset = { 0.0f, 0.0f, 0.0f };
}

Vector3 NPC::getPosition() const
{
	return player_->getPosition();
}

void NPC::setPosition(Vector3 pos)
{
	footSync_.Position = pos;

	// Let it update for all players and internally in open.mp
	sendFootSync();

	if (moving_)
	{
		move(targetPosition_, moveType_);
	}
}

GTAQuat NPC::getRotation() const
{
	return player_->getRotation();
}

void NPC::setRotation(GTAQuat rot)
{
	footSync_.Rotation = rot;

	// Let it update for all players and internally in open.mp
	sendFootSync();

	if (moving_)
	{
		move(targetPosition_, moveType_);
	}
}

int NPC::getVirtualWorld() const
{
	return player_->getVirtualWorld();
}

void NPC::setVirtualWorld(int vw)
{
	player_->setVirtualWorld(vw);
}

void NPC::spawn()
{
	NetworkBitStream requestClassBS;
	NetworkBitStream emptyBS;

	requestClassBS.writeUINT16(0);
	npcComponent_->emulateRPCIn(this, NetCode::RPC::PlayerRequestClass::PacketID, requestClassBS);

	npcComponent_->emulateRPCIn(this, NetCode::RPC::PlayerRequestSpawn::PacketID, emptyBS);
	npcComponent_->emulateRPCIn(this, NetCode::RPC::PlayerSpawn::PacketID, emptyBS);
}

bool NPC::move(Vector3 pos, NPCMoveType moveType)
{
	if (moveType == NPCMoveType_None)
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
	if (moveType == NPCMoveType_Sprint)
	{
		speed = NPC_MOVE_SPEED_SPRINT;
		footSync_.Keys |= Key::SPRINT;
	}
	else if (moveType == NPCMoveType_Jog)
	{
		speed = NPC_MOVE_SPEED_JOG;
	}
	else
	{
		speed = NPC_MOVE_SPEED_WALK;
		footSync_.Keys |= Key::WALK;
	}

	footSync_.UpDown = static_cast<uint16_t>(Key::ANALOG_UP);

	// Calculate velocity to use on tick
	newVelocity *= (speed / 100.0f);
	velocity_ = newVelocity;

	// Calculate front vector and player's facing angle:
	Vector3 front;
	if (!(std::fabs(distance) < DBL_EPSILON))
	{
		front = (pos - position) / distance;
	}

	auto rotation = getRotation().ToEuler();
	rotation.z = utils::getAngleOfLine(front.x, front.y);
	footSync_.Rotation = rotation; // Do this directly, if you use NPC::setRotation it's going to cause recursion

	// Set internal variables
	moveSpeed_ = speed;
	targetPosition_ = pos;
	moving_ = true;
	moveType_ = moveType;
	lastMove_ = Time::now();
	return true;
}

void NPC::stopMove()
{
	moving_ = false;
	moveSpeed_ = 0.0f;
	targetPosition_ = { 0.0f, 0.0f, 0.0f };
	velocity_ = { 0.0f, 0.0f, 0.0f };
	moveType_ = NPCMoveType_None;

	footSync_.Keys &= Key::SPRINT;
	footSync_.Keys &= Key::WALK;
	footSync_.UpDown = 0;
}

void NPC::sendFootSync()
{
	// Only send foot sync if player is spawned
	if (!(player_->getState() == PlayerState_OnFoot || player_->getState() == PlayerState_Driver || player_->getState() == PlayerState_Passenger || player_->getState() == PlayerState_Spawned))
	{
		return;
	}

	NetworkBitStream bs;

	auto& quat = footSync_.Rotation.q;

	bs.writeUINT16(footSync_.LeftRight);
	bs.writeUINT16(footSync_.UpDown);
	bs.writeUINT16(footSync_.Keys);
	bs.writeVEC3(footSync_.Position);
	bs.writeVEC4(Vector4(quat.w, quat.x, quat.y, quat.z));
	bs.writeUINT8(uint8_t(footSync_.HealthArmour.x));
	bs.writeUINT8(uint8_t(footSync_.HealthArmour.y));
	bs.writeUINT8(footSync_.WeaponAdditionalKey);
	bs.writeUINT8(footSync_.SpecialAction);
	bs.writeVEC3(footSync_.Velocity);
	bs.writeVEC3(footSync_.SurfingData.offset);
	bs.writeUINT16(footSync_.SurfingData.ID);
	bs.writeUINT16(footSync_.AnimationID);
	bs.writeUINT16(footSync_.AnimationFlags);

	npcComponent_->emulatePacketIn(this, footSync_.PacketID, bs);
}

void NPC::advance(TimePoint now)
{
	auto position = getPosition();
	Milliseconds difference = duration_cast<Milliseconds>(now.time_since_epoch()) - duration_cast<Milliseconds>(lastMove_.time_since_epoch());
	float remainingDistance = glm::distance(position, targetPosition_);
	Vector3 travelled = velocity_ * static_cast<float>(difference.count());

	if (glm::length(travelled) >= remainingDistance)
	{
		stopMove();
		npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMove, *this);
	}
	else
	{
		position += travelled;
		footSync_.Velocity = velocity_;
	}

	lastMove_ = Time::now();
	footSync_.Position = position; // Do this directly, if you use NPC::setPosition it's going to cause recursion
}

void NPC::tick(Microseconds elapsed, TimePoint now)
{
	static auto footSyncRate = npcComponent_->getCore()->getConfig().getInt("network.on_foot_sync_rate");

	// Only process the NPC if it is spawned
	if (player_ && (player_->getState() == PlayerState_OnFoot || player_->getState() == PlayerState_Driver || player_->getState() == PlayerState_Passenger || player_->getState() == PlayerState_Spawned))
	{
		if (moving_)
		{
			advance(now);
		}

		if ((now - lastUpdate_).count() > *footSyncRate)
		{
			sendFootSync();
			lastUpdate_ = now;
		}
	}
}
