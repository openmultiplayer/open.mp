#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct ActorEvents : public ActorEventHandler, public Singleton<ActorEvents> {
    void onPlayerGiveDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) override
    {
        PawnManager::Get()->CallInSidesWhile0("OnPlayerGiveDamageActor", player.getID(), actor.getID(), amount, weapon, int(part));
        PawnManager::Get()->CallInEntry("OnPlayerGiveDamageActor", DefaultReturnValue_False, player.getID(), actor.getID(), amount, weapon, int(part));
    }

    void onActorStreamIn(IActor& actor, IPlayer& forPlayer) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnActorStreamIn", DefaultReturnValue_True, actor.getID(), forPlayer.getID());
    }

    void onActorStreamOut(IActor& actor, IPlayer& forPlayer) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnActorStreamOut", DefaultReturnValue_True, actor.getID(), forPlayer.getID());
    }
};
