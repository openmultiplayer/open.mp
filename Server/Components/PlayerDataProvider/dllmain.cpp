#include "playerdata.hpp"

ICore* core;

struct SomePlayerData final : public ISomePlayerData {
	int getSomeInt() override {
		return 420;
	}

	void free() override {
		delete this;
	}
};

struct MyPlugin : public IPlugin, public PlayerEventHandler {
	UUID getUUID() override {
		return 0x44a937350d612dde;
	}

	const char* pluginName() override {
		return "PlayerDataProvider";
	}

	void onConnect(IPlayer& player) override {
		player.addData(new SomePlayerData());
	}
} plugin;

PLUGIN_ENTRY_POINT(ICore* c) {
	core = c;
	core->getPlayers().getEventDispatcher().addEventHandler(&plugin);
	return &plugin;
}
