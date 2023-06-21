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

SCRIPT_API(DestroyPickup, bool(int pickupid))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	if (component)
	{
		int realid = component->fromLegacyID(pickupid);
		if (realid)
		{
			component->release(realid);
			component->releaseLegacyID(pickupid);
			return true;
		}
	}
	return false;
}

SCRIPT_API(IsValidPickup, bool(IPickup& pickup))
{
	return true;
}

SCRIPT_API(IsPickupStreamedIn, bool(IPlayer& player, IPickup& pickup))
{
	return pickup.isStreamedInForPlayer(player);
}

SCRIPT_API(GetPickupPos, bool(IPickup& pickup, Vector3& pos))
{
	pos = pickup.getPosition();
	return true;
}

SCRIPT_API(GetPickupModel, int(IPickup& pickup))
{
	return pickup.getModel();
}

SCRIPT_API_FAILRET(GetPickupType, -1, int(IPickup& pickup))
{
	return pickup.getType();
}

SCRIPT_API(GetPickupVirtualWorld, int(IPickup& pickup))
{
	return pickup.getVirtualWorld();
}

SCRIPT_API(SetPickupPos, bool(IPickup& pickup, Vector3 pos, bool update))
{
	if (update)
	{
		pickup.setPosition(pos);
	}
	else
	{
		pickup.setPositionNoUpdate(pos);
	}
	return true;
}

SCRIPT_API(SetPickupModel, bool(IPickup& pickup, int model, bool update))
{
	pickup.setModel(model, update);
	return true;
}

SCRIPT_API(SetPickupType, bool(IPickup& pickup, int type, bool update))
{
	pickup.setType(type, update);
	return true;
}

SCRIPT_API(SetPickupVirtualWorld, bool(IPickup& pickup, int virtualworld))
{
	pickup.setVirtualWorld(virtualworld);
	return true;
}

SCRIPT_API(ShowPickupForPlayer, bool(IPlayer& player, IPickup& pickup))
{
	pickup.setPickupHiddenForPlayer(player, false);
	return true;
}

SCRIPT_API(HidePickupForPlayer, bool(IPlayer& player, IPickup& pickup))
{
	pickup.setPickupHiddenForPlayer(player, true);
	return true;
}

SCRIPT_API(IsPickupHiddenForPlayer, bool(IPlayer& player, IPickup& pickup))
{
	return pickup.isPickupHiddenForPlayer(player);
}
