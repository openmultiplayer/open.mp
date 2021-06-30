#include "sdk.hpp"

ICore* core;

struct MyPlugin : public IPlugin, public CoreEventHandler {
	UUID getUUID() override {
		return 0x78906cd9f19c36a6;
	}

	const char* pluginName() override {
		return "PawnPlugin";
	}

	void onInit() override {
		core->printLn("Server initiated with SDK version %i", core->getVersion());
	}

	void onTick(uint64_t tick) override {
		if ((tick % 1000)==0) {
			core->printLn("1000 ticks passed");
		}
	}
} plugin;

PLUGIN_ENTRY_POINT(ICore* c) {
	core = c;
	core->getEventDispatcher().addEventHandler(&plugin);
	return &plugin;
}
