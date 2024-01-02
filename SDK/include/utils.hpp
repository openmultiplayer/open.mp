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
	auto slot = WeaponSlotData(weapon).slot();
	if (slot == INVALID_WEAPON_SLOT)
	{
		// 52 is a valid damage reason, though not a valid death reason.  It is given by the police boat gun.
		//
		//   https://github.com/openmultiplayer/open.mp/issues/600#issuecomment-1377236916
		//
		if (weapon < 49 || weapon > 54)
		{
			return false;
		}
	}
	return true;
}
