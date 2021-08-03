#include "sdk.hpp"
#include "Server/Components/Checkpoints/checkpoints.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(SetPlayerCheckpoint, bool(IPlayer* player, const Vector3& position, float size))
{
	IPlayerCheckpointData* cp = player->queryData<IPlayerCheckpointData>();
	if (cp) {
		cp->setType(CheckpointType::STANDARD);
		cp->setPosition(position);
		cp->setSize(size);
		cp->enable(*player);
		return true;
	}
	return false;
}

SCRIPT_API(DisablePlayerCheckpoint, bool(IPlayer* player))
{
	IPlayerCheckpointData* cp = player->queryData<IPlayerCheckpointData>();
	if (cp) {
		cp->disable(*player);
		return true;
	}
	return false;
}

SCRIPT_API(IsPlayerInCheckpoint, bool(IPlayer* player))
{
	IPlayerCheckpointData* cp = player->queryData<IPlayerCheckpointData>();
	if (cp && cp->getType() == CheckpointType::STANDARD) {
		return cp->hasPlayerInside();
	}
	return false;
}

SCRIPT_API(SetPlayerRaceCheckpoint, bool(IPlayer* player, int type, const Vector3& position, const Vector3& nextPosition, float size))
{
	IPlayerCheckpointData* cp = player->queryData<IPlayerCheckpointData>();
	if (cp && type >= 0 && type <= 8) {
		cp->setType(CheckpointType(type));
		cp->setPosition(position);
		cp->setNextPosition(nextPosition);
		cp->setSize(size);
		cp->enable(*player);
		return true;
	}
	return false;
}

SCRIPT_API(DisablePlayerRaceCheckpoint, bool(IPlayer* player))
{
	IPlayerCheckpointData* cp = player->queryData<IPlayerCheckpointData>();
	if (cp) {
		cp->disable(*player);
		return true;
	}
	return false;
}

SCRIPT_API(IsPlayerInRaceCheckpoint, bool(IPlayer* player))
{
	IPlayerCheckpointData* cp = player->queryData<IPlayerCheckpointData>();
	if (cp && cp->getType() != CheckpointType::STANDARD) {
		return cp->hasPlayerInside();
	}
	return false;
}
