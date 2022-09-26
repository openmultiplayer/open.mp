/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "Server/Components/Checkpoints/checkpoints.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(SetPlayerCheckpoint, bool(IPlayer& player, Vector3 centrePosition, float radius))
{
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
	if (playerCheckpointData)
	{
		ICheckpointData& cp = playerCheckpointData->getCheckpoint();
		cp.setPosition(centrePosition);
		cp.setRadius(radius); // samp native receives radius not diameter
		cp.enable();
		return true;
	}
	return false;
}

SCRIPT_API(DisablePlayerCheckpoint, bool(IPlayer& player))
{
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
	if (playerCheckpointData)
	{
		ICheckpointData& cp = playerCheckpointData->getCheckpoint();
		cp.disable();
		return true;
	}
	return false;
}

SCRIPT_API(IsPlayerInCheckpoint, bool(IPlayer& player))
{
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
	if (playerCheckpointData)
	{
		ICheckpointData& cp = playerCheckpointData->getCheckpoint();
		if (cp.isEnabled())
		{
			return cp.isPlayerInside();
		}
	}
	return false;
}

SCRIPT_API(SetPlayerRaceCheckpoint, bool(IPlayer& player, int type, Vector3 centrePosition, Vector3 nextPosition, float radius))
{
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
	if (playerCheckpointData)
	{
		IRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
		if (type >= 0 && type <= 8)
		{
			cp.setType(RaceCheckpointType(type));
			cp.setPosition(centrePosition);
			cp.setNextPosition(nextPosition);
			cp.setRadius(radius); // samp native receives radius unlike standard checkpoints
			cp.enable();
			return true;
		}
	}
	return false;
}

SCRIPT_API(DisablePlayerRaceCheckpoint, bool(IPlayer& player))
{
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
	if (playerCheckpointData)
	{
		IRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
		cp.disable();
		return true;
	}
	return false;
}

SCRIPT_API(IsPlayerInRaceCheckpoint, bool(IPlayer& player))
{
	IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
	if (playerCheckpointData)
	{
		IRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
		if (cp.getType() != RaceCheckpointType::RACE_NONE && cp.isEnabled())
		{
			return cp.isPlayerInside();
		}
	}
	return false;
}

SCRIPT_API(IsPlayerCheckpointActive, bool(IPlayer& player))
{
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player);
	if (playerData)
	{
		return playerData->getCheckpoint().isEnabled();
	}
	return false;
}

SCRIPT_API(GetPlayerCheckpoint, bool(IPlayer& player, Vector3& centrePosition, float& radius))
{
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player);
	if (playerData)
	{
		const ICheckpointData& data = playerData->getCheckpoint();
		centrePosition = data.getPosition();
		radius = data.getRadius();
		return true;
	}
	return false;
}

SCRIPT_API(IsPlayerRaceCheckpointActive, bool(IPlayer& player))
{
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player);
	if (playerData)
	{
		return playerData->getRaceCheckpoint().isEnabled();
	}
	return false;
}

SCRIPT_API(GetPlayerRaceCheckpoint, bool(IPlayer& player, Vector3& centrePosition, Vector3& nextPosition, float& radius))
{
	IPlayerCheckpointData* playerData = queryExtension<IPlayerCheckpointData>(player);
	if (playerData)
	{
		const IRaceCheckpointData& data = playerData->getRaceCheckpoint();
		centrePosition = data.getPosition();
		nextPosition = data.getNextPosition();
		radius = data.getRadius();
		return true;
	}
	return false;
}
