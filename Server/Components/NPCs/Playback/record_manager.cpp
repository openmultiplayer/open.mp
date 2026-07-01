/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#include "record_manager.hpp"
#include <fstream>

int NPCRecordManager::loadRecord(StringView filePath)
{
	int existingIndex = findRecord(filePath);
	if (existingIndex != INVALID_RECORD_ID)
	{
		return existingIndex;
	}

	NPCRecord record;
	if (!parseRecordFile(filePath, record))
	{
		return INVALID_RECORD_ID;
	}

	int recordId = nextRecordId_++;
	records_[recordId] = std::move(record);
	return recordId;
}

bool NPCRecordManager::unloadRecord(int recordId)
{
	if (!isValidRecord(recordId))
	{
		return false;
	}

	records_.erase(recordId);
	return true;
}

bool NPCRecordManager::isValidRecord(int recordId) const
{
	return records_.find(recordId) != records_.end();
}

int NPCRecordManager::findRecord(StringView filePath) const
{
	for (const auto& pair : records_)
	{
		if (pair.second.filePath == filePath)
		{
			return pair.first;
		}
	}
	return INVALID_RECORD_ID;
}

const NPCRecord& NPCRecordManager::getRecord(int recordId) const
{
	auto it = records_.find(recordId);
	if (it != records_.end())
	{
		return it->second;
	}
	return emptyRecord_;
}

size_t NPCRecordManager::getRecordCount() const
{
	return records_.size();
}

void NPCRecordManager::unloadAllRecords()
{
	records_.clear();
	nextRecordId_ = 0;
}

bool NPCRecordManager::parseRecordFile(StringView filePath, NPCRecord& record)
{
	std::ifstream file(String(filePath).data(), std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	if (fileSize == 0)
	{
		file.close();
		return false;
	}

	record.filePath = String(filePath);

	uint32_t fileSignature;
	int playbackType;

	file.read(reinterpret_cast<char*>(&fileSignature), sizeof(uint32_t));
	if (!file.good())
	{
		file.close();
		return false;
	}

	file.read(reinterpret_cast<char*>(&playbackType), sizeof(int));
	if (!file.good())
	{
		file.close();
		return false;
	}

	if (playbackType != static_cast<int>(NPCPlaybackType::Driver) && playbackType != static_cast<int>(NPCPlaybackType::OnFoot))
	{
		file.close();
		return false;
	}

	record.playbackType = static_cast<NPCPlaybackType>(playbackType);

	uint32_t timeStamp;

	if (record.playbackType == NPCPlaybackType::Driver)
	{
		LegacyVehicleSyncData legacyData;

		while (file.read(reinterpret_cast<char*>(&timeStamp), sizeof(uint32_t)))
		{
			if (!file.read(reinterpret_cast<char*>(&legacyData), sizeof(LegacyVehicleSyncData)))
			{
				break;
			}

			record.timeStamps.emplace_back(timeStamp);

			NetCode::Packet::PlayerVehicleSync syncData;
			syncData.VehicleID = legacyData.vehicleId;
			syncData.LeftRight = legacyData.leftRight;
			syncData.UpDown = legacyData.upDown;
			syncData.Keys = legacyData.keys;
			syncData.Position = legacyData.position;
			syncData.Rotation = GTAQuat(legacyData.quaternion[0], legacyData.quaternion[1],
				legacyData.quaternion[2], legacyData.quaternion[3]);
			syncData.Velocity = legacyData.velocity;
			syncData.Health = legacyData.health;
			syncData.PlayerHealthArmour.x = legacyData.playerHealth;
			syncData.PlayerHealthArmour.y = legacyData.playerArmour;
			syncData.AdditionalKeyWeapon = legacyData.playerWeaponAndAdditionalKey;
			syncData.Siren = legacyData.sirenState;
			syncData.LandingGear = legacyData.gearState;
			syncData.HydraThrustAngle = legacyData.hydraThrusterAngle;
			syncData.TrainSpeed = legacyData.trainSpeed;
			syncData.TrailerID = 0xFFFF;
			syncData.HasTrailer = false;

			record.vehicleData.emplace_back(syncData);
		}
	}
	else if (record.playbackType == NPCPlaybackType::OnFoot)
	{
		LegacyOnFootSyncData legacyData;

		while (file.read(reinterpret_cast<char*>(&timeStamp), sizeof(uint32_t)))
		{
			if (!file.read(reinterpret_cast<char*>(&legacyData), sizeof(LegacyOnFootSyncData)))
			{
				break;
			}

			record.timeStamps.emplace_back(timeStamp);

			NetCode::Packet::PlayerFootSync syncData;
			syncData.LeftRight = legacyData.leftRight;
			syncData.UpDown = legacyData.upDown;
			syncData.Keys = legacyData.keys;
			syncData.Position = legacyData.position;
			syncData.Rotation = GTAQuat(legacyData.quaternion[0], legacyData.quaternion[1],
				legacyData.quaternion[2], legacyData.quaternion[3]);
			syncData.HealthArmour.x = legacyData.health;
			syncData.HealthArmour.y = legacyData.armour;
			syncData.WeaponAdditionalKey = legacyData.weaponAndAdditionalKey;
			syncData.SpecialAction = legacyData.specialAction;
			syncData.Velocity = legacyData.velocity;
			syncData.SurfingData.offset = legacyData.surfingOffsets;
			syncData.SurfingData.ID = legacyData.surfingId;
			syncData.AnimationFlags = legacyData.animFlags;
			syncData.AnimationID = legacyData.animId;

			if (syncData.SurfingData.ID < 1)
			{
				syncData.SurfingData.type = PlayerSurfingData::Type::None;
			}
			else if (syncData.SurfingData.ID < VEHICLE_POOL_SIZE)
			{
				syncData.SurfingData.type = PlayerSurfingData::Type::Vehicle;
			}
			else if (syncData.SurfingData.ID < VEHICLE_POOL_SIZE + OBJECT_POOL_SIZE)
			{
				syncData.SurfingData.ID -= VEHICLE_POOL_SIZE;
				syncData.SurfingData.type = PlayerSurfingData::Type::Object;
			}
			else
			{
				syncData.SurfingData.type = PlayerSurfingData::Type::None;
			}

			record.onFootData.emplace_back(syncData);
		}
	}

	file.close();
	return true;
}