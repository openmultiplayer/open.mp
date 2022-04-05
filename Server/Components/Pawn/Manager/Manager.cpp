/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include "Manager.hpp"
#include "../PluginManager/PluginManager.hpp"
#include "../utils.hpp"

#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <iostream>
#include <charconv>

#include <pawn-natives/NativeFunc.hpp>
#include <pawn-natives/NativesMain.hpp>
#include "../Scripting/Player/Events.hpp"

extern "C" {
#if defined _UNICODE
#include <tchar.h>
#elif !defined __T
typedef char TCHAR;
#define __T(string) string
#define _tcscat strcat
#define _tcschr strchr
#define _tcscpy strcpy
#define _tcslen strlen
#endif

#include <amx/amxcons.h>
}

PawnManager::PawnManager()
    : mainName_("")
    , mainScript_()
    , gamemodes_()
    , nextRestart_(TimePoint::min())
    , restartDelay_(12000)
    , scriptPath_("")
    , basePath_("./")
{
}

PawnManager::~PawnManager()
{
	if (mainScript_) {
		mainScript_->Call("OnGameModeExit", DefaultReturnValue_False);
		eventDispatcher.dispatch(&PawnEventHandler::onAmxUnload, mainScript_->GetAMX());
		pluginManager.AmxUnload(mainScript_->GetAMX());
		PawnTimerImpl::Get()->killTimers(mainScript_->GetAMX());
	}
    for (auto& cur : scripts_) {
        auto& script = *cur.second;
        script.Call("OnFilterScriptExit", DefaultReturnValue_False);
        eventDispatcher.dispatch(&PawnEventHandler::onAmxUnload, script.GetAMX());
        pluginManager.AmxUnload(script.GetAMX());
        PawnTimerImpl::Get()->killTimers(script.GetAMX());
    }
}

bool PawnManager::OnServerCommand(const ConsoleCommandSenderData& sender, std::string const& cmd, std::string const& args)
{
    // Legacy commands.
    if (cmd == "loadfs") {
        if (!Load("filterscripts/" + args)) {
            console->sendMessage(sender, "Filterscript '" + args + "' load failed.");
        } else {
            console->sendMessage(sender, "Filterscript '" + args + "' loaded.");
        }
        return true;
    } else if (cmd == "unloadfs") {
        if (!Unload("filterscripts/" + args)) {
            console->sendMessage(sender, "Filterscript '" + args + "' unload failed.");
        } else {
            console->sendMessage(sender, "Filterscript '" + args + "' unloaded.");
        }
        return true;
    } else if (cmd == "reloadfs") {
        if (!Reload("filterscripts/" + args)) {
            console->sendMessage(sender, "Filterscript '" + args + "' reload failed.");
        } else {
            console->sendMessage(sender, "Filterscript '" + args + "' reloaded.");
        }
        return true;
    } else if (cmd == "gmx") {
		if (reloading_)
		{
			return true;
		}
        // How many times should we repeat this mode?
        --gamemodeRepeat_;
        int initial = gamemodeIndex_;
        for ( ; ; )
        {
            if (gamemodeRepeat_ == 0)
            {
                // Advance to the next script in the list.
                ++gamemodeIndex_;
                if (gamemodeIndex_ == gamemodes_.size())
                {
                    gamemodeIndex_ = 0;
                }
                gamemodeRepeat_ = repeats_[gamemodeIndex_];
            }
            if (Changemode("gamemodes/" + gamemodes_[gamemodeIndex_]))
            {
                break;
            }
            else if ((gamemodeIndex_ + 1 == initial) || (gamemodeIndex_ + 1 == gamemodes_.size() && initial == 0))
            {
                // Tried all the GMs in the list, couldn't load any.
                break;
            }
            else
            {
                // Couldn't load this mode, try the next one.
                gamemodeRepeat_ = 0;
            }
        }
        return true;
    } else if (cmd == "changemode") {
		if (reloading_)
		{
			return true;
		}
		if (Changemode("gamemodes/" + args))
        {
            gamemodeRepeat_ = 1;
        }
        return true;
    }
    // New commands.
    else if (cmd == "loadscript") {
        if (!Load(args)) {
            console->sendMessage(sender, "Script '" + args + "' load failed.");
        } else {
            console->sendMessage(sender, "Script '" + args + "' loaded.");
        }
        return true;
    } else if (cmd == "unloadscript") {
        if (!Unload(args)) {
            console->sendMessage(sender, "Script '" + args + "' unload failed.");
        } else {
            console->sendMessage(sender, "Script '" + args + "' unloaded.");
        }
        return true;
    } else if (cmd == "reloadscript") {
        if (!Reload(args)) {
            console->sendMessage(sender, "Script '" + args + "' reload failed.");
        } else {
            console->sendMessage(sender, "Script '" + args + "' reloaded.");
        }
        return true;
    }
    return false;
}

AMX* PawnManager::AMXFromID(int id) const
{
	if (mainScript_ && mainScript_->GetID() == id)
	{
		return mainScript_->GetAMX();
	}
	for (auto& cur : scripts_) {
        if (cur.second->GetID() == id) {
            return cur.second->GetAMX();
        }
    }
    return nullptr;
}

int PawnManager::IDFromAMX(AMX* amx) const
{
	if (mainScript_ && mainScript_->GetAMX() == amx)
	{
		return mainScript_->GetID();
	}
	for (auto& cur : scripts_) {
        if (cur.second->GetAMX() == amx) {
            return cur.second->GetID();
        }
    }
    return 0;
}

void PawnManager::CheckNatives(PawnScript& script)
{
    int
        count;
    script.NumNatives(&count);
    AMX_NATIVE_INFO
		func;
    while (count--) {
        script.GetNativeByIndex(count, &func);
        if (func.func == nullptr) {
            core->logLn(LogLevel::Error, "Function not registered: %s", func.name);
        }
    }
}

bool PawnManager::Changemode(std::string const& name)
{
    // First check that the new script exists.
    FILE* fp;
    std::string ext = utils::endsWith(name, ".amx") ? "" : ".amx";
    std::string canon;
    utils::Canonicalise(basePath_ + scriptPath_ + name + ext, canon);
	// This is exactly the code that pawn itself uses to open a mode, and here we're basically
	// checking that the mode will be loadable when it comes to it.  Using a different system such
	// as `std::filesystem` might introduce incompatibilities between what we think can be loaded
	// and what can actually be loaded.  So while this is "old" C, it is better in this use-case.
    if ((fp = fopen(canon.c_str(), "rb")) == NULL)
    {
        core->printLn("Could not find:\n\n\t %s %s", name.c_str(),
            R"(
                While attempting to load a PAWN gamemode, a file-not-found error was
                encountered.  This could be caused by many things:
                
                * The wrong filename was given.
                * The wrong gamemodes path was given.
                * The server was launched from a different directory, making relative paths relative to the wrong place (and thus wrong).
                * You didn't copy the file to the correct directory or server.
                * The compilation failed, leading to no output file.
                * `-l` or `-a` were used to compile, which output intermediate steps for inspecting, rather than a full script.
                * Anything else, really just check the file is at the path given.
            )");
        return false;
    }
    // Close it for now, it'll be reopened again by `aux_LoadProgram`.
    fclose(fp);
    // Disconnect players.
    // Unload the old main script.
    Unload(mainName_);
    // TODO: Trigger all components to reset.
    PlayerEvents::Get()->IgnoreOneDisconnect();
    // TODO: Inform clients of the restart.
    // Save the name of the next script.
    mainName_ = name;
    // Start the changemode timer.
    nextRestart_ = Time::now() + restartDelay_;
	reloading_ = true;
    return true;
}

void PawnManager::ProcessTick(Microseconds elapsed, TimePoint now)
{
    if (nextRestart_ == TimePoint::min())
    {
        return;
    }
    // Reloading a script.
    if (nextRestart_ < now)
    {
        // Restart is in the past, load the next GM.
        Load(mainName_, true);
        nextRestart_ = TimePoint::min();
    }
}

bool PawnManager::Load(DynamicArray<StringView> const& mainScripts)
{
    if (mainScripts.empty())
    {
        return false;
    }
    gamemodes_.clear();
    gamemodeIndex_ = 0;
    for (auto const & i : mainScripts)
    {
        // Split the mode name and count.
        auto space = i.find_last_of(' ');
        if (space == std::string::npos)
        {
            repeats_.push_back(1);
            gamemodes_.push_back(String(i));
        }
        else
        {
            int count = 0;
            auto conv = std::from_chars(i.data() + space + 1, i.data() + i.size(), count, 10);
            if (conv.ec == std::errc::invalid_argument || conv.ec == std::errc::result_out_of_range || count < 1)
            {
                count = 1;
            }
            repeats_.push_back(count);
            gamemodes_.push_back(String(i.substr(0, space)));
        }
    }
    gamemodeRepeat_ = repeats_[0];
    return Load("gamemodes/" + gamemodes_[0], true);
}

bool PawnManager::Load(std::string const& name, bool isEntryScript)
{
	if (mainName_ == name)
	{
		if (mainScript_)
		{
			return false;
		}
	}
	else
	{
		if (findScript(name) != scripts_.end())
		{
			return false;
		}
	}

    // if the user just supplied a script name, add the extension
    // otherwise, don't, as they may have supplied a full abs/rel path.
    std::string ext = utils::endsWith(name, ".amx") ? "" : ".amx";

    std::string canon;
    utils::Canonicalise(basePath_ + scriptPath_ + name + ext, canon);
    std::unique_ptr<PawnScript> ptr = std::make_unique<PawnScript>(++id_, canon, core);

    if (!ptr.get()->IsLoaded()) {
        //core->logLn(LogLevel::Error, "Unable to load script %s\n\n", name.c_str());
        return false;
    }

    PawnScript& script = *ptr;

	if (isEntryScript)
	{
        mainName_ = name;
		mainScript_ = std::move(ptr);
	}
	else
	{
		Pair<String, std::unique_ptr<PawnScript>> pair = std::make_pair(name, std::move(ptr));
		scripts_.push_back(std::move(pair));
		auto res = findScript(name);
		if (res != scripts_.end()) {
			amxToScript_.emplace(script.GetAMX(), res->second.get());
		}
	}
    script.Register("CallLocalFunction", &utils::pawn_Script_Call);
    script.Register("CallRemoteFunction", &utils::pawn_Script_CallAll);
    script.Register("Script_CallTargeted", &utils::pawn_Script_CallOne);
    script.Register("format", &utils::pawn_format);
    script.Register("printf", &utils::pawn_printf);
    script.Register("Script_GetID", &utils::pawn_Script_GetID);
    script.Register("SetTimer", &utils::pawn_settimer);
    script.Register("SetTimerEx", &utils::pawn_settimerex);
    script.Register("KillTimer", &utils::pawn_killtimer);
    script.Register("SetModeRestartTime", &utils::pawn_SetModeRestartTime);
    script.Register("SetModeRestartTime", &utils::pawn_GetModeRestartTime);

    pawn_natives::AmxLoad(script.GetAMX());
    pluginManager.AmxLoad(script.GetAMX());
    eventDispatcher.dispatch(&PawnEventHandler::onAmxLoad, script.GetAMX());

    CheckNatives(script);

    if (isEntryScript) {
        script.Call("OnGameModeInit", DefaultReturnValue_False);
        CallInSides("OnGameModeInit", DefaultReturnValue_False);

        int err = script.Exec(nullptr, AMX_EXEC_MAIN);
        if (err != AMX_ERR_INDEX && err != AMX_ERR_NONE) {
            // If there's no `main` ignore it for now.
            core->logLn(LogLevel::Error, "%s", aux_StrError(err));
        } else {
            script.cache_.inited = true;
        }
        PlayerEvents::Get()->IgnoreOneConnect();
    } else {
        script.Call("OnFilterScriptInit", DefaultReturnValue_False);

        for (IPlayer* player : players->entries()) {
            script.Call("OnPlayerConnect", DefaultReturnValue_True, player->getID());
        }

        script.cache_.inited = true;
    }

    // TODO: `AMX_EXEC_CONT` support.
    // Assume that all initialisation and header mangling is now complete, and that it is safe to
    // cache public pointers.

    // Call `OnPlayerConnect` (can be after caching).
    if (isEntryScript && reloading_)
    {
        core->reloadAll();
		reloading_ = false;
		setRestartMS(12000);
    }
    for (auto p : players->entries())
    {
        script.Call("OnPlayerConnect", DefaultReturnValue_True, p->getID());
    }
    return true;
}

bool PawnManager::Reload(std::string const& name)
{
    // Entry script reload is not supported.
    if (mainName_ == name)
	{
        return false;
    }

    Unload(name);
    return Load(name, false);
}

bool PawnManager::Unload(std::string const& name)
{
	auto pos = findScript(name);
    bool isEntryScript = mainName_ == name;
	if (isEntryScript)
	{
		if (!mainScript_)
		{
			return false;
		}
	}
	else
	{
		if (pos == scripts_.end())
		{
			return false;
		}
	}
    auto& script = isEntryScript ? *mainScript_ : *pos->second;

    // Call `OnPlayerDisconnect`.
    for (auto const p : players->entries())
    {
        script.Call("OnPlayerDisconnect", DefaultReturnValue_True, p->getID(), 3);
    }
    if (isEntryScript) {
        CallInSides("OnGameModeExit", DefaultReturnValue_False);
        script.Call("OnGameModeExit", DefaultReturnValue_False);
    } else {
        for (IPlayer* player : players->entries()) {
            script.Call("OnPlayerDisconnect", DefaultReturnValue_True, player->getID());
        }
        script.Call("OnFilterScriptExit", DefaultReturnValue_False);
    }

    eventDispatcher.dispatch(&PawnEventHandler::onAmxUnload, script.GetAMX());
    pluginManager.AmxUnload(script.GetAMX());
    PawnTimerImpl::Get()->killTimers(script.GetAMX());
    amxToScript_.erase(script.GetAMX());

    if (isEntryScript)
    {
        core->resetAll();
        mainName_ = "";
		mainScript_.reset();
    }
	else
	{
	    scripts_.erase(pos);
	}

    return true;
}

void PawnManager::SetBasePath(std::string const& path)
{
    if (path.length() == 0) {
        basePath_ = "/";
    } else if (path.back() == '/') {
        basePath_ = path;
    } else {
        basePath_ = path + '/';
    }
}

void PawnManager::SetScriptPath(std::string const& path)
{
    if (path.length() == 0) {
        scriptPath_ = "/";
    } else if (path.back() == '/') {
        scriptPath_ = path;
    } else {
        scriptPath_ = path + '/';
    }
}
