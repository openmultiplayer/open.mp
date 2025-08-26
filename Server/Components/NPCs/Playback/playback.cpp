/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "playback.hpp"
#include "record_manager.hpp"
#include "../NPC/npc.hpp"
#include "../utils.hpp"

NPCPlayback::NPCPlayback(StringView filePath, StringView playbackPath, bool autoUnload, NPCRecordManager* recordManager)
	: autoUnload_(autoUnload)
	, recordId_(INVALID_RECORD_ID)
	, paused_(false)
	, currentIndex_(0)
	, recordManager_(recordManager)
{
	if (recordManager_)
	{
		String fullPath = String(playbackPath) + String(filePath) + ".rec";
		recordId_ = recordManager_->loadRecord(fullPath);
		if (recordId_ == INVALID_RECORD_ID)
		{
			// Log error: Playback file not found
		}
	}
}

NPCPlayback::NPCPlayback(int recordId, bool autoUnload, NPCRecordManager* recordManager)
	: autoUnload_(autoUnload)
	, recordId_(recordId)
	, paused_(false)
	, currentIndex_(0)
	, recordManager_(recordManager)
{
}

NPCPlayback::~NPCPlayback()
{
	if (autoUnload_ && recordManager_)
	{
		recordManager_->unloadRecord(recordId_);
	}
}

bool NPCPlayback::initialize(const Vector3& startPoint, const GTAQuat& startRotation)
{
	if (!isValid())
	{
		return false;
	}

	recordData_ = recordManager_->getRecord(recordId_);

	if (startPoint != Vector3(0.0f, 0.0f, 0.0f))
	{
		if (recordData_.playbackType == NPCPlaybackType::Driver && !recordData_.vehicleData.empty())
		{
			Vector3 offset = recordData_.vehicleData[0].Position - startPoint;
			adjustPositionOffset(offset);
		}
		else if (recordData_.playbackType == NPCPlaybackType::OnFoot && !recordData_.onFootData.empty())
		{
			Vector3 offset = recordData_.onFootData[0].Position - startPoint;
			adjustPositionOffset(offset);
		}
	}

	if (startRotation.q != GTAQuat().q)
	{
		if (recordData_.playbackType == NPCPlaybackType::Driver && !recordData_.vehicleData.empty())
		{
			GTAQuat rotationOffset;
			rotationOffset.q = recordData_.vehicleData[0].Rotation.q - startRotation.q;
			adjustRotationOffset(rotationOffset);
		}
		else if (recordData_.playbackType == NPCPlaybackType::OnFoot && !recordData_.onFootData.empty())
		{
			GTAQuat rotationOffset;
			rotationOffset.q = recordData_.onFootData[0].Rotation.q - startRotation.q;
			adjustRotationOffset(rotationOffset);
		}
	}

	startTime_ = Time::now();
	currentIndex_ = 0;
	return true;
}

bool NPCPlayback::process(NPC& npc, TimePoint now)
{
	if (currentIndex_ >= recordData_.timeStamps.size())
	{
		return false;
	}

	if (paused_)
	{
		if (recordData_.playbackType == NPCPlaybackType::Driver)
		{
			npc.resetKeys();
		}
		else if (recordData_.playbackType == NPCPlaybackType::OnFoot)
		{
			npc.resetKeys();
		}
		startTime_ = now - recordData_.timeStamps[currentIndex_];
		return true;
	}

	auto elapsed = duration_cast<Milliseconds>(now - startTime_);
	if (elapsed >= recordData_.timeStamps[currentIndex_])
	{
		if (recordData_.playbackType == NPCPlaybackType::Driver)
		{
			if (currentIndex_ < recordData_.vehicleData.size())
			{
				const auto& syncData = recordData_.vehicleData[currentIndex_];

				if (npc.getVehicle())
				{
					npc.setKeys(syncData.UpDown, syncData.LeftRight, syncData.Keys);
					npc.setPosition(syncData.Position, false);
					npc.setRotation(syncData.Rotation, false);
					npc.setVelocity(syncData.Velocity, false);
					npc.setVehicleHealth(syncData.Health);
					npc.setHealth(syncData.PlayerHealthArmour.x);
					npc.setArmour(syncData.PlayerHealthArmour.y);
					npc.setWeapon(syncData.WeaponID);
					npc.useVehicleSiren(syncData.Siren != 0);
					npc.setVehicleGearState(syncData.LandingGear);

					if (npc.getVehicle()->getModel() == 520) // Hydra
					{
						npc.setVehicleHydraThrusters(static_cast<int>(syncData.HydraThrustAngle & 0xFFFF));
					}
					else if (npc.getVehicle()->getModel() == 537 || npc.getVehicle()->getModel() == 538 || npc.getVehicle()->getModel() == 570 || npc.getVehicle()->getModel() == 569 || npc.getVehicle()->getModel() == 449) // Train models
					{
						npc.setVehicleTrainSpeed(syncData.TrainSpeed);
					}
				}
			}
		}
		else if (recordData_.playbackType == NPCPlaybackType::OnFoot)
		{
			if (currentIndex_ < recordData_.onFootData.size())
			{
				const auto& syncData = recordData_.onFootData[currentIndex_];

				npc.setKeys(syncData.UpDown, syncData.LeftRight, syncData.Keys);
				npc.setPosition(syncData.Position, false);
				npc.setRotation(syncData.Rotation, false);
				npc.setHealth(syncData.HealthArmour.x);
				npc.setArmour(syncData.HealthArmour.y);
				npc.setWeapon(syncData.Weapon);
				npc.setVelocity(syncData.Velocity, false);
			}
		}

		currentIndex_++;
	}

	return true;
}

void NPCPlayback::unloadRecord()
{
	if (recordManager_)
	{
		recordManager_->unloadRecord(recordId_);
	}
}

void NPCPlayback::setPaused(bool paused)
{
	paused_ = paused;
}

bool NPCPlayback::isPaused() const
{
	return paused_;
}

bool NPCPlayback::isValid() const
{
	return recordId_ != INVALID_RECORD_ID && recordManager_ && recordManager_->isValidRecord(recordId_);
}

NPCPlaybackType NPCPlayback::getPlaybackType() const
{
	if (isValid())
	{
		return recordManager_->getRecord(recordId_).playbackType;
	}
	return NPCPlaybackType::None;
}

size_t NPCPlayback::getRecordSize() const
{
	if (isValid())
	{
		return recordManager_->getRecord(recordId_).timeStamps.size();
	}
	return 0;
}

bool NPCPlayback::isFinished() const
{
	return currentIndex_ >= recordData_.timeStamps.size();
}

void NPCPlayback::adjustPositionOffset(const Vector3& offset)
{
	if (recordData_.playbackType == NPCPlaybackType::Driver)
	{
		for (auto& data : recordData_.vehicleData)
		{
			data.Position -= offset;
		}
	}
	else if (recordData_.playbackType == NPCPlaybackType::OnFoot)
	{
		for (auto& data : recordData_.onFootData)
		{
			data.Position -= offset;
		}
	}
}

void NPCPlayback::adjustRotationOffset(const GTAQuat& rotationOffset)
{
	if (recordData_.playbackType == NPCPlaybackType::Driver)
	{
		for (auto& data : recordData_.vehicleData)
		{
			data.Rotation.q -= rotationOffset.q;
		}
	}
	else if (recordData_.playbackType == NPCPlaybackType::OnFoot)
	{
		for (auto& data : recordData_.onFootData)
		{
			data.Rotation.q -= rotationOffset.q;
		}
	}
}