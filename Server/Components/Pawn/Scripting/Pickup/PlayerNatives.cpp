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

SCRIPT_API(CreatePlayerPickup, int(int model, int type, Vector3 position, int virtualWorld))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	if (component)
	{
		IPickup* pickup = component->create(model, type, position, virtualWorld, false);
		if (pickup)
		{
			return pickup->getID();
		}
	}
	return INVALID_PICKUP_ID;
}

SCRIPT_API(DestroyPlayerPickup, bool(IPickup& pickup))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	component->release(pickup.getID());
	return true;
}

SCRIPT_API(IsValidPlayerPickup, bool(IPickup* pickup))
{
	return pickup != nullptr;
}

SCRIPT_API(IsPlayerPickupStreamedIn, bool(IPlayer& player, IPickup& pickup))
{
	return pickup.isStreamedInForPlayer(player);
}

SCRIPT_API(GetPlayerPickupPos, bool(IPickup& pickup, Vector3& pos))
{
	pos = pickup.getPosition();
	return true;
}

SCRIPT_API(GetPlayerPickupModel, int(IPickup& pickup))
{
	return pickup.getModel();
}

SCRIPT_API(GetPlayerPickupType, int(IPickup& pickup))
{
	return pickup.getType();
}

SCRIPT_API(GetPlayerPickupVirtualWorld, int(IPickup& pickup))
{
	return pickup.getVirtualWorld();
}

SCRIPT_API(SetPlayerPickupPos, bool(IPickup& pickup, Vector3 pos, bool update))
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

SCRIPT_API(SetPlayerPickupModel, bool(IPickup& pickup, int model, bool update))
{
	pickup.setModel(model, update);
	return true;
}

SCRIPT_API(SetPlayerPickupType, bool(IPickup& pickup, int type, bool update))
{
	pickup.setType(type, update);
	return true;
}

SCRIPT_API(SetPlayerPickupVirtualWorld, bool(IPickup& pickup, int virtualworld))
{
	pickup.setVirtualWorld(virtualworld);
	return true;
}

