/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "Server/Components/Recordings/recordings.hpp"

SCRIPT_API(StartRecordingPlayerData, bool(IPlayer& player, int type, std::string const& file))
{
	IPlayerRecordingData* recording = queryExtension<IPlayerRecordingData>(player);
	if (recording)
	{
		recording->start(PlayerRecordingType(type), file);
		return true;
	}
	return false;
}

SCRIPT_API(StopRecordingPlayerData, bool(IPlayer& player))
{
	IPlayerRecordingData* recording = queryExtension<IPlayerRecordingData>(player);
	if (recording)
	{
		recording->stop();
		return true;
	}
	return false;
}
