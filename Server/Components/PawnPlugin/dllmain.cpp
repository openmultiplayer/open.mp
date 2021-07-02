#include "sdk.hpp"

struct MyPlugin : public IPlugin, public CoreEventHandler {
	ICore* c = nullptr;

	UUID getUUID() override {
		return 0x78906cd9f19c36a6;
	}

	const char* pluginName() override {
		return "PawnPlugin";
	}

	void onInit(ICore* core) override {
		c = core;
		c->getEventDispatcher().addEventHandler(this);
	}

	void onInit() override {
		c->printLn("Server initiated with SDK version %i", c->getVersion());
	}

	void onTick(uint64_t tick) override {
		if ((tick % 1000)==0) {
			c->printLn("1000 ticks passed");
		}
	}

	~MyPlugin() {
		if (c) {
			c->getEventDispatcher().removeEventHandler(this);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
