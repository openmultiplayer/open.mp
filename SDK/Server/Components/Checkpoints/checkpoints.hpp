#pragma once

#include <sdk.hpp>

static const UUID CheckpointData_UUID = UUID(0xbc07576aa3591a66);
struct IPlayerCheckpointData : public IPlayerData {
	PROVIDE_UUID(CheckpointData_UUID)

	virtual IPlayer& getPlayer() const = 0;
	virtual Vector3 getPosition() const = 0;
	virtual void setPosition(const Vector3 position) = 0;
	virtual float getSize() const = 0;
	virtual void setSize(const float radius) = 0;
	virtual bool hasPlayerInside() const = 0;
	virtual void setPlayerInside(const bool inside) = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;
};

struct PlayerCheckpointEventHandler {
	virtual void onPlayerEnterCheckpoint(IPlayer& player) {}
	virtual void onPlayerLeaveCheckpoint(IPlayer& player) {}
};

static const UUID CheckpointsPlugin_UUID = UUID(0x44a937350d611dde);
struct ICheckpointsPlugin : public IPlugin {
	PROVIDE_UUID(CheckpointsPlugin_UUID)

	// Access to event dispatchers for other components to add handlers to
	virtual DefaultEventDispatcher<PlayerCheckpointEventHandler>& getCheckpointDispatcher() = 0;

	// Wrappers
	virtual void setPlayerCheckpoint(const IPlayer& player, const Vector3 position, const float size) = 0;
	virtual void disablePlayerCheckpoint(const IPlayer& player) = 0;

};