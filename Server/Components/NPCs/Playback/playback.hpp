/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once

#include <sdk.hpp>
#include <netcode.hpp>

enum class NPCPlaybackType : int
{
	None = 0,
	Driver = 1,
	OnFoot = 2
};

struct NPCRecord
{
	String filePath;
	NPCPlaybackType playbackType;
	DynamicArray<Milliseconds> timeStamps;
	DynamicArray<NetCode::Packet::PlayerFootSync> onFootData;
	DynamicArray<NetCode::Packet::PlayerVehicleSync> vehicleData;
};

class NPC;
class NPCRecordManager;
class NPCComponent;

class NPCPlayback
{
public:
	NPCPlayback(StringView filePath, StringView playbackPath, bool autoUnload, NPCComponent* npcComponent);
	NPCPlayback(int recordId, bool autoUnload, NPCComponent* npcComponent);
	~NPCPlayback();

	bool initialize(const Vector3& startPoint = Vector3(0.0f, 0.0f, 0.0f), const GTAQuat& startRotation = GTAQuat());
	bool process(NPC& npc, TimePoint now);
	void unloadRecord();
	void setPaused(bool paused);
	bool isPaused() const;
	bool isValid() const;

	int getRecordId() const { return recordId_; }
	NPCPlaybackType getPlaybackType() const;
	size_t getRecordSize() const;
	bool isFinished() const;

private:
	void adjustPositionOffset(const Vector3& offset);
	void adjustRotationOffset(const GTAQuat& rotationOffset);

	bool autoUnload_;
	int recordId_;
	TimePoint startTime_;
	bool paused_;
	NPCRecord recordData_;
	size_t currentIndex_;
	NPCComponent* npcComponent_;
	NPCRecordManager* recordManager_;
};