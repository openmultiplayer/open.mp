#include <sdk.hpp>
#include "../PlayerDataProvider/playerdata.hpp"

ICore* core;

struct MyPlugin : public IPlugin, public PlayerEventHandler {
	UUID getUUID() override {
		return 0x8ee619785a95627e;
	}

	const char* pluginName() override {
		return "PlayerDataConsumer";
	}

	bool onPlayerRequestSpawn(IPlayer& player) override {
		ISomePlayerData* data = player.queryData<ISomePlayerData>();
		if (data) {
			core->printLn("Player %s with some data %i requested spawn", player.name().c_str(), data->getSomeInt());
		}
		return true;
	}
} plugin;

PLUGIN_ENTRY_POINT(ICore* c) {
	core = c;
	core->getPlayers().getEventDispatcher().addEventHandler(&plugin);
	return &plugin;
}
