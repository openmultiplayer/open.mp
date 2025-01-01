/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(CreatePlayerActor, int(IPlayer& player, int skin, Vector3 position, float angle))
{
	IActorsComponent* component = PawnManager::Get()->actors;
	auto data = queryExtension<IPlayerActorData>(player);
	if (component && data)
	{
		int id = data->reserveLegacyID();
		if (id == INVALID_ACTOR_ID)
		{
			return INVALID_ACTOR_ID;
		}

		IActor* actor = component->create(skin, position, angle);
		if (actor)
		{
			data->setLegacyID(id, actor->getID());
			actor->setLegacyPlayer(&player);
			return id;
		}
		else
		{
			data->releaseLegacyID(id);
		}
	}
	return INVALID_ACTOR_ID;
}
SCRIPT_API(DestroyPlayerActor, bool(IPlayer& player, int actorid))
{
	IActorsComponent* component = PawnManager::Get()->actors;
	auto data = queryExtension<IPlayerActorData>(player);
	if (component && data)
	{
		int realid = data->fromLegacyID(actorid);
		if (realid)
		{
			component->release(realid);
			data->releaseLegacyID(actorid);
			return true;
		}
	}
	return false;
}

SCRIPT_API(IsPlayerActorStreamedIn, bool(IPlayer& player, IPlayerActor& actor))
{
	return actor.isStreamedInForPlayer(player);
}

SCRIPT_API(SetPlayerActorVirtualWorld, bool(IPlayer& player, IPlayerActor& actor, int virtualWorld))
{
	actor.setVirtualWorld(virtualWorld);
	return true;
}

SCRIPT_API(GetPlayerActorVirtualWorld, int(IPlayer& player, IPlayerActor& actor))
{
	return actor.getVirtualWorld();
}

SCRIPT_API(ApplyPlayerActorAnimation, bool(IPlayer& player, IPlayerActor& actor, const std::string& animationLibrary, const std::string& animationName, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time))
{
	const AnimationData animationData(delta, loop, lockX, lockY, freeze, time, animationLibrary, animationName);
	actor.applyAnimation(animationData);
	return true;
}

SCRIPT_API(ClearPlayerActorAnimations, bool(IPlayer& player, IPlayerActor& actor))
{
	actor.clearAnimations();
	return true;
}

SCRIPT_API(SetPlayerActorPos, bool(IPlayer& player, IPlayerActor& actor, Vector3 position))
{
	actor.setPosition(position);
	return true;
}

SCRIPT_API(GetPlayerActorPos, bool(IPlayer& player, IPlayerActor& actor, Vector3& position))
{
	position = actor.getPosition();
	return true;
}

SCRIPT_API(SetPlayerActorFacingAngle, bool(IPlayer& player, IPlayerActor& actor, float angle))
{
	actor.setRotation(Vector3(0.0f, 0.0f, angle));
	return true;
}

SCRIPT_API(GetPlayerActorFacingAngle, bool(IPlayer& player, IPlayerActor& actor, float& angle))
{
	angle = actor.getRotation().ToEuler().z;
	return true;
}

SCRIPT_API(SetPlayerActorHealth, bool(IPlayer& player, IPlayerActor& actor, float health))
{
	actor.setHealth(health);
	return true;
}

SCRIPT_API(GetPlayerActorHealth, bool(IPlayer& player, IPlayerActor& actor, float& health))
{
	health = actor.getHealth();
	return true;
}

SCRIPT_API(SetPlayerActorInvulnerable, bool(IPlayer& player, IPlayerActor& actor, bool invulnerable))
{
	
	actor.setInvulnerable(invulnerable);
	return true;
}

SCRIPT_API(IsPlayerActorInvulnerable, bool(IPlayer& player, IPlayerActor& actor))
{
	return actor.isInvulnerable();
}

SCRIPT_API(IsValidPlayerActor, bool(IPlayer& player, IPlayerActor& actor))
{
	return true;
}

SCRIPT_API(SetPlayerActorSkin, bool(IPlayer& player, IPlayerActor& actor, int skin))
{
	actor.setSkin(skin);
	return true;
}

SCRIPT_API(GetPlayerActorSkin, int(IPlayer& player, IPlayerActor& actor))
{
	return actor.getSkin();
}

SCRIPT_API(GetPlayerActorAnimation, bool(IPlayer& player, IPlayerActor& actor, OutputOnlyString& animationLibrary, OutputOnlyString& animationName, float& delta, bool& loop, bool& lockX, bool& lockY, bool& freeze, int& time))
{
	const AnimationData& anim = actor.getAnimation();

	animationLibrary = anim.lib;
	animationName = anim.name;
	delta = anim.delta;
	loop = anim.loop;
	lockX = anim.lockX;
	lockY = anim.lockY;
	freeze = anim.freeze;
	time = anim.time;

	return true;
}

SCRIPT_API(GetPlayerActorSpawnInfo, bool(IPlayerActor& actor, int& skin, Vector3& position, float& angle))
{
	const ActorSpawnData& spawnData = actor.getSpawnData();

	position = spawnData.position;
	angle = spawnData.facingAngle;
	skin = spawnData.skin;
	return true;
}