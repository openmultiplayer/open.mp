#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

namespace pawn_natives {
	template <>
    struct ParamLookup<IGangZone> {
        static IGangZone& ValReq(IPlayer& player, cell ref)
        {
            auto data = queryExtension<IPlayerGangZoneData>(player);
			auto pool = PawnManager::Get()->gangzones;
            if (data && pool) {
                auto ptr = pool->get(data->fromLegacyID(ref));
                if (ptr) {
                    return *ptr;
                }
            }
            throw pawn_natives::ParamCastFailure();
        }

        static IGangZone* Val(IPlayer& player, cell ref) noexcept
        {
            auto data = queryExtension<IPlayerGangZoneData>(player);
			auto pool = PawnManager::Get()->gangzones;
			if (data && pool) {
				return pool->get(data->fromLegacyID(ref));
			}
			return nullptr;
        }
    };

    template <>
    class ParamCast<IGangZone*> {
    public:
        ParamCast(AMX* amx, cell* params, int idx)
        {
            value_ = ParamLookup<IGangZone>::Val(ParamLookup<IPlayer>::ValReq(params[1] /* first param is always playerid */), params[idx]);
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
    class ParamCast<IGangZone const*> {
    public:
        ParamCast(AMX* amx, cell* params, int idx) = delete;
        ParamCast() = delete;
    };

    template <>
    class ParamCast<IGangZone&> {
    public:
        ParamCast(AMX* amx, cell* params, int idx)
            : value_(ParamLookup<IGangZone>::ValReq(ParamLookup<IPlayer>::ValReq(params[1] /* first param is always playerid */), params[idx]))
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

SCRIPT_API(CreatePlayerGangZone, int(IPlayer& player, Vector2 min, Vector2 max))
{
    IGangZonesComponent* component = PawnManager::Get()->gangzones;
	auto data = queryExtension<IPlayerGangZoneData>(player);
	if (component && data) {
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
			return id;
		}
		else
		{
			data->releaseLegacyID(id);
		}
	}
    return INVALID_GANG_ZONE_ID;
}

SCRIPT_API(PlayerGangZoneDestroy, bool(IGangZone& gangzone))
{
    PawnManager::Get()->gangzones->release(gangzone.getID());
    return true;
}

SCRIPT_API(PlayerGangZoneShow, bool(IPlayer& player, IGangZone& gangzone, uint32_t colour))
{
    gangzone.showForPlayer(player, Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(PlayerGangZoneHide, bool(IPlayer& player, IGangZone& gangzone))
{
    gangzone.hideForPlayer(player);
    return true;
}

SCRIPT_API(PlayerGangZoneFlash, bool(IPlayer& player, IGangZone& gangzone, uint32_t colour))
{
    gangzone.flashForPlayer(player, Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(PlayerGangZoneStopFlash, bool(IPlayer& player, IGangZone& gangzone))
{
    gangzone.stopFlashForPlayer(player);
    return true;
}

SCRIPT_API(IsValidPlayerGangZone, bool(IGangZone* zone))
{
    return zone != nullptr;
}

SCRIPT_API(IsPlayerInPlayerGangZone, bool(IPlayer& player, IGangZone& zone))
{
    return zone.isPlayerInside(player);
}

SCRIPT_API(IsPlayerGangZoneVisible, bool(IPlayer& player, IGangZone& zone))
{
    return zone.isShownForPlayer(player);
}

SCRIPT_API(PlayerGangZoneGetColor, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(PlayerGangZoneGetFlashColor, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getFlashingColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(PlayerGangZoneGetColour, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(PlayerGangZoneGetFlashColour, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getFlashingColourForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(IsPlayerGangZoneFlashing, bool(IPlayer& player, IGangZone& zone))
{
    return zone.isFlashingForPlayer(player);
}

SCRIPT_API(PlayerGangZoneGetPos, bool(IGangZone& zone, Vector2& min, Vector2& max))
{
    const GangZonePos& pos = zone.getPosition();
    min = pos.min;
    max = pos.max;
    return true;
}

SCRIPT_API(UseGangZoneCheck, bool(IGangZone& zone, bool toggle))
{
    IGangZonesComponent* component = PawnManager::Get()->gangzones;
    if (component) {
        component->toggleGangZoneCheck(zone, toggle);
        return true;
    }
    return false;
}

