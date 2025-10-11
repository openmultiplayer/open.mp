/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once

#include <sdk.hpp>
#include "playback.hpp"
#include <sdk.hpp>

class NPCRecordManager
{
public:
	NPCRecordManager()
		: nextRecordId_(0)
	{
	}
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

	FlatHashMap<int, NPCRecord> records_;
	int nextRecordId_;
	NPCRecord emptyRecord_;
};

#pragma pack(push, 1)

struct LegacyVehicleSyncData
{
	uint16_t vehicleId;
	uint16_t leftRight;
	uint16_t upDown;
	uint16_t keys;
	float quaternion[4];
	Vector3 position;
	Vector3 velocity;
	float health;
	uint8_t playerHealth;
	uint8_t playerArmour;
	uint8_t playerWeaponAndAdditionalKey;
	uint8_t sirenState;
	uint8_t gearState;
	uint16_t trailerId;
	union
	{
		uint32_t hydraThrusterAngle;
		float trainSpeed;
	};
};
static_assert(sizeof(LegacyVehicleSyncData) == 63, "Invalid LegacyVehicleSyncData size");

struct LegacyOnFootSyncData
{
	uint16_t leftRight;
	uint16_t upDown;
	uint16_t keys;
	Vector3 position;
	float quaternion[4];
	uint8_t health;
	uint8_t armour;
	uint8_t weaponAndAdditionalKey;
	uint8_t specialAction;
	Vector3 velocity;
	Vector3 surfingOffsets;
	uint16_t surfingId;
	union
	{
		uint32_t animationData;
		struct
		{
			uint16_t animId;
			uint16_t animFlags;
		};
	};
};
static_assert(sizeof(LegacyOnFootSyncData) == 68, "Invalid LegacyOnFootSyncData size");

#pragma pack(pop)
