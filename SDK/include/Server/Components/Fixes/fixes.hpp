/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <sdk.hpp>

static const UID FixesData_UID = UID(0x672d5d6fbb094ef7);
struct IPlayerFixesData : public IExtension
{
	PROVIDE_EXT_UID(FixesData_UID);
};

static const UID FixesComponent_UID = UID(0xb5c615eff0329ff7);
struct IFixesComponent : public IComponent
{
	PROVIDE_UID(FixesComponent_UID);

	bool sendGameTextToAll(StringView message, Milliseconds time, int style);
	bool sendGameTextToPlayer(IPlayer& who, StringView message, Milliseconds time, int style);
};

