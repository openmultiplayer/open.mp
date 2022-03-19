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

#include <pawn-natives/NativeFunc.hpp>
#include <pawn-natives/NativesMain.hpp>

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
    : scriptPath_("")
    , basePath_("./")
    , gamemodes_()
{
}

PawnManager::~PawnManager()
{
    FlatHashMap<String, std::unique_ptr<PawnScript>>::const_iterator const& entryScriptIter = scripts_.find(entryScript);
    for (auto& cur : scripts_) {
        const bool isEntryScript = cur.second == entryScriptIter->second;
        auto& script = *cur.second;
        if (isEntryScript) {
            script.Call("OnGameModeExit", DefaultReturnValue_False);
        } else {
            script.Call("OnFilterScriptExit", DefaultReturnValue_False);
        }
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
		// Advance to the next script in the list.
        ++gamemodeIndex_;
		if (gamemodeIndex_ == gamemodes_.size())
		{
            gamemodeIndex_ = 0;
		}
        Changemode("gamemodes/" + gamemodes_.at(gamemodeIndex_));
        return true;
    } else if (cmd == "changemode") {
        Changemode("gamemodes/" + args);
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
    for (auto& cur : scripts_) {
        if (cur.second->GetID() == id) {
            return cur.second->GetAMX();
        }
    }
    return nullptr;
}

int PawnManager::IDFromAMX(AMX* amx) const
{
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
	if ((fp = fopen(name.c_str(), "rb")) == NULL)
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
	// Unload the old main script.
    Unload(entryScript);
    entryScript = name;
	// Start the changemode timer.
    return true;
}

bool PawnManager::Load(std::string const& name, bool primary)
{
    if (scripts_.count(name)) {
        return false;
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

    auto res = scripts_.emplace(name, std::move(ptr));
    if (res.second) {
        amxToScript_.emplace(script.GetAMX(), res.first->second.get());
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

    pawn_natives::AmxLoad(script.GetAMX());
    pluginManager.AmxLoad(script.GetAMX());
    eventDispatcher.dispatch(&PawnEventHandler::onAmxLoad, script.GetAMX());

    CheckNatives(script);

    if (primary) {
        entryScript = name;
        script.Call("OnGameModeInit", DefaultReturnValue_False);
        CallInSides("OnGameModeInit", DefaultReturnValue_False);

        int err = script.Exec(nullptr, AMX_EXEC_MAIN);
        if (err != AMX_ERR_INDEX && err != AMX_ERR_NONE) {
            // If there's no `main` ignore it for now.
            core->logLn(LogLevel::Error, "%s", aux_StrError(err));
        } else {
            script.cache_.inited = true;
        }
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

    return true;
}

bool PawnManager::Reload(std::string const& name)
{
    // Entry script reload is not supported.
    if (entryScript == name) {
        return false;
    }

    Unload(name);
    return Load(name, false);
}

bool PawnManager::Unload(std::string const& name)
{
    auto pos = scripts_.find(name);
    bool isEntryScript = entryScript == name;
    if (pos == scripts_.end()) {
        return false;
    }
    auto& script = *pos->second;

    if (isEntryScript) {
        CallInSides("OnGameModeExit", DefaultReturnValue_False);
        script.Call("OnGameModeExit", DefaultReturnValue_False);
        entryScript = "";
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
    scripts_.erase(pos);

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
