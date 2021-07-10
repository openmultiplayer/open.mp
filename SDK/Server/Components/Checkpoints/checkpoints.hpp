#pragma once

#include <sdk.hpp>

/// The type of the checkpoint: https://open.mp/docs/scripting/functions/SetPlayerRaceCheckpoint
/// Note: checkpoint types are not restricted to any type of vehicle/on foot
enum class CheckpointType {
	RACE_NORMAL = 0, // Must have nextPosition, else it shows as RACE_FINISH
	RACE_FINISH, // Must have no nextPosition, else it shows as RACE_NORMAL
	RACE_NOTHING,
	RACE_AIR_NORMAL,
	RACE_AIR_FINISH,
	RACE_AIR_ONE,
	RACE_AIR_TWO,
	RACE_AIR_THREE,
	RACE_AIR_FOUR,
	STANDARD
};

static const UUID CheckpointData_UUID = UUID(0xbc07576aa3591a66);
struct IPlayerCheckpointData : public IPlayerData {
	PROVIDE_UUID(CheckpointData_UUID)

	virtual CheckpointType getType() const = 0;
	virtual void setType(const CheckpointType type) = 0;
	virtual Vector3 getPosition() const = 0;
	virtual void setPosition(const Vector3 position) = 0;
	virtual Vector3 getNextPosition() const = 0;
	virtual void setNextPosition(const Vector3 nextPosition) = 0;
	virtual float getSize() const = 0;
	virtual void setSize(const float radius) = 0;
	virtual bool hasPlayerInside() const = 0;
	virtual void setPlayerInside(const bool inside) = 0;

	virtual void enable(IPlayer& player) = 0;
	virtual void disable(IPlayer& player) = 0;
};

struct PlayerCheckpointEventHandler {
	virtual void onPlayerEnterCheckpoint(IPlayer& player) {}
	virtual void onPlayerLeaveCheckpoint(IPlayer& player) {}
	virtual void onPlayerEnterRaceCheckpoint(IPlayer& player) {}
	virtual void onPlayerLeaveRaceCheckpoint(IPlayer& player) {}
};

static const UUID CheckpointsPlugin_UUID = UUID(0x44a937350d611dde);
struct ICheckpointsPlugin : public IPlugin {
	PROVIDE_UUID(CheckpointsPlugin_UUID)

	// Access to event dispatchers for other components to add handlers to
	virtual IEventDispatcher<PlayerCheckpointEventHandler>& getCheckpointDispatcher() = 0;
};