/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "../../Manager/Manager.hpp"
#include "sdk.hpp"

struct ClassEvents : public ClassEventHandler, public Singleton<ClassEvents>
{
	bool onPlayerRequestClass(IPlayer& player, unsigned int classId) override
	{
		// only return value of the one in entry script (gamdemode) matters
		return !!PawnManager::Get()->CallAllInSidesFirst("OnPlayerRequestClass", DefaultReturnValue_True, player.getID(), classId);
	}
};
