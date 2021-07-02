#include "someplugin.hpp"

struct MyPlugin : public ISomePlugin {
	const char* pluginName() override {
		return "PluginProvider";
	}

	void onInit(ICore* core) override {

	}

	Vector3 someVec() override {
		return Vector3(1.f, 2.f, 3.f);
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
