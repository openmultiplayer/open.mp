#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct PlayerEvents : public PlayerEventHandler, public Singleton<PlayerEvents>{
	void onConnect(IPlayer & player) override {
		PawnManager::Get()->CallInSidesWhile1("OnPlayerConnect", player.getID());
		PawnManager::Get()->CallInEntry("OnPlayerConnect", player.getID());
	}

	void onSpawn(IPlayer& player) override {
		PawnManager::Get()->CallInSidesWhile1("OnPlayerSpawn", player.getID());
		PawnManager::Get()->CallInEntry("OnPlayerSpawn", player.getID());
	}

	bool onCommandText(IPlayer& player, StringView cmdtext) override {
		cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerCommandText", player.getID(), cmdtext);
		if (!ret) {
			ret = PawnManager::Get()->CallInEntry("OnPlayerCommandText", player.getID(), cmdtext);
		}
		return ret;
	}

	void onKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) override {
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerKeyStateChange", player.getID(), newKeys, oldKeys);
	}
};
