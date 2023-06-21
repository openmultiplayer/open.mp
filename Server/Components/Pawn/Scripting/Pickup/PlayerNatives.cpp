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

SCRIPT_API(CreatePlayerPickup, int(IPlayer& player, int model, int type, Vector3 position, int virtualWorld))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	auto data = queryExtension<IPlayerPickupData>(player);
	if (component && data)
	{
		int id = data->reserveLegacyID();
		if (id == INVALID_PICKUP_ID)
		{
			return INVALID_PICKUP_ID;
		}

		IPickup* pickup = component->create(model, type, position, virtualWorld, false);
		if (pickup)
		{
			data->setLegacyID(id, pickup->getID());
			pickup->setLegacyPlayer(&player);
			return id;
		}
		else
		{
			data->releaseLegacyID(id);
		}
	}
	return INVALID_PICKUP_ID;
}

SCRIPT_API(DestroyPlayerPickup, bool(IPlayer& player, int pickupid))
{
	IPickupsComponent* component = PawnManager::Get()->pickups;
	auto data = queryExtension<IPlayerPickupData>(player);
	if (component && data)
	{
		int realid = data->fromLegacyID(pickupid);
		if (realid)
		{
			component->release(realid);
			data->releaseLegacyID(pickupid);
			return true;
		}
	}
	return false;
}

SCRIPT_API(IsValidPlayerPickup, bool(IPlayer& player, IPlayerPickup& pickup))
{
	return true;
}

SCRIPT_API(IsPlayerPickupStreamedIn, bool(IPlayer& player, IPlayerPickup& pickup))
{
	return pickup.isStreamedInForPlayer(player);
}

SCRIPT_API(GetPlayerPickupPos, bool(IPlayer& player, IPlayerPickup& pickup, Vector3& pos))
{
	pos = pickup.getPosition();
	return true;
}

SCRIPT_API(GetPlayerPickupModel, int(IPlayer& player, IPlayerPickup& pickup))
{
	return pickup.getModel();
}

SCRIPT_API_FAILRET(GetPlayerPickupType, -1, int(IPlayer& player, IPlayerPickup& pickup))
{
	return pickup.getType();
}

SCRIPT_API(GetPlayerPickupVirtualWorld, int(IPlayer& player, IPlayerPickup& pickup))
{
	return pickup.getVirtualWorld();
}

SCRIPT_API(SetPlayerPickupPos, bool(IPlayer& player, IPlayerPickup& pickup, Vector3 pos, bool update))
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

SCRIPT_API(SetPlayerPickupModel, bool(IPlayer& player, IPlayerPickup& pickup, int model, bool update))
{
	pickup.setModel(model, update);
	return true;
}

SCRIPT_API(SetPlayerPickupType, bool(IPlayer& player, IPlayerPickup& pickup, int type, bool update))
{
	pickup.setType(type, update);
	return true;
}

SCRIPT_API(SetPlayerPickupVirtualWorld, bool(IPlayer& player, IPlayerPickup& pickup, int virtualworld))
{
	pickup.setVirtualWorld(virtualworld);
	return true;
}
