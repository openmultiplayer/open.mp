#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct PickupEvents : public PickupEventHandler, public Singleton<PickupEvents> {
    void onPlayerPickUpPickup(IPlayer& player, IPickup& pickup) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerPickUpPickup", DefaultReturnValue_True, player.getID(), pickup.getID());
    }
};
