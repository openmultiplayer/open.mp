#pragma once
#include "player.hpp"

#define STRINGIFY(s) _STRINGIFY(s)
#define _STRINGIFY(s) #s

inline StringView trim(StringView view)
{
	char const* const whitespace = " \t\n\r\f\v";
	const size_t start = view.find_first_not_of(whitespace);
	if (start == StringView::npos)
	{
		return "";
	}
	const size_t end = view.find_last_not_of(whitespace);
	return view.substr(start, end - start + 1);
}

inline bool IsWeaponForTakenDamageValid(int weapon)
{
	switch (weapon)
	{
	case 49: // Run over
	case 50: // Helicopter blades
	case 51: // Explosion
	case 53: // Drowned
	case 54: // Collision
		return true;
	default:
		return (WeaponSlotData(weapon).slot() != INVALID_WEAPON_SLOT);
	}
}
