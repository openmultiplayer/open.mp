#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct PlayerEvents : public PlayerEventHandler, public Singleton<PlayerEvents>{
	void onConnect(IPlayer & player) {
		PawnManager::Get()->CallAll("OnPlayerConnect", player.getID());
	}
};
