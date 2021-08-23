#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct CheckpointEvents : public PlayerCheckpointEventHandler, public Singleton<CheckpointEvents> {
	void onPlayerEnterCheckpoint(IPlayer& player) override {
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerEnterCheckpoint", player.getID());
	}

	void onPlayerLeaveCheckpoint(IPlayer& player) override {
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerLeaveCheckpoint", player.getID());
	}

	void onPlayerEnterRaceCheckpoint(IPlayer& player) override {
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerEnterRaceCheckpoint", player.getID());
	}

	void onPlayerLeaveRaceCheckpoint(IPlayer& player) override {
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerLeaveRaceCheckpoint", player.getID());
	}
};
