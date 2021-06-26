#include "sdk.hpp"

struct MyEventHandler : public CoreEventHandler {
	ICore* core;

	void onInit() override {
		core->printLn("Server initiated");
	}

	void onTick(uint64_t tick) override {
		if ((tick % 1000)==0) {
			core->printLn("1000 ticks passed");
		}
	}
} eventHandler;

PLUGIN_ENTRY_POINT(ICore* core) {
	eventHandler.core = core;
	core->printLn("Version: %i", core->getVersion());
	core->getEventDispatcher().addEventHandler(&eventHandler);
	return true;
}
