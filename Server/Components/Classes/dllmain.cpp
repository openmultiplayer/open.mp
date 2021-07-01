#include "classes_impl.hpp"

struct PlayerClassData final : IPlayerClassData {
	Class cls;
	IClass& getClass() override {
		return cls;
	}

	void free() override {
		delete this;
	}
};

struct ClassesPlugin final : public IClassesPlugin, public PlayerEventHandler {
	ClassPool classes;
	ICore& core;

	ClassesPlugin(ICore& core) :
		classes(core),
		core(core)
	{
		core.getPlayers().getEventDispatcher().addEventHandler(this);
	}

	IClassPool& getClasses() override {
		return classes;
	}

	const char* pluginName() override {
		return "Classes";
	}

	IPlayerData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerClassData();
	}

	void free() override {
		delete this;
	}

	~ClassesPlugin() {
		core.getPlayers().getEventDispatcher().removeEventHandler(this);
	}
};

PLUGIN_ENTRY_POINT(ICore* core) {
	return new ClassesPlugin(*core);
}
