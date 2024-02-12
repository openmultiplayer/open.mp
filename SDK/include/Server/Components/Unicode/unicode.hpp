/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <component.hpp>
#include <types.hpp>

static const UID UnicodeComponent_UID = UID(0x8ffb446f8353922b);
struct IUnicodeComponent : public IComponent
{
	PROVIDE_UID(UnicodeComponent_UID);

	virtual OptimisedString toUTF8(StringView input) = 0;
};
