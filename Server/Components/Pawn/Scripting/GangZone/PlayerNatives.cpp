#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

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

SCRIPT_API(PlayerGangZoneDestroy, bool(IPlayer& player, int gangzoneid))
{
	auto data = queryExtension<IPlayerGangZoneData>(player);
	auto pool = PawnManager::Get()->gangzones;
	if (data && pool)
	{
		int realid = data->fromLegacyID(gangzoneid);
		if (realid)
		{
			pool->release(realid);
			data->releaseLegacyID(gangzoneid);
			return true;
		}
	}
	return false;
}

SCRIPT_API(PlayerGangZoneShow, bool(IPlayer& player, IPlayerGangZone& gangzone, uint32_t colour))
{
	gangzone.showForPlayer(player, Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(PlayerGangZoneHide, bool(IPlayer& player, IPlayerGangZone& gangzone))
{
	gangzone.hideForPlayer(player);
	return true;
}

SCRIPT_API(PlayerGangZoneFlash, bool(IPlayer& player, IPlayerGangZone& gangzone, uint32_t colour))
{
	gangzone.flashForPlayer(player, Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(PlayerGangZoneStopFlash, bool(IPlayer& player, IPlayerGangZone& gangzone))
{
	gangzone.stopFlashForPlayer(player);
	return true;
}

SCRIPT_API(IsValidPlayerGangZone, bool(IPlayer& player, IPlayerGangZone& gangzone))
{
	return true;
}

SCRIPT_API(IsPlayerInPlayerGangZone, bool(IPlayer& player, IPlayerGangZone& gangzone))
{
	return gangzone.isPlayerInside(player);
}

SCRIPT_API(IsPlayerGangZoneVisible, bool(IPlayer& player, IPlayerGangZone& gangzone))
{
	return gangzone.isShownForPlayer(player);
}

SCRIPT_API(PlayerGangZoneGetColor, int(IPlayer& player, IPlayerGangZone& gangzone))
{
	if (gangzone.isShownForPlayer(player))
	{
		return gangzone.getColourForPlayer(player).RGBA();
	}
	return 0;
}

SCRIPT_API(PlayerGangZoneGetFlashColor, int(IPlayer& player, IPlayerGangZone& gangzone))
{
	if (gangzone.isShownForPlayer(player))
	{
		return gangzone.getFlashingColourForPlayer(player).RGBA();
	}
	return 0;
}

SCRIPT_API(PlayerGangZoneGetColour, int(IPlayer& player, IPlayerGangZone& gangzone))
{
	if (gangzone.isShownForPlayer(player))
	{
		return gangzone.getColourForPlayer(player).RGBA();
	}
	return 0;
}

SCRIPT_API(PlayerGangZoneGetFlashColour, int(IPlayer& player, IPlayerGangZone& gangzone))
{
	if (gangzone.isShownForPlayer(player))
	{
		return gangzone.getFlashingColourForPlayer(player).RGBA();
	}
	return 0;
}

SCRIPT_API(IsPlayerGangZoneFlashing, bool(IPlayer& player, IPlayerGangZone& gangzone))
{
	return gangzone.isFlashingForPlayer(player);
}

SCRIPT_API(PlayerGangZoneGetPos, bool(IPlayer& player, IPlayerGangZone& gangzone, Vector2& min, Vector2& max))
{
	const GangZonePos& pos = gangzone.getPosition();
	min = pos.min;
	max = pos.max;
	return true;
}

SCRIPT_API(UsePlayerGangZoneCheck, bool(IPlayer& player, IPlayerGangZone* gangzone, bool enable))
{
	if (gangzone == nullptr)
	{
		return false;
	}
	IGangZonesComponent* component = PawnManager::Get()->gangzones;
	if (component)
	{
		component->useGangZoneCheck(*reinterpret_cast<IGangZone*>(gangzone), enable);
		return true;
	}
	return false;
}
