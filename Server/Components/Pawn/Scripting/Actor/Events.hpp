#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct PlayerEvents : public ActorEventHandler, public Singleton<PlayerEvents> {
	void onPlayerDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) override {
		PawnManager::Get()->CallAllInSidesFirstWhile0("OnPlayerGiveDamageActor", player.getID(), actor.getID(), amount, weapon, int(part));
	}

	void onActorStreamIn(IActor& actor, IPlayer& forPlayer) override {
		PawnManager::Get()->CallAllInSidesFirst("OnActorStreamIn", actor.getID(), forPlayer.getID());
	}

	void onActorStreamOut(IActor& actor, IPlayer& forPlayer) override {
		PawnManager::Get()->CallAllInSidesFirst("OnActorStreamOut", actor.getID(), forPlayer.getID());
	}
};
