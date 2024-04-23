/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Server/Components/NPCs/npcs.hpp>
#include <netcode.hpp>
#include <Impl/pool_impl.hpp>

class NPCComponent;

class NPC : public INPC, public PoolIDProvider, public NoCopy
{
public:
	NPC(NPCComponent* npcComponent, IPlayer* playerPtr);

	Vector3 getPosition() const override;

	void setPosition(Vector3 position) override;

	GTAQuat getRotation() const override;

	void setRotation(GTAQuat rotation) override;

	int getVirtualWorld() const override;

	void setVirtualWorld(int vw) override;

	void spawn() override;

	bool move(Vector3 position, NPCMoveType moveType) override;

	void stopMove() override;

	void sendFootSync();

	void tick(Microseconds elapsed, TimePoint now);

	void advance(TimePoint now);

	int getID() const override
	{
		return poolID;
	}

	IPlayer* getPlayer() override
	{
		return player_;
	}

	void setPlayer(IPlayer* player)
	{
		player_ = player;
	}

private:
	// The NPC's player pointer.
	IPlayer* player_;
	TimePoint lastUpdate_;

	// Movements
	NPCMoveType moveType_;
	TimePoint lastMove_;
	long long estimatedArrivalTimeNS_;
	TimePoint moveStart_;
	float moveSpeed_;
	Vector3 targetPosition_;
	Vector3 velocity_;
	bool moving_;

	// Packets
	NetCode::Packet::PlayerFootSync footSync_;

	NPCComponent* npcComponent_;
};
