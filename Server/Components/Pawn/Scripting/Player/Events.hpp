#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct PlayerEvents : public PlayerEventHandler, public Singleton<PlayerEvents>{
	void onConnect(IPlayer & player) {
		PawnManager::Get()->CallAll("OnPlayerConnect", player.getID());
	}

	void onSpawn(IPlayer& player) {
		PawnManager::Get()->CallAll("OnPlayerSpawn", player.getID());
	}

	bool onCommandText(IPlayer& player, StringView cmdtext) {
		return PawnManager::Get()->CallAll("OnPlayerCommandText", player.getID(), cmdtext);
	}
	void onKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) {
		PawnManager::Get()->CallAll("OnPlayerKeyStateChange", player.getID(), newKeys, oldKeys);
	}
};
