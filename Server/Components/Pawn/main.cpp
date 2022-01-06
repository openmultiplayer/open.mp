#include "Manager/Manager.hpp"
#include "PluginManager/PluginManager.hpp"
#include "Scripting/Impl.hpp"
#include "Server/Components/Pawn/pawn.hpp"
#include <filesystem>
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
};

struct PawnComponent : public IPawnComponent, public CoreEventHandler, ConsoleEventHandler {
    ICore* core = nullptr;
    Scripting scriptingInstance;
    DefaultEventDispatcher<PawnEventHandler> eventDispatcher;

    StringView componentName() const override
    {
        return "Pawn";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    IEventDispatcher<PawnEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    void onLoad(ICore* c) override
    {
        core = c;
        // store core instance and add event handlers
        PawnManager::Get()->core = core;
        PawnManager::Get()->config = &core->getConfig();
        PawnManager::Get()->players = &core->getPlayers();
        PawnPluginManager::Get()->core = core;
        core->getEventDispatcher().addEventHandler(this);

        // Set AMXFILE environment variable to "{current_dir}/scriptfiles"
        std::filesystem::path scriptfilesPath = std::filesystem::absolute("scriptfiles");
        if (!std::filesystem::exists(scriptfilesPath) || !std::filesystem::is_directory(scriptfilesPath)) {
            std::filesystem::create_directory(scriptfilesPath);
        }
        std::string amxFileEnvVar = scriptfilesPath.string();

        amxFileEnvVar.insert(0, "AMXFILE=");
        putenv(amxFileEnvVar.data());
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
        mgr->gangzones = components->queryComponent<IGangZonesComponent>();
        mgr->menus = components->queryComponent<IMenusComponent>();
        mgr->objects = components->queryComponent<IObjectsComponent>();
        mgr->pickups = components->queryComponent<IPickupsComponent>();
        mgr->textdraws = components->queryComponent<ITextDrawsComponent>();
        mgr->textlabels = components->queryComponent<ITextLabelsComponent>();
        mgr->timers = components->queryComponent<ITimersComponent>();
        mgr->vars = components->queryComponent<IVariablesComponent>();
        mgr->vehicles = components->queryComponent<IVehiclesComponent>();

        scriptingInstance.addEvents();

        if (mgr->console) {
            mgr->console->getEventDispatcher().addEventHandler(this);
        }
    }

    void onReady() override
    {
        PawnManager* mgr = PawnManager::Get();
        PawnPluginManager* pluginMgr = PawnPluginManager::Get();

        // read values of plugins, entry_file and side_scripts from config file
        IConfig& config = core->getConfig();
        Span<const StringView> plugins = config.getStrings("legacy_plugins");
        StringView entryFile = config.getString("entry_file");
        Span<const StringView> sideScripts = config.getStrings("side_scripts");

        // load plugins
        for (auto& plugin : plugins) {
            pluginMgr->Load(String(plugin));
        }

        // load scripts
        mgr->Load(eventDispatcher, String(entryFile), true);
        for (auto& script : sideScripts) {
            mgr->Load(eventDispatcher, String(script), false);
        }
    }

    const StaticArray<void*, NUM_AMX_FUNCS>& getAmxFunctions() const override
    {
        return AMX_FUNCTIONS;
    }

    bool onConsoleText(StringView command, StringView parameters) override
    {
        return PawnManager::Get()->OnServerCommand(eventDispatcher, String(command), String(parameters));
    }

    void onTick(Microseconds elapsed, TimePoint now) override
    {
        PawnPluginManager::Get()->ProcessTick();
    }

    ~PawnComponent()
    {
        if (core) {
            core->getEventDispatcher().removeEventHandler(this);
        }
        if (PawnManager::Get()->console) {
            PawnManager::Get()->console->getEventDispatcher().removeEventHandler(this);
        }
    }
} component;

COMPONENT_ENTRY_POINT()
{
    return &component;
}
