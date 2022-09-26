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

static IPickup* TryGetPickup(IPlayer& player, cell ref) noexcept
{
	auto data = queryExtension<IPlayerPickupData>(player);
	auto pool = PawnManager::Get()->pickups;
	if (data && pool)
	{
		return pool->get(data->fromLegacyID(ref));
	}
	return nullptr;
}

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

SCRIPT_API(DestroyPlayerPickup, bool(IPlayer& player, cell pickupid))
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

SCRIPT_API(IsValidPlayerPickup, bool(IPlayer& player, cell pickupid))
{
	return TryGetPickup(player, pickupid) != nullptr;
}

SCRIPT_API(IsPlayerPickupStreamedIn, bool(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	return pickup->isStreamedInForPlayer(player);
}

SCRIPT_API(GetPlayerPickupPos, bool(IPlayer& player, cell pickupid, Vector3& pos))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pos = pickup->getPosition();
	return true;
}

SCRIPT_API(GetPlayerPickupModel, int(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return 0;
	}
	return pickup->getModel();
}

SCRIPT_API(GetPlayerPickupType, int(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return -1;
	}
	return pickup->getType();
}

SCRIPT_API(GetPlayerPickupVirtualWorld, int(IPlayer& player, cell pickupid))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return 0;
	}
	return pickup->getVirtualWorld();
}

SCRIPT_API(SetPlayerPickupPos, bool(IPlayer& player, cell pickupid, Vector3 pos, bool update))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
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

SCRIPT_API(SetPlayerPickupModel, bool(IPlayer& player, cell pickupid, int model, bool update))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setModel(model, update);
	return true;
}

SCRIPT_API(SetPlayerPickupType, bool(IPlayer& player, cell pickupid, int type, bool update))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setType(type, update);
	return true;
}

SCRIPT_API(SetPlayerPickupVirtualWorld, bool(IPlayer& player, cell pickupid, int virtualworld))
{
	IPickup* pickup = TryGetPickup(player, pickupid);
	if (pickup == nullptr)
	{
		return false;
	}
	pickup->setVirtualWorld(virtualworld);
	return true;
}

