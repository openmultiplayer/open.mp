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

		// store core instance and add event handlers
		PawnManager::Get()->core = core;
		PawnManager::Get()->players = &core->getPlayers();
		Scripting scriptingInstance = Scripting(core);
		scriptingInstance.addEvents();

		// load scripts
		PawnManager::Get()->Load(String(entryFile), true);
		for (auto& script : sideScripts) {
			PawnManager::Get()->Load(String(script), false);
		}

		core->getEventDispatcher().addEventHandler(this);
	}

	void onInit(IComponentList* components) override {
		PawnManager::Get()->actors = components->queryComponent<IActorsComponent>();
		PawnManager::Get()->checkpoints = components->queryComponent<ICheckpointsComponent>();
		PawnManager::Get()->classes = components->queryComponent<IClassesComponent>();
		PawnManager::Get()->console = components->queryComponent<IConsoleComponent>();
		PawnManager::Get()->dialogs = components->queryComponent<IDialogsComponent>();
		PawnManager::Get()->gangzones = components->queryComponent<IGangZonesComponent>();
		PawnManager::Get()->menus = components->queryComponent<IMenusComponent>();
		PawnManager::Get()->objects = components->queryComponent<IObjectsComponent>();
		PawnManager::Get()->pickups = components->queryComponent<IPickupsComponent>();
		PawnManager::Get()->textdraws = components->queryComponent<ITextDrawsComponent>();
		PawnManager::Get()->textlabels = components->queryComponent<ITextLabelsComponent>();
		PawnManager::Get()->timers = components->queryComponent<ITimersComponent>();
		PawnManager::Get()->vars = components->queryComponent<IVariablesComponent>();
		PawnManager::Get()->vehicles = components->queryComponent<IVehiclesComponent>();
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