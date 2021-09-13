#include "sdk.hpp"

#include "Manager/Manager.hpp"
#include "PluginManager/PluginManager.hpp"
#include "Scripting/Impl.hpp"
#include "utils.hpp"

struct PawnComponent : public IComponent, public CoreEventHandler {
	ICore * core = nullptr;
	Scripting* scriptingInstance;

	UUID getUUID() override {
		return 0x78906cd9f19c36a6;
	}

	StringView componentName() const override {
		return "Pawn";
	}

	SemanticVersion componentVersion() const override {
		return SemanticVersion(0, 0, 0, BUILD_NUMBER);
	}

	void onLoad(ICore * c) override {
		core = c;
		// store core instance and add event handlers
		PawnManager::Get()->core = core;
		PawnManager::Get()->config = &core->getConfig();
		PawnManager::Get()->players = &core->getPlayers();

		PawnPluginManager::Get()->core = core;

		// Set AMXFILE environment variable to "{current_dir}/scriptfiles"
		std::string scriptfilesPath;
		utils::GetCurrentWorkingDirectory(scriptfilesPath);
		int length = scriptfilesPath.length();

#ifdef WIN32
		if (scriptfilesPath[length - 1] != '\\')
		{
			scriptfilesPath.append("\\\0");
		}
		scriptfilesPath.append("scriptfiles\\");
		scriptfilesPath.insert(0, "AMXFILE=");
		_putenv(scriptfilesPath.c_str());
#else
		if (scriptfilesPath[length - 1] != '/')
		{
			scriptfilesPath.append("/\0");
		}
		scriptfilesPath.append("scriptfiles/");
		setenv("AMXFILE", scriptfilesPath.c_str(), 1);
#endif

	}

	void onInit(IComponentList* components) override {
		PawnManager::Get()->actors = components->queryComponent<IActorsComponent>();
		PawnManager::Get()->checkpoints = components->queryComponent<ICheckpointsComponent>();
		PawnManager::Get()->classes = components->queryComponent<IClassesComponent>();
		PawnManager::Get()->console = components->queryComponent<IConsoleComponent>();
		PawnManager::Get()->databases = components->queryComponent<IDatabasesComponent>();
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

		// read values of plugins, entry_file and side_scripts from config file
		IConfig& config = core->getConfig();
		Span<const StringView> plugins = config.getStrings("legacy_plugins");
		StringView entryFile = config.getString("entry_file");
		Span<const StringView> sideScripts = config.getStrings("side_scripts");

		scriptingInstance = new Scripting();
		scriptingInstance->addEvents();

		// load plugins
		for (auto& plugin : plugins) {
			PawnPluginManager::Get()->Load(String(plugin));
		}

		// load scripts
		PawnManager::Get()->Load(String(entryFile), true);
		for (auto& script : sideScripts) {
			PawnManager::Get()->Load(String(script), false);
		}

		core->getEventDispatcher().addEventHandler(this);
	}

	void onTick(Microseconds elapsed, TimePoint now) override {
		PawnPluginManager::Get()->ProcessTick();
	}

	~PawnComponent() {
		if (core) {
			core->getEventDispatcher().removeEventHandler(this);
		}

		if (scriptingInstance) {
			delete scriptingInstance;
		}
	}
} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}