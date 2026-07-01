/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Actor_Create, objectPtr(int model, float x, float y, float z, float rot, int* id))
{
	IActorsComponent* component = ComponentManager::Get()->actors;
	if (component)
	{
		IActor* actor = component->create(model, { x, y, z }, rot);
		if (actor)
		{
			*id = actor->getID();
			return actor;
		}
	}
	return nullptr;
}

OMP_CAPI(Actor_Destroy, bool(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actors->release(actor_->getID());
	return true;
}

OMP_CAPI(Actor_FromID, objectPtr(int actorid))
{
	IActorsComponent* component = ComponentManager::Get()->actors;
	if (component)
	{
		return component->get(actorid);
	}
	return nullptr;
}

OMP_CAPI(Actor_GetID, int(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, INVALID_ACTOR_ID);
	return actor_->getID();
}

OMP_CAPI(Actor_IsStreamedInFor, bool(objectPtr actor, objectPtr player))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	return actor_->isStreamedInForPlayer(*player_);
}

OMP_CAPI(Actor_SetVirtualWorld, bool(objectPtr actor, int vw))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actor_->setVirtualWorld(vw);
	return true;
}

OMP_CAPI(Actor_GetVirtualWorld, int(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, 0);
	return actor_->getVirtualWorld();
}

OMP_CAPI(Actor_ApplyAnimation, bool(objectPtr actor, StringCharPtr name, StringCharPtr library, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	const AnimationData animationData(delta, loop, lockX, lockY, freeze, uint32_t(time), library, name);
	actor_->applyAnimation(animationData);
	return true;
}

OMP_CAPI(Actor_ClearAnimations, bool(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actor_->clearAnimations();
	return true;
}

OMP_CAPI(Actor_SetPos, bool(objectPtr actor, float x, float y, float z))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actor_->setPosition({ x, y, z });
	return true;
}

OMP_CAPI(Actor_GetPos, bool(objectPtr actor, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	const Vector3& pos = actor_->getPosition();

	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(Actor_SetFacingAngle, bool(objectPtr actor, float angle))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actor_->setRotation(Vector3(0.0f, 0.0f, angle));
	return true;
}

OMP_CAPI(Actor_GetFacingAngle, float(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, 0.0f);
	return actor_->getRotation().ToEuler().z;
}

OMP_CAPI(Actor_SetHealth, bool(objectPtr actor, float hp))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actor_->setHealth(hp);
	return true;
}

OMP_CAPI(Actor_GetHealth, float(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, 0.0f);
	return actor_->getHealth();
}

OMP_CAPI(Actor_SetInvulnerable, bool(objectPtr actor, bool toggle))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actor_->setInvulnerable(toggle);
	return true;
}

OMP_CAPI(Actor_IsInvulnerable, bool(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	return actor_->isInvulnerable();
}

OMP_CAPI(Actor_IsValid, bool(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	if (!actors->get(actor_->getID()))
		return false;
	return true;
}

OMP_CAPI(Actor_SetSkin, bool(objectPtr actor, int skin))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	actor_->setSkin(skin);
	return true;
}

OMP_CAPI(Actor_GetSkin, int(objectPtr actor))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, 0);
	return actor_->getSkin();
}

OMP_CAPI(Actor_GetAnimation, bool(objectPtr actor, OutputStringViewPtr library, OutputStringViewPtr name, float* delta, bool* loop, bool* lockX, bool* lockY, bool* freeze, int* time))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	const AnimationData& anim = actor_->getAnimation();

	SET_CAPI_STRING_VIEW(library, anim.lib);
	SET_CAPI_STRING_VIEW(name, anim.name);
	*delta = anim.delta;
	*loop = anim.loop;
	*lockX = anim.lockX;
	*lockY = anim.lockY;
	*freeze = anim.freeze;
	*time = int(anim.time);
	return true;
}

OMP_CAPI(Actor_GetSpawnInfo, bool(objectPtr actor, float* x, float* y, float* z, float* angle, int* skin))
{
	POOL_ENTITY_RET(actors, IActor, actor, actor_, false);
	const ActorSpawnData& spawnData = actor_->getSpawnData();

	*x = spawnData.position.x;
	*y = spawnData.position.y;
	*z = spawnData.position.z;
	*angle = spawnData.facingAngle;
	*skin = spawnData.skin;
	return true;
}
