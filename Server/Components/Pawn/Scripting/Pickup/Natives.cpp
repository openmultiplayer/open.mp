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

static IPickup* TryGetPickup(cell ref) noexcept
{
	auto pool = PawnManager::Get()->pickups;
	if (pool)
	{
		return pool->get(pool->fromLegacyID(ref));
	}
	return nullptr;
}

SCRIPT_API(CreatePickup, int(int model, int type, Vector3 position, int virtualWorld))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	if (component)
	{
		int id = component->reserveLegacyID();
		if (id == INVALID_PICKUP_ID)
		{
			return INVALID_PICKUP_ID;
		}

		IPickup* pickup = component->create(model, type, position, virtualWorld, false);
		if (pickup)
		{
			component->setLegacyID(id, pickup->getID());
			return id;
		}
		else
		{
			component->releaseLegacyID(id);
		}
	}
	return INVALID_PICKUP_ID;
}

SCRIPT_API(AddStaticPickup, bool(int model, int type, Vector3 position, int virtualWorld))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	if (component)
	{
		int id = component->reserveLegacyID();
		if (id == INVALID_PICKUP_ID)
		{
			return INVALID_PICKUP_ID;
		}

		IPickup* pickup = component->create(model, type, position, virtualWorld, true);
		if (pickup)
		{
			component->setLegacyID(id, pickup->getID());
			return true;
		}
		else
		{
			component->releaseLegacyID(id);
		}
	}
	return false;
}

SCRIPT_API(DestroyPickup, bool(cell legacyid))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	if (component)
	{
		int realid = component->fromLegacyID(legacyid);
		if (realid)
		{
			component->release(realid);
			component->releaseLegacyID(legacyid);
			return true;
		}
	}
	return false;
}

SCRIPT_API(IsValidPickup, bool(cell pickupid))
{
	return TryGetPickup(pickupid) != nullptr;
}

SCRIPT_API(IsPickupStreamedIn, bool(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	return pickup->isStreamedInForPlayer(player);
}

SCRIPT_API(GetPickupPos, bool(cell pickupid, Vector3& pos))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pos = pickup->getPosition();
	return true;
}

SCRIPT_API(GetPickupModel, int(cell pickupid))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return 0;
	}
	return pickup->getModel();
}

SCRIPT_API(GetPickupType, int(cell pickupid))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return -1;
	}
	return pickup->getType();
}

SCRIPT_API(GetPickupVirtualWorld, int(cell pickupid))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return 0;
	}
	return pickup->getVirtualWorld();
}

SCRIPT_API(SetPickupPos, bool(cell pickupid, Vector3 pos, bool update))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	if (update)
	{
		pickup->setPosition(pos);
	}
	else
	{
		pickup->setPositionNoUpdate(pos);
	}
	return true;
}

SCRIPT_API(SetPickupModel, bool(cell pickupid, int model, bool update))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setModel(model, update);
	return true;
}

SCRIPT_API(SetPickupType, bool(cell pickupid, int type, bool update))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setType(type, update);
	return true;
}

SCRIPT_API(SetPickupVirtualWorld, bool(cell pickupid, int virtualworld))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setVirtualWorld(virtualworld);
	return true;
}

SCRIPT_API(ShowPickupForPlayer, bool(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setPickupHiddenForPlayer(player, false);
	return true;
}

SCRIPT_API(HidePickupForPlayer, bool(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setPickupHiddenForPlayer(player, true);
	return true;
}

SCRIPT_API(IsPickupHiddenForPlayer, bool(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	return pickup->isPickupHiddenForPlayer(player);
}
