#include "sdk.hpp"
#include "Server/Components/Checkpoints/checkpoints.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(SetPlayerCheckpoint, bool(IPlayer& player, Vector3 position, float size))
{
	IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
	if (cp) {
		cp->setPosition(position);
		cp->setRadius(size / 2.0f);
		cp->enable();
		return true;
	}
	return false;
}

SCRIPT_API(DisablePlayerCheckpoint, bool(IPlayer& player))
{
	IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
	if (cp) {
		cp->disable();
		return true;
	}
	return false;
}

SCRIPT_API(IsPlayerInCheckpoint, bool(IPlayer& player))
{
	IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
	if (cp->isEnabled()) {
		return cp->isPlayerInside();
	}
	return false;
}

SCRIPT_API(SetPlayerRaceCheckpoint, bool(IPlayer& player, int type, Vector3 position, Vector3 nextPosition, float size))
{
	IPlayerRaceCheckpointData* cp = player.queryData<IPlayerRaceCheckpointData>();
	if (cp && type >= 0 && type <= 8) {
		cp->setType(RaceCheckpointType(type));
		cp->setPosition(position);
		cp->setNextPosition(nextPosition);
		cp->setRadius(size / 2.0f);
		cp->enable();
		return true;
	}
	return false;
}

SCRIPT_API(DisablePlayerRaceCheckpoint, bool(IPlayer& player))
{
	IPlayerRaceCheckpointData* cp = player.queryData<IPlayerRaceCheckpointData>();
	if (cp) {
		cp->disable();
		return true;
	}
	return false;
}

SCRIPT_API(IsPlayerInRaceCheckpoint, bool(IPlayer& player))
{
	IPlayerRaceCheckpointData* cp = player.queryData<IPlayerRaceCheckpointData>();
	if (cp && cp->getType() != RaceCheckpointType::RACE_NONE && cp->isEnabled()) {
		return cp->isPlayerInside();
	}
	return false;
}
