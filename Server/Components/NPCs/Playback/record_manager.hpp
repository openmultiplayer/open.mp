/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <sdk.hpp>
#include "playback.hpp"
#include <Impl/pool_impl.hpp>

class NPCRecordManager
{
public:
	NPCRecordManager() = default;
	~NPCRecordManager() = default;

	int loadRecord(StringView filePath);
	bool unloadRecord(int recordId);
	bool isValidRecord(int recordId) const;
	int findRecord(StringView filePath) const;
	const NPCRecord& getRecord(int recordId) const;
	size_t getRecordCount() const;
	void unloadAllRecords();

private:
	bool parseRecordFile(StringView filePath, NPCRecord& record);
	
	DynamicArray<NPCRecord> records_;
	NPCRecord emptyRecord_;
};