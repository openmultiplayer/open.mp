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

static IGangZone* TryGetGangZone(cell ref) noexcept
{
	auto pool = PawnManager::Get()->gangzones;
	if (pool)
	{
		return pool->get(pool->fromLegacyID(ref));
	}
	return nullptr;
}

SCRIPT_API(GangZoneCreate, int(Vector2 min, Vector2 max))
{
	IGangZonesComponent* component = PawnManager::Get()->gangzones;
	if (component)
	{
		int id = component->reserveLegacyID();
		if (id == INVALID_GANG_ZONE_ID)
		{
			return INVALID_GANG_ZONE_ID;
		}

		GangZonePos pos;
		pos.min.x = truncf(min.x);
		pos.min.y = truncf(min.y);
		pos.max.x = truncf(max.x);
		pos.max.y = truncf(max.y);

		IGangZone* gz = component->create(pos);
		if (gz)
		{
			component->setLegacyID(id, gz->getID());
			return id;
		}
		else
		{
			component->releaseLegacyID(id);
		}
	}
	return INVALID_GANG_ZONE_ID;
}

SCRIPT_API(GangZoneDestroy, bool(int legacyid))
{
	auto pool = PawnManager::Get()->gangzones;
	if (pool)
	{
		int realid = pool->fromLegacyID(legacyid);
		if (realid)
		{
			pool->release(realid);
			pool->releaseLegacyID(legacyid);
			return true;
		}
	}

	return false;
}

SCRIPT_API(GangZoneShowForPlayer, bool(IPlayer& player, cell gangzoneid, uint32_t colour))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->showForPlayer(player, Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(GangZoneShowForAll, bool(cell gangzoneid, uint32_t colour))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	IPlayerPool* pool = PawnManager::Get()->players;
	for (IPlayer* player : pool->entries())
	{
		gangzone->showForPlayer(*player, Colour::FromRGBA(colour));
	}
	return true;
}

SCRIPT_API(GangZoneHideForPlayer, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->hideForPlayer(player);
	return true;
}

SCRIPT_API(GangZoneHideForAll, bool(cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	IPlayerPool* pool = PawnManager::Get()->players;
	for (IPlayer* player : pool->entries())
	{
		gangzone->hideForPlayer(*player);
	}
	return true;
}

SCRIPT_API(GangZoneFlashForPlayer, bool(IPlayer& player, cell gangzoneid, uint32_t colour))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->flashForPlayer(player, Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(GangZoneFlashForAll, bool(cell gangzoneid, uint32_t colour))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	IPlayerPool* pool = PawnManager::Get()->players;
	for (IPlayer* player : pool->entries())
	{
		gangzone->flashForPlayer(*player, Colour::FromRGBA(colour));
	}
	return true;
}

SCRIPT_API(GangZoneStopFlashForPlayer, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->stopFlashForPlayer(player);
	return true;
}

SCRIPT_API(GangZoneStopFlashForAll, bool(cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	IPlayerPool* pool = PawnManager::Get()->players;
	for (IPlayer* player : pool->entries())
	{
		gangzone->stopFlashForPlayer(*player);
	}
	return true;
}

SCRIPT_API(IsValidGangZone, bool(cell gangzoneid))
{
	return TryGetGangZone(gangzoneid) != nullptr;
}

SCRIPT_API(IsPlayerInGangZone, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	return gangzone->isPlayerInside(player);
}

SCRIPT_API(IsGangZoneVisibleForPlayer, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	return gangzone->isShownForPlayer(player);
}

SCRIPT_API(GangZoneGetColorForPlayer, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return 0;
	}
	if (gangzone->isShownForPlayer(player))
	{
		return gangzone->getColourForPlayer(player).RGBA();
	}
	else
	{
		return 0;
	}
}

SCRIPT_API(GangZoneGetFlashColorForPlayer, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return 0;
	}
	if (gangzone->isShownForPlayer(player))
	{
		return gangzone->getFlashingColourForPlayer(player).RGBA();
	}
	else
	{
		return 0;
	}
}

SCRIPT_API(GangZoneGetColourForPlayer, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return 0;
	}
	if (gangzone->isShownForPlayer(player))
	{
		return gangzone->getColourForPlayer(player).RGBA();
	}
	else
	{
		return 0;
	}
}

SCRIPT_API(GangZoneGetFlashColourForPlayer, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return 0;
	}
	if (gangzone->isShownForPlayer(player))
	{
		return gangzone->getFlashingColourForPlayer(player).RGBA();
	}
	else
	{
		return 0;
	}
}

SCRIPT_API(IsGangZoneFlashingForPlayer, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	return gangzone->isFlashingForPlayer(player);
}

SCRIPT_API(GangZoneGetPos, bool(cell gangzoneid, Vector2& min, Vector2& max))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	const GangZonePos& pos = gangzone->getPosition();
	min = pos.min;
	max = pos.max;
	return true;
}

SCRIPT_API(UseGangZoneCheck, bool(cell gangzoneid, bool enable))
{
	IGangZone* gangzone = TryGetGangZone(gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	IGangZonesComponent* component = PawnManager::Get()->gangzones;
	if (component)
	{
		component->useGangZoneCheck(*gangzone, enable);
		return true;
	}
	return false;
}
