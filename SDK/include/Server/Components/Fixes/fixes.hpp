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
	virtual void applyAnimation(IPlayer* player, IActor* actor, AnimationData const* animation) = 0;
	PROVIDE_EXT_UID(FixesData_UID);

	virtual bool sendGameText(StringView message, Milliseconds time, int style) = 0;
	virtual bool hideGameText(int style) = 0;
	virtual bool hasGameText(int style) = 0;
	virtual bool getGameText(int style, Impl::String& message, Milliseconds& time, Milliseconds& remaining) = 0;
};

static const UID FixesComponent_UID = UID(0xb5c615eff0329ff7);
struct IFixesComponent : public IComponent
{
	virtual void clearAnimation(IPlayer* player, IActor* actor) = 0;
	PROVIDE_UID(FixesComponent_UID);

	virtual bool sendGameText(StringView message, Milliseconds time, int style) = 0;
	virtual bool hideGameText(int style) = 0;
};
