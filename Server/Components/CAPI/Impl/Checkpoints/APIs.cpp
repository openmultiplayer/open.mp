/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Checkpoint_Set, bool(objectPtr player, float x, float y, float z, float radius))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerCheckpointData)
	{
		ICheckpointData& cp = playerCheckpointData->getCheckpoint();
		cp.setPosition({ x, y, z });
		cp.setRadius(radius);
		cp.enable();
		return true;
	}
	return false;
}

OMP_CAPI(Checkpoint_Disable, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerCheckpointData)
	{
		ICheckpointData& cp = playerCheckpointData->getCheckpoint();
		cp.disable();
		return true;
	}
	return false;
}

OMP_CAPI(Checkpoint_IsPlayerIn, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerCheckpointData)
	{
		ICheckpointData& cp = playerCheckpointData->getCheckpoint();
		if (cp.isEnabled())
		{
			bool isIn = cp.isPlayerInside();
			return isIn;
		}
	}
	return false;
}

OMP_CAPI(Checkpoint_IsActive, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerData)
	{
		bool active = playerData->getCheckpoint().isEnabled();
		return active;
	}
	return false;
}

OMP_CAPI(Checkpoint_Get, bool(objectPtr player, float* x, float* y, float* z, float* radius))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerData)
	{
		const ICheckpointData& data = playerData->getCheckpoint();
		*x = data.getPosition().x;
		*y = data.getPosition().y;
		*z = data.getPosition().z;
		*radius = data.getRadius();
		return true;
	}
	return false;
}

OMP_CAPI(RaceCheckpoint_Set, bool(objectPtr player, int type, float x, float y, float z, float nextX, float nextY, float nextZ, float radius))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerCheckpointData)
	{
		IRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
		if (type >= 0 && type <= 8)
		{
			cp.setType(RaceCheckpointType(type));
			cp.setPosition({ x, y, z });
			cp.setNextPosition({ nextX, nextY, nextZ });
			cp.setRadius(radius);
			cp.enable();
			return true;
		}
	}
	return false;
}

OMP_CAPI(RaceCheckpoint_Disable, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerCheckpointData)
	{
		IRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
		cp.disable();
		return true;
	}
	return false;
}

OMP_CAPI(RaceCheckpoint_IsPlayerIn, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerCheckpointData)
	{
		IRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
		if (cp.getType() != RaceCheckpointType::RACE_NONE && cp.isEnabled())
		{
			bool isIn = cp.isPlayerInside();
			return isIn;
		}
	}
	return false;
}

OMP_CAPI(RaceCheckpoint_IsActive, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerData)
	{
		bool active = playerData->getCheckpoint().isEnabled();
		return active;
	}
	return false;
}

OMP_CAPI(RaceCheckpoint_Get, bool(objectPtr player, float* x, float* y, float* z, float* nextX, float* nextY, float* nextZ, float* radius))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player_);
	if (playerData)
	{
		const IRaceCheckpointData& data = playerData->getRaceCheckpoint();
		*x = data.getPosition().x;
		*y = data.getPosition().y;
		*z = data.getPosition().z;
		*nextX = data.getNextPosition().x;
		*nextY = data.getNextPosition().y;
		*nextZ = data.getNextPosition().z;
		*radius = data.getRadius();
		return true;
	}
	return false;
}
