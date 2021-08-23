#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct ClassEvents : public ClassEventHandler, public Singleton<ClassEvents> {
	bool onPlayerRequestClass(IPlayer& player, unsigned int classId) override {
		// only return value of the one in entry script (gamdemode) matters 
		return !!PawnManager::Get()->CallAllInSidesFirst("OnPlayerRequestClass", player.getID(), classId);
	}
};
