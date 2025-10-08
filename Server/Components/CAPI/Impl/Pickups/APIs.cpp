/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Pickup_Create, objectPtr(int model, int type, float x, float y, float z, int virtualWorld, int* id))
{
	IPickupsComponent* component = ComponentManager::Get()->pickups;
	if (component)
	{
		int id_ = component->reserveLegacyID();
		if (id_ == INVALID_PICKUP_ID)
		{
			return nullptr;
		}

		IPickup* pickup = component->create(model, PickupType(type), { x, y, z }, virtualWorld, false);
		if (pickup)
		{
			component->setLegacyID(id_, pickup->getID());
			*id = id_;
			return pickup;
		}
		else
		{
			component->releaseLegacyID(id_);
		}
	}
	return nullptr;
}

OMP_CAPI(Pickup_AddStatic, bool(int model, int type, float x, float y, float z, int virtualWorld))
{
	IPickupsComponent* component = ComponentManager::Get()->pickups;
	if (component)
	{
		int id_ = component->reserveLegacyID();
		if (id_ == INVALID_PICKUP_ID)
		{
			return false;
		}

		IPickup* pickup = component->create(model, PickupType(type), { x, y, z }, virtualWorld, true);
		if (pickup)
		{
			component->setLegacyID(id_, pickup->getID());
			return true;
		}
		else
		{
			component->releaseLegacyID(id_);
		}
	}
	return false;
}

OMP_CAPI(Pickup_Destroy, bool(objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	pickups->release(p->getID());
	pickups->releaseLegacyID(pickups->toLegacyID(p->getID()));
	return true;
}

OMP_CAPI(Pickup_FromID, objectPtr(int pickupid))
{
	IPickupsComponent* component = ComponentManager::Get()->pickups;
	if (component)
	{
		return component->get(pickupid);
	}
	return nullptr;
}

OMP_CAPI(Pickup_GetID, int(objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, pickup_, INVALID_PICKUP_ID);
	return pickup_->getID();
}

OMP_CAPI(Pickup_IsValid, bool(objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	if (!pickups->get(p->getID()))
		return false;
	return true;
}

OMP_CAPI(Pickup_IsStreamedIn, bool(objectPtr player, objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	return p->isStreamedInForPlayer(*player_);
}

OMP_CAPI(Pickup_GetPos, bool(objectPtr pickup, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	auto pos = p->getPosition();
	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(Pickup_GetModel, int(objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, 0);
	return p->getModel();
}

OMP_CAPI(Pickup_GetType, int(objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, -1);
	return p->getType();
}

OMP_CAPI(Pickup_GetVirtualWorld, int(objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, 0);
	return p->getVirtualWorld();
}

OMP_CAPI(Pickup_SetPos, bool(objectPtr pickup, float x, float y, float z, bool update))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	if (update)
	{
		p->setPosition({ x, y, z });
	}
	else
	{
		p->setPositionNoUpdate({ x, y, z });
	}
	return true;
}

OMP_CAPI(Pickup_SetModel, bool(objectPtr pickup, int model, bool update))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	p->setModel(model, update);
	return true;
}

OMP_CAPI(Pickup_SetType, bool(objectPtr pickup, int type, bool update))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	p->setType(PickupType(type), update);
	return true;
}

OMP_CAPI(Pickup_SetVirtualWorld, bool(objectPtr pickup, int virtualworld))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	p->setVirtualWorld(virtualworld);
	return true;
}

OMP_CAPI(Pickup_ShowForPlayer, bool(objectPtr player, objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	p->setPickupHiddenForPlayer(*player_, false);
	return true;
}

OMP_CAPI(Pickup_HideForPlayer, bool(objectPtr player, objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	p->setPickupHiddenForPlayer(*player_, true);
	return true;
}

OMP_CAPI(Pickup_IsHiddenForPlayer, bool(objectPtr player, objectPtr pickup))
{
	POOL_ENTITY_RET(pickups, IPickup, pickup, p, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	return p->isPickupHiddenForPlayer(*player_);
}
