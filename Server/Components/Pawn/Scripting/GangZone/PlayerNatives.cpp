#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

static IGangZone* TryGetGangZone(IPlayer& player, cell ref) noexcept
{
	auto data = queryExtension<IPlayerGangZoneData>(player);
	auto pool = PawnManager::Get()->gangzones;
	if (data && pool)
	{
		return pool->get(data->fromLegacyID(ref));
	}
	return nullptr;
}

SCRIPT_API(CreatePlayerGangZone, int(IPlayer& player, Vector2 min, Vector2 max))
{
	IGangZonesComponent* component = PawnManager::Get()->gangzones;
	auto data = queryExtension<IPlayerGangZoneData>(player);
	if (component && data)
	{
		int id = data->reserveLegacyID();
		if (id == INVALID_GANG_ZONE_ID)
		{
			return INVALID_GANG_ZONE_ID;
		}

		GangZonePos pos;
		pos.min = min;
		pos.max = max;

		IGangZone* gz = component->create(pos);
		if (gz)
		{
			data->setLegacyID(id, gz->getID());
			gz->setLegacyPlayer(&player);
			return id;
		}
		else
		{
			data->releaseLegacyID(id);
		}
	}
	return INVALID_GANG_ZONE_ID;
}

SCRIPT_API(PlayerGangZoneDestroy, bool(IPlayer& player, int legacyid))
{
	auto data = queryExtension<IPlayerGangZoneData>(player);
	auto pool = PawnManager::Get()->gangzones;
	if (data && pool)
	{
		int realid = data->fromLegacyID(legacyid);
		if (realid)
		{
			pool->release(realid);
			data->releaseLegacyID(legacyid);
		}
	}

	return false;
}

SCRIPT_API(PlayerGangZoneShow, bool(IPlayer& player, cell gangzoneid, uint32_t colour))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->showForPlayer(player, Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(PlayerGangZoneHide, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->hideForPlayer(player);
	return true;
}

SCRIPT_API(PlayerGangZoneFlash, bool(IPlayer& player, cell gangzoneid, uint32_t colour))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->flashForPlayer(player, Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(PlayerGangZoneStopFlash, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	gangzone->stopFlashForPlayer(player);
	return true;
}

SCRIPT_API(IsValidPlayerGangZone, bool(IPlayer& player, cell gangzoneid))
{
	return TryGetGangZone(player, gangzoneid) != nullptr;
}

SCRIPT_API(IsPlayerInPlayerGangZone, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	return gangzone->isPlayerInside(player);
}

SCRIPT_API(IsPlayerGangZoneVisible, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	return gangzone->isShownForPlayer(player);
}

SCRIPT_API(PlayerGangZoneGetColor, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
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

SCRIPT_API(PlayerGangZoneGetFlashColor, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
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

SCRIPT_API(PlayerGangZoneGetColour, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
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

SCRIPT_API(PlayerGangZoneGetFlashColour, int(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
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

SCRIPT_API(IsPlayerGangZoneFlashing, bool(IPlayer& player, cell gangzoneid))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	return gangzone->isFlashingForPlayer(player);
}

SCRIPT_API(PlayerGangZoneGetPos, bool(IPlayer& player, cell gangzoneid, Vector2& min, Vector2& max))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
	if (gangzone == nullptr)
	{
		return false;
	}
	const GangZonePos& pos = gangzone->getPosition();
	min = pos.min;
	max = pos.max;
	return true;
}

SCRIPT_API(UsePlayerGangZoneCheck, bool(IPlayer& player, cell gangzoneid, bool enable))
{
	IGangZone* gangzone = TryGetGangZone(player, gangzoneid);
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
