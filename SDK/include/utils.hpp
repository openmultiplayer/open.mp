/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "game_types.hpp"

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
		if (weapon < 49 || weapon > 54)
		{
			return false;
		}
	}
	return true;
}
