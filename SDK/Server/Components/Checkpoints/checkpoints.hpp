#pragma once

#include <sdk.hpp>

struct CheckpointEventHandler {
	virtual bool onPlayerEnterCheckpoint(IPlayer& player);
	// TODO: fill with other checkpoint events
};

static const UUID CheckpointsPlugin_UUID = UUID(0x8cfb3183976da215);
struct ICheckpointsPlugin : public IPlugin {
	PROVIDE_UUID(CheckpointsPlugin_UUID);
};