#pragma once

#include <component.hpp>

/// The type of the recording: https://open.mp/docs/scripting/resources/recordtypes
enum PlayerRecordingType
{
	PlayerRecordingType_None,
	PlayerRecordingType_Driver,
	PlayerRecordingType_OnFoot
};

static const UID RecordingData_UID = UID(0x34DB532857286482);
struct IPlayerRecordingData : public IExtension
{
	PROVIDE_EXT_UID(RecordingData_UID);

	/// Start recording the player's data to a file
	virtual void start(PlayerRecordingType type, StringView file) = 0;

	/// Stop recording the player's data to a file
	virtual void stop() = 0;
};

static const UID RecordingsComponent_UID = UID(0x871144D399F5F613);
struct IRecordingsComponent : public IComponent
{
	PROVIDE_UID(RecordingsComponent_UID);
};
