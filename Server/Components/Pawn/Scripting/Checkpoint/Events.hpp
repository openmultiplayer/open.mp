#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct CheckpointEvents : public PlayerCheckpointEventHandler, public Singleton<CheckpointEvents> {
    void onPlayerEnterCheckpoint(IPlayer& player) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnPlayerEnterCheckpoint", DefaultReturnValue_True, player.getID());
    }

    void onPlayerLeaveCheckpoint(IPlayer& player) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnPlayerLeaveCheckpoint", DefaultReturnValue_True, player.getID());
    }

    void onPlayerEnterRaceCheckpoint(IPlayer& player) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnPlayerEnterRaceCheckpoint", DefaultReturnValue_True, player.getID());
    }

    void onPlayerLeaveRaceCheckpoint(IPlayer& player) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnPlayerLeaveRaceCheckpoint", DefaultReturnValue_True, player.getID());
    }
};
