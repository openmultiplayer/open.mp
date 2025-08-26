/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
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

	records_.emplace_back(std::move(record));
	return static_cast<int>(records_.size() - 1);
}

bool NPCRecordManager::unloadRecord(int recordId)
{
	if (!isValidRecord(recordId))
	{
		return false;
	}

	records_.erase(records_.begin() + recordId);
	return true;
}

bool NPCRecordManager::isValidRecord(int recordId) const
{
	return recordId >= 0 && recordId < static_cast<int>(records_.size());
}

int NPCRecordManager::findRecord(StringView filePath) const
{
	for (size_t i = 0; i < records_.size(); ++i)
	{
		if (records_[i].filePath == filePath)
		{
			return static_cast<int>(i);
		}
	}
	return INVALID_RECORD_ID;
}

const NPCRecord& NPCRecordManager::getRecord(int recordId) const
{
	if (isValidRecord(recordId))
	{
		return records_[recordId];
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
			syncData.WeaponID = legacyData.playerWeaponAndAdditionalKey;
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
			syncData.Weapon = legacyData.weaponAndAdditionalKey;
			syncData.SpecialAction = legacyData.specialAction;
			syncData.Velocity = legacyData.velocity;
			syncData.SurfingData.offset = legacyData.surfingOffsets;
			syncData.SurfingData.ID = legacyData.surfingId;

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