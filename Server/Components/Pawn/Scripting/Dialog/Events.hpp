#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct DialogEvents : public PlayerDialogEventHandler, public Singleton<DialogEvents> {
	void onDialogResponse(IPlayer& player, uint16_t dialogId, DialogResponse response, uint16_t listItem, StringView inputText) override {
		// Keep going in side scripts first until it hits a return value of 1, then call event in entry script 
		PawnManager::Get()->CallAllInSidesFirstWhile0("OnDialogResponse", player.getID(), dialogId, int(response), listItem, inputText);
	}
};
