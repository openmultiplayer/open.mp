/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef _INCLUDE_FORMATTING_H
#define _INCLUDE_FORMATTING_H

#include <amx/amx.h>
#include <types.hpp>

// Amx Templatized Cell Printf
template <typename D, typename S>
size_t atcprintf(D* buffer, size_t maxlen, const S* format, AMX* amx, cell const* params, int* param);

/// Amx string format which can be cast to StringView
class AmxStringFormatter
{
private:
	StaticArray<char, 8192> buf; ///< The buffer of the formatted string
	int length; ///< The length of the formatted string

public:
	/// Constructor which fills the buffer and length with the formatted string
	AmxStringFormatter(cell const* format, AMX* amx, cell const* params, int paramOffset) noexcept;
	/// True if the formatted string is empty, false otherwise
	bool empty() const noexcept { return (length == 0); }

	/// Convert to StringView for convenience
	operator StringView() const noexcept
	{
		return StringView(buf.data(), length);
	}
};

#endif //_INCLUDE_FORMATTING_H
