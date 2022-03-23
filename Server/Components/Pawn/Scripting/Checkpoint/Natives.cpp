#include "../Types.hpp"
#include "Server/Components/Checkpoints/checkpoints.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(SetPlayerCheckpoint, bool(IPlayer& player, Vector3 position, float size))
{
    IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
    if (playerCheckpointData) {
        IPlayerStandardCheckpointData& cp = playerCheckpointData->getStandardCheckpoint();
        cp.setPosition(position);
        cp.setRadius(size); //samp native receives radius not diameter
        cp.enable();
        return true;
    }
    return false;
}

SCRIPT_API(DisablePlayerCheckpoint, bool(IPlayer& player))
{
    IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
    if (playerCheckpointData) {
        IPlayerStandardCheckpointData& cp = playerCheckpointData->getStandardCheckpoint();
        cp.disable();
        return true;
    }
    return false;
}

SCRIPT_API(IsPlayerInCheckpoint, bool(IPlayer& player))
{
    IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
    if (playerCheckpointData) {
        IPlayerStandardCheckpointData& cp = playerCheckpointData->getStandardCheckpoint();
        if (cp.isEnabled()) {
            return cp.isPlayerInside();
        }
    }
    return false;
}

SCRIPT_API(SetPlayerRaceCheckpoint, bool(IPlayer& player, int type, Vector3 position, Vector3 nextPosition, float size))
{
    IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
    if (playerCheckpointData) {
        IPlayerRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
        if (type >= 0 && type <= 8) {
            cp.setType(RaceCheckpointType(type));
            cp.setPosition(position);
            cp.setNextPosition(nextPosition);
            cp.setRadius(size); // samp native receives radius unlike standard checkpoints
            cp.enable();
            return true;
        }
    }
    return false;
}

SCRIPT_API(DisablePlayerRaceCheckpoint, bool(IPlayer& player))
{
    IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
    if (playerCheckpointData) {
        IPlayerRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
        cp.disable();
        return true;
    }
    return false;
}

SCRIPT_API(IsPlayerInRaceCheckpoint, bool(IPlayer& player))
{
    IPlayerCheckpointData* playerCheckpointData = queryExtension<IPlayerCheckpointData>(player);
    if (playerCheckpointData) {
        IPlayerRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
        if (cp.getType() != RaceCheckpointType::RACE_NONE && cp.isEnabled()) {
            return cp.isPlayerInside();
        }
    }
    return false;
}
