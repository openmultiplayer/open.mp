#include "sdk.hpp"
#include <iostream>
#include "Types.hpp"

SCRIPT_API(GangZoneCreate, int(const Vector2& min, const Vector2& max))
{
	IGangZonesComponent* component = PawnManager::Get()->components->queryComponent<IGangZonesComponent>();
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

SCRIPT_API(GangZoneDestroy, bool(IGangZone* gangzone))
{
	IGangZonesComponent* component = PawnManager::Get()->components->queryComponent<IGangZonesComponent>();
	if (component) {
		component->release(gangzone->getID());
		return true;
	}
	return false;
}

SCRIPT_API(GangZoneShowForPlayer, bool(IPlayer* player, IGangZone* gangzone, uint32_t colour))
{
	gangzone->showForPlayer(*player, Colour::FromRGBA(colour));
	return false;
}

SCRIPT_API(GangZoneShowForAll, bool(IGangZone* gangzone, uint32_t colour))
{
	IPlayerPool& pool = PawnManager::Get()->serverCore->getPlayers();
	for (IPlayer* player : pool.entries()) {
		gangzone->showForPlayer(*player, Colour::FromRGBA(colour));
	}
	return false;
}

SCRIPT_API(GangZoneHideForPlayer, bool(IPlayer* player, IGangZone* gangzone))
{
	gangzone->hideForPlayer(*player);
	return false;
}

SCRIPT_API(GangZoneHideForAll, bool(IGangZone* gangzone))
{
	IPlayerPool& pool = PawnManager::Get()->serverCore->getPlayers();
	for (IPlayer* player : pool.entries()) {
		gangzone->hideForPlayer(*player);
	}
	return false;
}

SCRIPT_API(GangZoneFlashForPlayer, bool(IPlayer* player, IGangZone* gangzone, uint32_t colour))
{
	gangzone->flashForPlayer(*player, Colour::FromRGBA(colour));
	return false;
}

SCRIPT_API(GangZoneFlashForAll, bool(IGangZone* gangzone, uint32_t colour))
{
	IPlayerPool& pool = PawnManager::Get()->serverCore->getPlayers();
	for (IPlayer* player : pool.entries()) {
		gangzone->flashForPlayer(*player, Colour::FromRGBA(colour));
	}
	return false;
}

SCRIPT_API(GangZoneStopFlashForPlayer, bool(IPlayer* player, IGangZone* gangzone))
{
	gangzone->stopFlashForPlayer(*player);
	return false;
}

SCRIPT_API(GangZoneStopFlashForAll, bool(IGangZone* gangzone))
{
	IPlayerPool& pool = PawnManager::Get()->serverCore->getPlayers();
	for (IPlayer* player : pool.entries()) {
		gangzone->stopFlashForPlayer(*player);
	}
	return false;
}
