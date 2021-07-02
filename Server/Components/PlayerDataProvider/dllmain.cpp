#include "playerdata.hpp"

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

	void onInit(ICore* core) override {
		core->getPlayers().addEventHandler(this);
	}

	IPlayerData* onPlayerDataRequest(IPlayer& player) override {
		return new SomePlayerData();
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
