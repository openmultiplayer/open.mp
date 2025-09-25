/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "Manager/Manager.hpp"
#include "PluginManager/PluginManager.hpp"
#include "Scripting/Impl.hpp"
#include "Server/Components/Pawn/pawn.hpp"
#include <ghc/filesystem.hpp>
#include <stdlib.h>

static StaticArray<void*, NUM_AMX_FUNCS> AMX_FUNCTIONS = {
	reinterpret_cast<void*>(&amx_Align16),
	reinterpret_cast<void*>(&amx_Align32),
#if defined _I64_MAX || defined HAVE_I64
	reinterpret_cast<void*>(&amx_Align64),
#else
	nullptr,
#endif
	reinterpret_cast<void*>(&amx_Allot),
	reinterpret_cast<void*>(&amx_Callback),
	reinterpret_cast<void*>(&amx_Cleanup),
	reinterpret_cast<void*>(&amx_Clone),
	reinterpret_cast<void*>(&amx_Exec),
	reinterpret_cast<void*>(&amx_FindNative),
	reinterpret_cast<void*>(&amx_FindPublic),
	reinterpret_cast<void*>(&amx_FindPubVar),
	reinterpret_cast<void*>(&amx_FindTagId),
	reinterpret_cast<void*>(&amx_Flags),
	reinterpret_cast<void*>(&amx_GetAddr),
	reinterpret_cast<void*>(&amx_GetNative),
	reinterpret_cast<void*>(&amx_GetPublic),
	reinterpret_cast<void*>(&amx_GetPubVar),
	reinterpret_cast<void*>(&amx_GetString),
	reinterpret_cast<void*>(&amx_GetTag),
	reinterpret_cast<void*>(&amx_GetUserData),
	reinterpret_cast<void*>(&amx_Init),
	reinterpret_cast<void*>(&amx_InitJIT),
	reinterpret_cast<void*>(&amx_MemInfo),
	reinterpret_cast<void*>(&amx_NameLength),
	reinterpret_cast<void*>(&amx_NativeInfo),
	reinterpret_cast<void*>(&amx_NumNatives),
	reinterpret_cast<void*>(&amx_NumPublics),
	reinterpret_cast<void*>(&amx_NumPubVars),
	reinterpret_cast<void*>(&amx_NumTags),
	reinterpret_cast<void*>(&amx_Push),
	reinterpret_cast<void*>(&amx_PushArray),
	reinterpret_cast<void*>(&amx_PushString),
	reinterpret_cast<void*>(&amx_RaiseError),
	reinterpret_cast<void*>(&amx_Register),
	reinterpret_cast<void*>(&amx_Release),
	reinterpret_cast<void*>(&amx_SetCallback),
	reinterpret_cast<void*>(&amx_SetDebugHook),
	reinterpret_cast<void*>(&amx_SetString),
	reinterpret_cast<void*>(&amx_SetUserData),
	reinterpret_cast<void*>(&amx_StrLen),
	reinterpret_cast<void*>(&amx_UTF8Check),
	reinterpret_cast<void*>(&amx_UTF8Get),
	reinterpret_cast<void*>(&amx_UTF8Len),
	reinterpret_cast<void*>(&amx_UTF8Put),
	reinterpret_cast<void*>(&amx_PushStringLen),
	reinterpret_cast<void*>(&amx_SetStringLen),
#if PAWN_CELL_SIZE == 16
	reinterpret_cast<void*>(&amx_Swap16),
#else
	nullptr,
#endif
#if PAWN_CELL_SIZE == 32
	reinterpret_cast<void*>(&amx_Swap32),
#else
	nullptr,
#endif
#if PAWN_CELL_SIZE == 64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
	reinterpret_cast<void*>(&amx_Swap64),
#else
	nullptr,
#endif
	reinterpret_cast<void*>(&amx_GetNativeByIndex),
	reinterpret_cast<void*>(&amx_MakeAddr),
	reinterpret_cast<void*>(&amx_StrSize),
};

class PawnComponent final : public IPawnComponent, public CoreEventHandler, public ConsoleEventHandler
{
private:
	ICore* core = nullptr;
	Scripting scriptingInstance;

public:
	StringView componentName() const override
	{
		return "Pawn";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	IEventDispatcher<PawnEventHandler>& getEventDispatcher() override
	{
		return PawnManager::Get()->eventDispatcher;
	}

	void onLoad(ICore* c) override
	{
		core = c;
		// store core instance and add event handlers
		PawnManager::Get()->core = core;
		PawnManager::Get()->config = &core->getConfig();
		PawnManager::Get()->players = &core->getPlayers();
		PawnManager::Get()->pluginManager.core = core;
		core->getEventDispatcher().addEventHandler(this);

		// Set AMXFILE environment variable to "{current_dir}/scriptfiles"
		ghc::filesystem::path scriptfilesPath = ghc::filesystem::absolute("scriptfiles");
		if (!ghc::filesystem::exists(scriptfilesPath) || !ghc::filesystem::is_directory(scriptfilesPath))
		{
			ghc::filesystem::create_directory(scriptfilesPath);
		}

#if defined(GHC_USE_WCHAR_T)
		std::wstring wstr_path = scriptfilesPath.wstring();
		std::wstring::size_type size = wstr_path.size();

		wchar_t* path = new wchar_t[size + 1];
		memcpy((void*)path, (void*)wstr_path.c_str(), (size + 1) * sizeof(wchar_t));

		_wputenv_s(L"AMXFILE", path);
#else
		std::string amxFileEnvVar = scriptfilesPath.string();

		amxFileEnvVar.insert(0, "AMXFILE=");

		// putenv() must own the string, so we aren't actually leaking it
		const std::string::size_type size = amxFileEnvVar.size();
		char* amxFileEnvVarCString = new char[size + 1];
		memcpy(amxFileEnvVarCString, amxFileEnvVar.c_str(), size + 1);

		putenv(amxFileEnvVarCString);
#endif
	}

	void onInit(IComponentList* components) override
	{
		PawnManager* mgr = PawnManager::Get();

		mgr->actors = components->queryComponent<IActorsComponent>();
		mgr->checkpoints = components->queryComponent<ICheckpointsComponent>();
		mgr->classes = components->queryComponent<IClassesComponent>();
		mgr->console = components->queryComponent<IConsoleComponent>();
		mgr->databases = components->queryComponent<IDatabasesComponent>();
		mgr->dialogs = components->queryComponent<IDialogsComponent>();
		mgr->fixes = components->queryComponent<IFixesComponent>();
		mgr->gangzones = components->queryComponent<IGangZonesComponent>();
		mgr->menus = components->queryComponent<IMenusComponent>();
		mgr->objects = components->queryComponent<IObjectsComponent>();
		mgr->pickups = components->queryComponent<IPickupsComponent>();
		mgr->recordings = components->queryComponent<IRecordingsComponent>();
		mgr->textdraws = components->queryComponent<ITextDrawsComponent>();
		mgr->textlabels = components->queryComponent<ITextLabelsComponent>();
		mgr->timers = components->queryComponent<ITimersComponent>();
		mgr->vars = components->queryComponent<IVariablesComponent>();
		mgr->vehicles = components->queryComponent<IVehiclesComponent>();
		mgr->models = components->queryComponent<ICustomModelsComponent>();
		mgr->npcs = components->queryComponent<INPCComponent>();

		scriptingInstance.addEvents();

		if (mgr->console)
		{
			mgr->console->getEventDispatcher().addEventHandler(this);
		}
	}

	void onReady() override
	{
		PawnManager* mgr = PawnManager::Get();
		PawnPluginManager& pluginMgr = PawnManager::Get()->pluginManager;

		// read values of plugins, main_scripts and side_scripts from config file
		IConfig& config = core->getConfig();

		// load plugins
		DynamicArray<StringView> plugins(config.getStringsCount("pawn.legacy_plugins"));
		config.getStrings("pawn.legacy_plugins", Span<StringView>(plugins.data(), plugins.size()));
		for (auto& plugin : plugins)
		{
			pluginMgr.Load(String(plugin));
		}

		// load scripts
		DynamicArray<StringView> sideScripts(config.getStringsCount("pawn.side_scripts"));
		config.getStrings("pawn.side_scripts", Span<StringView>(sideScripts.data(), sideScripts.size()));
		for (auto& script : sideScripts)
		{
			mgr->Load(String(script), false);
		}

		DynamicArray<StringView> mainScripts(config.getStringsCount("pawn.main_scripts"));
		config.getStrings("pawn.main_scripts", Span<StringView>(mainScripts.data(), mainScripts.size()));
		mgr->Load(mainScripts);
	}

	const StaticArray<void*, NUM_AMX_FUNCS>& getAmxFunctions() const override
	{
		return AMX_FUNCTIONS;
	}

	IPawnScript const* getScript(AMX* amx) const override
	{
		auto& amx_map = PawnManager::Get()->amxToScript_;
		auto script_itr = amx_map.find(amx);
		if (script_itr != amx_map.end())
		{
			return script_itr->second;
		}
		return nullptr;
	}

	IPawnScript* getScript(AMX* amx) override
	{
		auto& amx_map = PawnManager::Get()->amxToScript_;
		auto script_itr = amx_map.find(amx);
		if (script_itr != amx_map.end())
		{
			return script_itr->second;
		}
		return nullptr;
	}

	const Span<IPawnScript*> sideScripts() override
	{
		return PawnManager::Get()->scripts_;
	}

	IPawnScript* mainScript() override
	{
		return PawnManager::Get()->mainScript_;
	}

	void onConsoleCommandListRequest(FlatHashSet<StringView>& commands) override
	{
		PawnManager::Get()->OnServerCommandList(commands);
	}

	bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override
	{
		return PawnManager::Get()->OnServerCommand(sender, String(command), String(parameters));
	}

	void onTick(Microseconds elapsed, TimePoint now) override
	{
		PawnManager::Get()->pluginManager.ProcessTick();
		PawnManager::Get()->ProcessTick(elapsed, now);
	}

	void onFree(IComponent* component) override
	{
#define COMPONENT_UNLOADED(var) \
	if (component == var)       \
		var = nullptr;

		PawnManager* mgr = PawnManager::Get();

		COMPONENT_UNLOADED(mgr->actors)
		COMPONENT_UNLOADED(mgr->console)
		COMPONENT_UNLOADED(mgr->checkpoints)
		COMPONENT_UNLOADED(mgr->classes)
		COMPONENT_UNLOADED(mgr->databases)
		COMPONENT_UNLOADED(mgr->dialogs)
		COMPONENT_UNLOADED(mgr->fixes)
		COMPONENT_UNLOADED(mgr->gangzones)
		COMPONENT_UNLOADED(mgr->menus)
		COMPONENT_UNLOADED(mgr->objects)
		COMPONENT_UNLOADED(mgr->pickups)
		COMPONENT_UNLOADED(mgr->recordings)
		COMPONENT_UNLOADED(mgr->textdraws)
		COMPONENT_UNLOADED(mgr->textlabels)
		COMPONENT_UNLOADED(mgr->timers)
		COMPONENT_UNLOADED(mgr->vars)
		COMPONENT_UNLOADED(mgr->vehicles)
		COMPONENT_UNLOADED(mgr->models)
		COMPONENT_UNLOADED(mgr->npcs)
	}

	void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) override
	{
		if (defaults)
		{
			StringView scripts[] = { "test 1" };
			config.setStrings("pawn.main_scripts", Span<StringView>(scripts, 1));
			config.setStrings("pawn.side_scripts", Span<StringView>());
			config.setStrings("pawn.legacy_plugins", Span<StringView>());
		}
	}

	~PawnComponent()
	{
		if (core)
		{
			core->getEventDispatcher().removeEventHandler(this);
		}
		if (PawnManager::Get()->console)
		{
			PawnManager::Get()->console->getEventDispatcher().removeEventHandler(this);
		}
		PawnManager::Destroy();
	}

	void free() override { delete this; }

	void reset() override
	{
		// Nothing to reset here.  This component did the resetting in the first place.
	}
};

COMPONENT_ENTRY_POINT()
{
	return new PawnComponent();
}

PawnLookup* getAmxLookups()
{
	return PawnManager::Get();
}
