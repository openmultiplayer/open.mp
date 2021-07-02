#include <sdk.hpp>
#include "../PlayerDataProvider/playerdata.hpp"

struct MyPlugin : public IPlugin, public PlayerEventHandler {
	ICore* c = nullptr;

	UUID getUUID() override {
		return 0x8ee619785a95627e;
	}

	const char* pluginName() override {
		return "PlayerDataConsumer";
	}

	void onInit(ICore* core) override {
		c = core;
		c->getPlayers().getEventDispatcher().addEventHandler(this);
	}

	bool onPlayerRequestSpawn(IPlayer& player) override {
		ISomePlayerData* data = player.queryData<ISomePlayerData>();
		if (data) {
			c->printLn("Player %s with some data %i requested spawn", player.name().c_str(), data->getSomeInt());
		}
		return true;
	}

	~MyPlugin() {
		if (c) {
			c->getPlayers().getEventDispatcher().removeEventHandler(this);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
