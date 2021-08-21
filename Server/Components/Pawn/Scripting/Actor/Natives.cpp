#include "sdk.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(CreateActor, int(int modelid, Vector3 position, float rotation))
{
	IActorsComponent* component = PawnManager::Get()->actors;
	if (component) {
		IActor* actor = component->create(modelid, position, rotation);
		if (actor) {
			return actor->getID();
		}
	}
	return INVALID_ACTOR_ID;
}

SCRIPT_API(DestroyActor, bool(IActor& actor))
{
	PawnManager::Get()->actors->release(actor.getID());
	return true;
}

SCRIPT_API(IsActorStreamedIn, bool(IActor& actor, IPlayer& player))
{
	return actor.isStreamedInForPlayer(player);
}

SCRIPT_API(SetActorVirtualWorld, bool(IActor& actor, int virtualWorld))
{
	actor.setVirtualWorld(virtualWorld);
	return true;
}

SCRIPT_API(GetActorVirtualWorld, int(IActor& actor))
{
	return actor.getVirtualWorld();
}

SCRIPT_API(ApplyActorAnimation, bool(IActor& actor, const std::string& animLib, const std::string& animName, float delta, int loop, int lockX, int lockY, int freeze, int time))
{
	Animation animation(animLib, animName, AnimationTimeData(delta, loop, lockX, lockY, freeze, time));
	actor.applyAnimation(animation);
	return true;
}

SCRIPT_API(ClearActorAnimations, bool(IActor& actor))
{
	actor.clearAnimations();
	return true;
}

SCRIPT_API(SetActorPos, bool(IActor& actor, Vector3 position))
{
	actor.setPosition(position);
	return true;
}

SCRIPT_API(GetActorPos, bool(IActor& actor, Vector3& position))
{
	position = actor.getPosition();
	return true;
}

SCRIPT_API(SetActorFacingAngle, bool(IActor& actor, float angle))
{
	actor.setRotation(Vector3(0.0f, 0.0f, angle));
	return true;
}

SCRIPT_API(GetActorFacingAngle, bool(IActor& actor, float& angle))
{
	angle = actor.getRotation().ToEuler().z;
	return true;
}

SCRIPT_API(SetActorHealth, bool(IActor& actor, float health))
{
	actor.setHealth(health);
	return true;
}

SCRIPT_API(GetActorHealth, bool(IActor& actor, float& health))
{
	health = actor.getHealth();
	return true;
}

SCRIPT_API(SetActorInvulnerable, bool(IActor& actor, bool invulnerable))
{
	actor.setInvulnerable(invulnerable);
	return true;
}

SCRIPT_API(IsActorInvulnerable, bool(IActor& actor))
{
	return actor.isInvulnerable();
}

SCRIPT_API(IsValidActor, bool(IActor& actor))
{
	return true;
}
