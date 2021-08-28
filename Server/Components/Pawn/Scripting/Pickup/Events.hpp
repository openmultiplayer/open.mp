#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct PickupEvents : public PickupEventHandler, public Singleton<PickupEvents> {
	void onPlayerPickUpPickup(IPlayer& player, IPickup& pickup) override {
		PawnManager::Get()->CallAllInEntryFirst("OnPlayerPickUpPickup", player.getID(), pickup.getID());
	}
};
