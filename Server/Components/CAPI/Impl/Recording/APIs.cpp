/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Recording_Start, bool(objectPtr player, int type, StringCharPtr file))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerRecordingData* recording = queryExtension<IPlayerRecordingData>(player_);
	if (recording)
	{
		recording->start(PlayerRecordingType(type), file);
		return true;
	}
	return false;
}

OMP_CAPI(Recording_Stop, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerRecordingData* recording = queryExtension<IPlayerRecordingData>(player_);
	if (recording)
	{
		recording->stop();
		return true;
	}
	return false;
}
