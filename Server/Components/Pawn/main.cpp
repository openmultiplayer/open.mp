#include "sdk.hpp"

#include "Manager/Manager.hpp"
#include "Scripting/Impl.hpp"

struct PawnComponent : public IComponent, public CoreEventHandler {
	ICore * core = nullptr;

	UUID getUUID() override {
		return 0x78906cd9f19c36a6;
	}

	StringView componentName() override {
		return "Pawn";
	}

	void onLoad(ICore * c) override {
		core = c;
	}

	void onInit(IComponentList* components) override {
		// read values of entry_file and side_scripts from config file
		IConfig& config = core->getConfig();
		StringView entryFile = config.getString("entry_file");
		Span<const StringView> sideScripts = config.getStrings("side_scripts");

		// store core instance and get component list
		PawnManager::Get()->SetComponentList(components);
		PawnManager::Get()->SetServerCoreInstance(core);

		// add event handlers
		Scripting scriptingInstance = Scripting(core);
		scriptingInstance.addEvents();

		// load scripts
		PawnManager::Get()->Load(String(entryFile), true);
		for (auto& script : sideScripts) {
			PawnManager::Get()->Load(String(script), false);
		}

		core->getEventDispatcher().addEventHandler(this);
	}

	void onTick(std::chrono::microseconds elapsed) override {
	}

	~PawnComponent() {
		if (core) {
			core->getEventDispatcher().removeEventHandler(this);
		}
	}
} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}