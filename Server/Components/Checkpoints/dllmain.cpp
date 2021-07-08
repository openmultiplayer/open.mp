#include <Server/Components/Checkpoints/checkpoints.hpp>

struct CheckpointsPlugin final : public ICheckpointsPlugin {
	DefaultEventDispatcher<CheckpointEventHandler> eventDispatcher;
	ICore* core;

	// some handler here for entering CPs?

	CheckpointsPlugin()
		// handlers would go here
	{
	}

	void onInit(ICore* c) override {
		core = c;
		// add handlers above here
		// not player related, but would this do something like core->getCheckpoints().getEv...addHandler?
		core->printLn("Hello from checkpoints plugin!");
	}

	const char* pluginName() override {
		return "Checkpoints";
	}

	void free() override {
		delete this;
	}

	~CheckpointsPlugin() {
		// remove handlers that may have been assigned above here
	}
};

PLUGIN_ENTRY_POINT() {
	return new CheckpointsPlugin();
}