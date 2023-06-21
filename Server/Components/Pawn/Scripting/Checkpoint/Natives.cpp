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

SCRIPT_API(SetPlayerCheckpoint, bool(IPlayerCheckpointData& data, Vector3 centrePosition, float radius))
{
	ICheckpointData& cp = data.getCheckpoint();
	cp.setPosition(centrePosition);
	cp.setRadius(radius); // samp native receives radius not diameter
	cp.enable();
	return true;
}

SCRIPT_API(DisablePlayerCheckpoint, bool(IPlayerCheckpointData& data))
{
	ICheckpointData& cp = data.getCheckpoint();
	cp.disable();
	return true;
}

SCRIPT_API(IsPlayerInCheckpoint, bool(IPlayerCheckpointData& data))
{
	ICheckpointData& cp = data.getCheckpoint();
	if (cp.isEnabled())
	{
		return cp.isPlayerInside();
	}
	return false;
}

SCRIPT_API(SetPlayerRaceCheckpoint, bool(IPlayerCheckpointData& data, int type, Vector3 centrePosition, Vector3 nextPosition, float radius))
{
	IRaceCheckpointData& cp = data.getRaceCheckpoint();
	if (type >= 0 && type <= 8)
	{
		cp.setType(RaceCheckpointType(type));
		cp.setPosition(centrePosition);
		cp.setNextPosition(nextPosition);
		cp.setRadius(radius); // samp native receives radius unlike standard checkpoints
		cp.enable();
		return true;
	}
	return false;
}

SCRIPT_API(DisablePlayerRaceCheckpoint, bool(IPlayerCheckpointData& data))
{
	IRaceCheckpointData& cp = data.getRaceCheckpoint();
	cp.disable();
	return true;
}

SCRIPT_API(IsPlayerInRaceCheckpoint, bool(IPlayerCheckpointData& data))
{
	IRaceCheckpointData& cp = data.getRaceCheckpoint();
	if (cp.getType() != RaceCheckpointType::RACE_NONE && cp.isEnabled())
	{
		return cp.isPlayerInside();
	}
	return false;
}

SCRIPT_API(IsPlayerCheckpointActive, bool(IPlayerCheckpointData& data))
{
	return data.getCheckpoint().isEnabled();
}

SCRIPT_API(GetPlayerCheckpoint, bool(IPlayerCheckpointData& data, Vector3& centrePosition, float& radius))
{
	const ICheckpointData& cp = data.getCheckpoint();
	centrePosition = cp.getPosition();
	radius = cp.getRadius();
	return true;
}

SCRIPT_API(IsPlayerRaceCheckpointActive, bool(IPlayerCheckpointData& data))
{
	return data.getRaceCheckpoint().isEnabled();
}

SCRIPT_API(GetPlayerRaceCheckpoint, bool(IPlayerCheckpointData& data, Vector3& centrePosition, Vector3& nextPosition, float& radius))
{
	const IRaceCheckpointData& cp = data.getRaceCheckpoint();
	centrePosition = cp.getPosition();
	nextPosition = cp.getNextPosition();
	radius = cp.getRadius();
	return true;
}
