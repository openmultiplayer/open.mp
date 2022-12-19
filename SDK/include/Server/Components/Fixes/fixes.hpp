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

	/// Send a game text message to the player
	virtual bool sendGameText(StringView message, Milliseconds time, int style) = 0;

	/// Hide a game text message from the player
	virtual bool hideGameText(int style) = 0;

	/// Check if the player can currently see this game text.
	virtual bool hasGameText(int style) = 0;

	/// Get the data for this gametext, if they have one.
	virtual bool getGameText(int style, StringView& message, Milliseconds& time, Milliseconds& remaining) = 0;

	// Apply animation to fix animation library not being loaded for the first time
	virtual void applyAnimation(IPlayer* player, IActor* actor, AnimationData const* animation) = 0;
};

static const UID FixesComponent_UID = UID(0xb5c615eff0329ff7);
struct IFixesComponent : public IComponent
{
	PROVIDE_UID(FixesComponent_UID);

	/// sendGameText for all players
	virtual bool sendGameTextToAll(StringView message, Milliseconds time, int style) = 0;

	/// hideGameText for all players
	virtual bool hideGameTextForAll(int style) = 0;

	// Cleat a player's or an actor's animation reapply storage
	virtual void clearAnimation(IPlayer* player, IActor* actor) = 0;
};
