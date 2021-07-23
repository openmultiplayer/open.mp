#include "sdk.hpp"

#include "Manager/Manager.hpp"
#include "Scripting/Impl.hpp"

struct PawnPlugin : public IPlugin, public CoreEventHandler {
	ICore * c = nullptr;

	UUID getUUID() override {
		return 0x78906cd9f19c36a6;
	}

	const char * pluginName() override {
		return "Pawn";
	}

	void onLoad(ICore * core) override {
		c = core;

		// read values of entry_file and side_scripts from config file
		const JSON & props = c->getProperties();
		std::string entryFile = Config::getOption<std::string>(props, "entry_file");
		std::vector<std::string> sideScripts = Config::getOption<std::vector<std::string>>(props, "side_scripts");

		// store core instance and add event handlers
		PawnManager::Get()->SetServerCoreInstance(c);
		Scripting scriptingInstance = Scripting(core);
		scriptingInstance.addEvents();

		// load scripts
		PawnManager::Get()->Load(entryFile, true);
		for (auto & script : sideScripts) {
			PawnManager::Get()->Load(script, false);
		}

		c->getEventDispatcher().addEventHandler(this);
	}

	void onTick(std::chrono::microseconds elapsed) override {
	}

	~PawnPlugin() {
		if (c) {
			c->getEventDispatcher().removeEventHandler(this);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}