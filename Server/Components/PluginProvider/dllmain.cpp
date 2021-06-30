#include "someplugin.hpp"

ICore* core;

struct MyPlugin : public ISomePlugin {
	const char* pluginName() override {
		return "PluginProvider";
	}

	vector3 someVec() override {
		return vector3(1.f, 2.f, 3.f);
	}
} plugin;

PLUGIN_ENTRY_POINT(ICore* c) {
	core = c;
	return &plugin;
}
