/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

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

extern "C"
{
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
	, nextSleep_(TimePoint::min())
	, sleepData_()
	, scriptPath_("")
	, basePath_("./")
{
}

PawnManager::~PawnManager()
{
	if (mainScript_)
	{
		mainScript_->Call("OnGameModeExit", DefaultReturnValue_False);
		CallInSides("OnGameModeExit", DefaultReturnValue_False);
		PawnTimerImpl::Get()->killTimers(mainScript_->GetAMX());
		pluginManager.AmxUnload(mainScript_->GetAMX());
		eventDispatcher.dispatch(&PawnEventHandler::onAmxUnload, *mainScript_);
	}
	for (IPawnScript* cur : scripts_)
	{
		IPawnScript& script = *cur;
		script.Call("OnFilterScriptExit", DefaultReturnValue_False);
		PawnTimerImpl::Get()->killTimers(script.GetAMX());
		pluginManager.AmxUnload(script.GetAMX());
		eventDispatcher.dispatch(&PawnEventHandler::onAmxUnload, script);
	}
}

void PawnManager::OnServerCommandList(FlatHashSet<StringView>& commands)
{
	commands.emplace("loadfs");
	commands.emplace("unloadfs");
	commands.emplace("reloadfs");
	commands.emplace("gmx");
	commands.emplace("changemode");
	commands.emplace("loadscript");
	commands.emplace("unloadscript");
	commands.emplace("reloadscript");
}

bool PawnManager::OnServerCommand(const ConsoleCommandSenderData& sender, std::string const& cmd, std::string const& args)
{
	// Legacy commands.
	if (cmd == "loadfs")
	{
		if (!Load("filterscripts/" + args))
		{
			console->sendMessage(sender, "Filterscript '" + args + "' load failed.");
		}
		else
		{
			console->sendMessage(sender, "Filterscript '" + args + "' loaded.");
		}
		return true;
	}
	else if (cmd == "unloadfs")
	{
		if (!Unload("filterscripts/" + args))
		{
			console->sendMessage(sender, "Filterscript '" + args + "' unload failed.");
		}
		else
		{
			console->sendMessage(sender, "Filterscript '" + args + "' unloaded.");
		}
		return true;
	}
	else if (cmd == "reloadfs")
	{
		if (!Reload("filterscripts/" + args))
		{
			console->sendMessage(sender, "Filterscript '" + args + "' reload failed.");
		}
		else
		{
			console->sendMessage(sender, "Filterscript '" + args + "' reloaded.");
		}
		return true;
	}
	else if (cmd == "gmx")
	{
		EndMainScript();
		return true;
	}
	else if (cmd == "changemode")
	{
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
	else if (cmd == "loadscript")
	{
		if (!Load(args))
		{
			console->sendMessage(sender, "Script '" + args + "' load failed.");
		}
		else
		{
			console->sendMessage(sender, "Script '" + args + "' loaded.");
		}
		return true;
	}
	else if (cmd == "unloadscript")
	{
		if (!Unload(args))
		{
			console->sendMessage(sender, "Script '" + args + "' unload failed.");
		}
		else
		{
			console->sendMessage(sender, "Script '" + args + "' unloaded.");
		}
		return true;
	}
	else if (cmd == "reloadscript")
	{
		if (!Reload(args))
		{
			console->sendMessage(sender, "Script '" + args + "' reload failed.");
		}
		else
		{
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
	for (IPawnScript* cur : scripts_)
	{
		if (cur->GetID() == id)
		{
			return cur->GetAMX();
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
	for (IPawnScript* cur : scripts_)
	{
		if (cur->GetAMX() == amx)
		{
			return cur->GetID();
		}
	}
	return 0;
}

void PawnManager::CheckNatives(PawnScript& script)
{
	int count;
	script.NumNatives(&count);
	AMX_NATIVE_INFO func;

	bool any_deprecated = false;

	while (count--)
	{
		script.GetNativeByIndex(count, &func);

		if (func.name == nullptr)
		{
			// This won't currently warn for using deprecated natives in x64, because the name gets
			// clobebered by the longer address when it is registered.  But warning for unregistered
			// natives will at least still work (because they, by definition, aren't clobbered).
			continue;
		}

		auto itr = DeprecatedNatives.find(func.name);
		bool is_deprecated = itr != DeprecatedNatives.end();

		if (func.func == nullptr)
		{
			if (!is_deprecated)
			{
				core->logLn(LogLevel::Error, "Function not registered: %s", func.name);
			}
			else
			{
				core->logLn(LogLevel::Error, "Function %s was removed and replaced by %s.", func.name, itr->second.c_str());
			}
		}
		else
		{
			if (is_deprecated)
			{
				core->logLn(LogLevel::Warning, "Deprecated function %s used. This function was replaced by %s.", func.name, itr->second.c_str());
				any_deprecated = true;
			}
		}
	}

	if (any_deprecated)
	{
		core->logLn(LogLevel::Warning, "Deprecated functions will be removed in the next open.mp release.");
	}
}

bool PawnManager::Changemode(std::string const& name)
{
	std::string normal_script_name;
	utils::NormaliseScriptName(name, normal_script_name);

	// First check that the new script exists.
	FILE* fp;
	std::string canon_path;
	utils::Canonicalise(basePath_ + scriptPath_ + normal_script_name, canon_path);
	// This is exactly the code that pawn itself uses to open a mode, and here we're basically
	// checking that the mode will be loadable when it comes to it.  Using a different system such
	// as `std::filesystem` might introduce incompatibilities between what we think can be loaded
	// and what can actually be loaded.  So while this is "old" C, it is better in this use-case.
	if ((fp = fopen(canon_path.c_str(), "rb")) == NULL)
	{
		core->printLn("Could not find:\n\n\t %s %s", normal_script_name.c_str(),
			R"(
While attempting to load a PAWN gamemode, a file-not-found error was
encountered.  This could be caused by many things:

 * The wrong filename was given.
 * The wrong gamemodes path was given.
 * The server was launched from a different directory, making relative paths
   relative to the wrong place (and thus wrong).
 * You didn't copy the file to the correct directory or server.
 * The compilation failed, leading to no output file.
 * `-l` or `-a` were used to compile, which output intermediate steps for
   inspecting, rather than a full script.
 * Anything else, really just check the file is at the path given.
            )");
		return false;
	}
	// Close it for now, it'll be reopened again by `aux_LoadProgram`.
	fclose(fp);

	// Unload the main script in the next server tick.
	unloadNextTick_ = true;
	nextScriptName_ = normal_script_name;

	return true;
}

void PawnManager::EndMainScript()
{
	if (reloading_ || gamemodes_.empty())
	{
		return;
	}
	// How many times should we repeat this mode?
	--gamemodeRepeat_;
	int initial = gamemodeIndex_;
	for (;;)
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
}

void PawnManager::ProcessTick(Microseconds elapsed, TimePoint now)
{
	if (unloadNextTick_)
	{
		// Disconnect players.
		// Unload the old main script.
		Unload(mainName_);
		// Save the name of the next script.
		mainName_ = nextScriptName_;
		// Start the changemode timer.
		nextRestart_ = Time::now() + restartDelay_;
		reloading_ = true;

		// Reset delayed unloading.
		unloadNextTick_ = false;
		nextScriptName_ = "";
	}
	if (nextRestart_ != TimePoint::min() && nextRestart_ <= now)
	{
		// Reloading a script.  Restart is in the past, load the next GM.
		Load(mainName_, true);
		nextRestart_ = TimePoint::min();
	}
	if (mainScript_ && nextSleep_ != TimePoint::min() && nextSleep_ <= now)
	{
		// AMX_EXEC_CONT
		// Restore the saved `sleep` data.  This is what sleep is meant to do itself, but the code
		// wasn't written to account for other callbacks being called in the interim, and they thus
		// clobber this data.
		AMX* amx = mainScript_->GetAMX();
		amx->cip = sleepData_.cip;
		amx->frm = sleepData_.frm;
		amx->hea = sleepData_.hea;
		amx->stk = sleepData_.stk;
		amx->pri = sleepData_.pri;
		amx->alt = sleepData_.alt;
		amx->reset_stk = sleepData_.reset_stk;
		amx->reset_hea = sleepData_.reset_hea;
		nextSleep_ = TimePoint::min();
		cell retval;
		int err = mainScript_->Exec(&retval, AMX_EXEC_CONT);
		if (err == AMX_ERR_SLEEP)
		{
			nextSleep_ = Time::now() + Milliseconds(retval);
			sleepData_ = {
				amx->cip,
				amx->frm,
				amx->hea,
				amx->stk,
				amx->pri,
				amx->alt,
				amx->reset_stk,
				amx->reset_hea,
			};
		}
		else if (err != AMX_ERR_NONE)
		{
			// If there's no `main` ignore it for now.
			core->logLn(LogLevel::Error, "%d %s", err, aux_StrError(err));
		}
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
	for (auto const& i : mainScripts)
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

void PawnManager::openAMX(PawnScript& script, bool isEntryScript)
{
	script.Register("CallLocalFunction", &utils::pawn_Script_Call);
	script.Register("Script_CallByIndex", &utils::pawn_Script_CallByIndex);
	script.Register("CallRemoteFunction", &utils::pawn_Script_CallAll);
	script.Register("Script_CallTargeted", &utils::pawn_Script_CallOne);
	script.Register("Script_CallTargetedByIndex", &utils::pawn_Script_CallOneByIndex);
	script.Register("format", &utils::pawn_format);
	script.Register("printf", &utils::pawn_printf);
	script.Register("Script_GetID", &utils::pawn_Script_GetID);
	script.Register("SetTimer", &utils::pawn_settimer);
	script.Register("SetTimerEx", &utils::pawn_settimerex);
	script.Register("KillTimer", &utils::pawn_killtimer);
	script.Register("IsValidTimer", &utils::pawn_IsValidTimer);
	script.Register("IsRepeatingTimer", &utils::pawn_IsRepeatingTimer);
	script.Register("GetTimerRemaining", &utils::pawn_GetTimerRemaining);
	script.Register("GetTimerInterval", &utils::pawn_GetTimerInterval);
	script.Register("SetModeRestartTime", &utils::pawn_SetModeRestartTime);
	script.Register("GetModeRestartTime", &utils::pawn_GetModeRestartTime);

	eventDispatcher.dispatch(&PawnEventHandler::onAmxLoad, script);
	pawn_natives::AmxLoad(script.GetAMX());
	pluginManager.AmxLoad(script.GetAMX());

	cell amxAddr;
	cell* realAddr;
	if (script.FindPubVar("__OPEN_MP_VERSION", &amxAddr) == AMX_ERR_NONE && script.GetAddr(amxAddr, &realAddr) == AMX_ERR_NONE)
	{
		// Compact the version number as tightly as we can.  This allows for up to 16 major
		// versions, 127 minor and patch versions, and 16383 prerel versions (so all three
		// minor segments can encode the full range of 2/4 digit decimal values).
		auto version = core->getVersion();
		*realAddr = ((version.major & 0x0F) << 28) | ((version.minor & 0x7F) << 21) | ((version.patch & 0x7F) << 14) | (version.prerel & 0x3FFF);
	}

	CheckNatives(script);

	if (isEntryScript)
	{
		script.Call("OnGameModeInit", DefaultReturnValue_False);
		CallInSides("OnGameModeInit", DefaultReturnValue_False);

		// We're calling reloadAll after mode initialisation because we want to send
		// updated settings to clients in PlayerInit RPC (such as available classes count)
		if (reloading_)
		{
			core->reloadAll();
			reloading_ = false;
			setRestartMS(12000);
		}

		nextSleep_ = TimePoint::min();
		cell retval;
		int err = script.Exec(&retval, AMX_EXEC_MAIN);
		if (err == AMX_ERR_NONE)
		{
			script.cache_.inited = true;
		}
		else if (err == AMX_ERR_SLEEP)
		{
			script.cache_.inited = true;
			nextSleep_ = Time::now() + Milliseconds(retval);
			// Save the `sleep` state so it doesn't get clobbered by other callbacks.
			AMX* amx = script.GetAMX();
			sleepData_ = {
				amx->cip,
				amx->frm,
				amx->hea,
				amx->stk,
				amx->pri,
				amx->alt,
				amx->reset_stk,
				amx->reset_hea,
			};
		}
		else if (err != AMX_ERR_NOTFOUND)
		{
			// If there's no `main` ignore it for now.
			core->logLn(LogLevel::Error, "%s", aux_StrError(err));
		}
		// TODO: `AMX_EXEC_CONT` support.
	}
	else
	{
		script.Call("OnFilterScriptInit", DefaultReturnValue_False);
		script.cache_.inited = true;
	}

	// Assume that all initialisation and header mangling is now complete, and that it is safe to
	// cache public pointers.

	// Call `OnPlayerConnect` (can be after caching).
	for (auto p : players->entries())
	{
		script.Call("OnPlayerConnect", DefaultReturnValue_True, p->getID());
	}
}

bool PawnManager::Load(std::string const& name, bool isEntryScript)
{
	std::string normal_script_name;
	utils::NormaliseScriptName(name, normal_script_name);

	if (mainName_ == normal_script_name)
	{
		if (mainScript_)
		{
			return false;
		}
	}
	else
	{
		if (findScript(normal_script_name) != scripts_.end())
		{
			return false;
		}
	}

	std::string canon_path;
	utils::Canonicalise(basePath_ + scriptPath_ + normal_script_name, canon_path);
	PawnScript* ptr = new PawnScript(++id_, canon_path, core);

	if (!ptr || !ptr->IsLoaded())
	{
		// core->logLn(LogLevel::Error, "Unable to load script %s\n\n", name.c_str());
		return false;
	}
	ptr->name_ = normal_script_name;

	PawnScript& script = *ptr;

	if (isEntryScript)
	{
		mainName_ = normal_script_name;
		delete mainScript_;
		mainScript_ = ptr;
		amxToScript_.emplace(mainScript_->GetAMX(), mainScript_);

		unloadNextTick_ = false;
		nextScriptName_ = "";
	}
	else
	{
		scripts_.push_back(ptr);
		amxToScript_.emplace(script.GetAMX(), ptr);
	}
	openAMX(script, isEntryScript);
	return true;
}

void PawnManager::closeAMX(PawnScript& script, bool isEntryScript)
{
	AMX* amx = script.GetAMX();
	int idx;
	bool once = true;
	// Reason 4, to match fixes.inc.  Why was it not 3?  I don't know.
	if (amx_FindPublic(amx, "OnPlayerDisconnect", &idx) == AMX_ERR_NONE)
	{
		for (auto const p : players->entries())
		{
			cell ret = 1;
			int err = script.CallChecked(idx, ret, p->getID(), PeerDisconnectReason_ModeEnd);
			switch (err)
			{
			case AMX_ERR_NONE:
				break;
			case AMX_ERR_BOUNDS:
				// Test the `OP_BOUNDS` parameter and the current index.
				if (once && (*(cell*)((uintptr_t)amx->base + (((AMX_HEADER*)amx->base)->cod + amx->cip - sizeof(cell)))) == 2) // && amx->pri == 4)
				{
					core->printLn(R"(
Array out-of-bounds encountered during `OnPlayerDisconnect` with reason `4`
(script exit).  This may be due to old code assuming the highest possible reason
is `2`.
)");
					// Only show the error once, don't spam it.
					once = false;
					break;
				}
				// Fallthrough
			default:
				core->logLn(LogLevel::Error, "%s", aux_StrError(err));
				break;
			}
		}
	}
	if (isEntryScript)
	{
		script.Call("OnGameModeExit", DefaultReturnValue_False);
		CallInSides("OnGameModeExit", DefaultReturnValue_False);
	}
	else
	{
		script.Call("OnFilterScriptExit", DefaultReturnValue_False);
	}

	PawnTimerImpl::Get()->killTimers(script.GetAMX());
	pluginManager.AmxUnload(script.GetAMX());
	eventDispatcher.dispatch(&PawnEventHandler::onAmxUnload, script);
	amxToScript_.erase(script.GetAMX());
}

bool PawnManager::Reload(std::string const& name)
{
	std::string normal_script_name;
	utils::NormaliseScriptName(name, normal_script_name);

	// Entry script reload is not supported.
	if (mainName_ == normal_script_name)
	{
		return false;
	}
	auto pos = findScript(normal_script_name);
	if (pos == scripts_.end())
	{
		return false;
	}
	PawnScript& script = *reinterpret_cast<PawnScript*>(*pos);
	closeAMX(script, false);
	std::string canon_path;
	utils::Canonicalise(basePath_ + scriptPath_ + normal_script_name, canon_path);
	script.tryLoad(canon_path);
	openAMX(script, false);
	amxToScript_.emplace(script.GetAMX(), &script);
	return true;
}

bool PawnManager::Unload(std::string const& name)
{
	std::string normal_script_name;
	utils::NormaliseScriptName(name, normal_script_name);

	auto pos = findScript(normal_script_name);
	bool isEntryScript = mainName_ == normal_script_name;
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
	PawnScript& script = isEntryScript ? *mainScript_ : *reinterpret_cast<PawnScript*>(*pos);
	closeAMX(script, isEntryScript);

	if (isEntryScript)
	{
		core->resetAll();
		mainName_ = "";
		delete mainScript_;
		mainScript_ = nullptr;
	}
	else
	{
		delete reinterpret_cast<PawnScript*>(*pos);
		scripts_.erase(pos);
	}

	return true;
}

void PawnManager::SetBasePath(std::string const& path)
{
	if (path.length() == 0)
	{
		basePath_ = "/";
	}
	else if (path.back() == '/')
	{
		basePath_ = path;
	}
	else
	{
		basePath_ = path + '/';
	}
}

void PawnManager::SetScriptPath(std::string const& path)
{
	if (path.length() == 0)
	{
		scriptPath_ = "/";
	}
	else if (path.back() == '/')
	{
		scriptPath_ = path;
	}
	else
	{
		scriptPath_ = path + '/';
	}
}
