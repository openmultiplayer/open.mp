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

// Custom parameter lookup with ID translation.
namespace pawn_natives
{
	template <>
    struct ParamLookup<IGangZone> {
        static IGangZone& ValReq(cell ref)
        {
            auto pool = PawnManager::Get()->gangzones;
            if (pool) {
                auto ptr = pool->get(pool->fromLegacyID(ref));
                if (ptr) {
                    return *ptr;
                }
            }
            throw pawn_natives::ParamCastFailure();
        }

        static IGangZone* Val(cell ref) noexcept
        {
            auto pool = PawnManager::Get()->gangzones;
            if (pool) {
                return pool->get(pool->fromLegacyID(ref));
            }
            return nullptr;
        }
    };

    template <>
    class ParamCast<IGangZone*> {
    public:
        ParamCast(AMX* amx, cell* params, int idx) noexcept
        {
            value_ = ParamLookup<IGangZone>::Val(params[idx]);
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<IGangZone*> const&) = delete;
        ParamCast(ParamCast<IGangZone*>&&) = delete;

        operator IGangZone*()
        {
            return value_;
        }

        static constexpr int Size = 1;

    private:
        IGangZone* value_;
    };

    template <>
    class ParamCast<IGangZone&> {
    public:
        ParamCast(AMX* amx, cell* params, int idx)
            : value_(ParamLookup<IGangZone>::ValReq(params[idx]))
        {
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<IGangZone&> const&) = delete;
        ParamCast(ParamCast<IGangZone&>&&) = delete;

        operator IGangZone&()
        {
            return value_;
        }

        static constexpr int Size = 1;

    private:
        IGangZone& value_;
    };

    template <>
    class ParamCast<const IGangZone&> {
    public:
        ParamCast(AMX*, cell*, int) = delete;
        ParamCast() = delete;
    };
}

SCRIPT_API(GangZoneCreate, int(Vector2 min, Vector2 max))
{
    IGangZonesComponent* component = PawnManager::Get()->gangzones;
    if (component) {
		int id = component->reserveLegacyID();
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

SCRIPT_API(GangZoneDestroy, bool(IGangZone& gangzone))
{
    PawnManager::Get()->gangzones->release(gangzone.getID());
    return true;
}

SCRIPT_API(GangZoneShowForPlayer, bool(IPlayer& player, IGangZone& gangzone, uint32_t colour))
{
    gangzone.showForPlayer(player, Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(GangZoneShowForAll, bool(IGangZone& gangzone, uint32_t colour))
{
    IPlayerPool* pool = PawnManager::Get()->players;
    for (IPlayer* player : pool->entries()) {
        gangzone.showForPlayer(*player, Colour::FromRGBA(colour));
    }
    return true;
}

SCRIPT_API(GangZoneHideForPlayer, bool(IPlayer& player, IGangZone& gangzone))
{
    gangzone.hideForPlayer(player);
    return true;
}

SCRIPT_API(GangZoneHideForAll, bool(IGangZone& gangzone))
{
    IPlayerPool* pool = PawnManager::Get()->players;
    for (IPlayer* player : pool->entries()) {
        gangzone.hideForPlayer(*player);
    }
    return true;
}

SCRIPT_API(GangZoneFlashForPlayer, bool(IPlayer& player, IGangZone& gangzone, uint32_t colour))
{
    gangzone.flashForPlayer(player, Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(GangZoneFlashForAll, bool(IGangZone& gangzone, uint32_t colour))
{
    IPlayerPool* pool = PawnManager::Get()->players;
    for (IPlayer* player : pool->entries()) {
        gangzone.flashForPlayer(*player, Colour::FromRGBA(colour));
    }
    return true;
}

SCRIPT_API(GangZoneStopFlashForPlayer, bool(IPlayer& player, IGangZone& gangzone))
{
    gangzone.stopFlashForPlayer(player);
    return true;
}

SCRIPT_API(GangZoneStopFlashForAll, bool(IGangZone& gangzone))
{
    IPlayerPool* pool = PawnManager::Get()->players;
    for (IPlayer* player : pool->entries()) {
        gangzone.stopFlashForPlayer(*player);
    }
    return true;
}

SCRIPT_API(IsValidGangZone, bool(IGangZone* zone))
{
    return zone != nullptr;
}

SCRIPT_API(IsPlayerInGangZone, bool(IPlayer& player, IGangZone& zone))
{
    return zone.isPlayerInside(player);
}

SCRIPT_API(IsGangZoneVisibleForPlayer, bool(IPlayer& player, IGangZone& zone))
{
    return zone.isShownForPlayer(player);
}

SCRIPT_API(GangZoneGetColorForPlayer, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(GangZoneGetFlashColorForPlayer, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getFlashingColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(GangZoneGetColourForPlayer, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(GangZoneGetFlashColourForPlayer, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getFlashingColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(IsGangZoneFlashingForPlayer, bool(IPlayer& player, IGangZone& zone))
{
    return zone.isFlashingForPlayer(player);
}

SCRIPT_API(GangZoneGetPos, bool(IGangZone& zone, Vector2& min, Vector2& max))
{
    const GangZonePos& pos = zone.getPosition();
    min = pos.min;
    max = pos.max;
    return true;
}

SCRIPT_API(UseGangZoneCheck, bool(IGangZone& zone, bool enable))
{
    IGangZonesComponent* component = PawnManager::Get()->gangzones;
    if (component) {
        component->useGangZoneCheck(zone, enable);
        return true;
    }
    return false;
}

