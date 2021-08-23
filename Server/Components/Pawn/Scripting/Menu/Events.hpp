#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct MenuEvents : public MenuEventHandler, public Singleton<MenuEvents> {
	void onPlayerSelectedMenuRow(IPlayer& player, MenuRow row) override {
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerSelectedMenuRow", player.getID(), int(row));
	}

	void onPlayerExitedMenu(IPlayer& player) override {
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerExitedMenu", player.getID());
	}
};
