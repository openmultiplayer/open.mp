#include <sdk.hpp>
#include "../PluginProvider/someplugin.hpp"

ICore* core;

struct MyPlugin : public IPlugin, public CoreEventHandler {
	UUID getUUID() override {
		return 0xab2c1f6bb3c0ef5b;
	}

	const char* pluginName() override {
		return "PluginConsumer";
	}

	void onInit() override {
		ISomePlugin* somePlugin = core->queryPlugin<ISomePlugin>();
		if (somePlugin) {
			vector3 someVec = somePlugin->someVec();
			core->printLn("Some plugin %s returned vector (%f, %f, %f)", somePlugin->pluginName(), someVec.x, someVec.y, someVec.z);
		}
		else {
			core->printLn("Some plugin missing");
		}
	}
} plugin;

PLUGIN_ENTRY_POINT(ICore* c) {
	core = c;
	core->getEventDispatcher().addEventHandler(&plugin);
	return &plugin;
}
