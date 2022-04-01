#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(GangZoneCreate, int(Vector2 min, Vector2 max))
{
    IGangZonesComponent* component = PawnManager::Get()->gangzones;
    if (component) {
        GangZonePos pos;
        pos.min = min;
        pos.max = max;

        IGangZone* gz = component->create(pos);
        if (gz) {
            return gz->getID();
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
        return zone.getColorForPlayer(player).RGBA();
    } else {
        return 0;
    }
}

SCRIPT_API(GangZoneGetFlashColorForPlayer, int(IPlayer& player, IGangZone& zone))
{
    if (zone.isShownForPlayer(player)) {
        return zone.getFlashingColorForPlayer(player).RGBA();
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

SCRIPT_API(ToggleGangZoneCheck, bool(IGangZone& zone, bool toggle))
{
    IGangZonesComponent* component = PawnManager::Get()->gangzones;
    if (component) {
        component->toggleGangZoneCheck(zone, toggle);
        return true;
    }
    return false;
}
