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

extern "C"
{
#if defined _UNICODE
#include <tchar.h>
#elif !defined __T
	typedef char TCHAR;
#define __T(string)    string
#define _tcscat strcat
#define _tcschr strchr
#define _tcscpy strcpy
#define _tcslen strlen
#endif

#include <amx/amxcons.h>
}


PawnManager::PawnManager()
	:
	scriptPath_(""),
	basePath_("./")
{}

PawnManager::~PawnManager()
{
	for (auto & cur : scripts_)
	{
		cur.second->Call("OnScriptExit", cur.second->GetID());
		PawnPluginManager::Get()->AmxUnload(cur.second->GetAMX());
	}
}

void PawnManager::OnScriptInit(const std::string & script)
{

}

void PawnManager::OnScriptExit(const std::string & script)
{

}

bool PawnManager::OnServerCommand(std::string const & cmd, std::string const & args)
{
	// Legacy commands.
	if (cmd == "loadfs")
	{
		Load(args);
	}
	else if (cmd == "reloadfs")
	{
		Reload(args);
	}
	else if (cmd == "unloadfs")
	{
		Unload(args);
	}
	else if (cmd == "changemode")
	{
		Unload(entryScript);
		Load(args, true);
	}
	return false;
}

AMX * PawnManager::AMXFromID(int id) const
{
	for (auto & cur : scripts_)
	{
		if (cur.second->GetID() == id)
		{
			return cur.second->GetAMX();
		}
	}
	return nullptr;
}

int PawnManager::IDFromAMX(AMX * amx) const
{
	for (auto & cur : scripts_)
	{
		if (cur.second->GetAMX() == amx)
		{
			return cur.second->GetID();
		}
	}
	return 0;
}

void PawnManager::Spawn(std::string const & name)
{
	// if the user just supplied a script name, add the extension
	// otherwise, don't, as they may have supplied a full abs/rel path.
	std::string ext = utils::endsWith(name, ".amx") ? "" : ".amx";

	std::string canon;
	utils::Canonicalise(basePath_ + scriptPath_ + name + ext, canon);
	std::unique_ptr<PawnScript> ptr = std::make_unique<PawnScript>(++id_, canon, core);

	if (!ptr.get()->IsLoaded())
	{
		PawnManager::Get()->core->printLn("Unable to load script %s\n\n", name.c_str());
		return;
	}

	PawnScript & script = *ptr;

	scripts_.emplace(name, std::move(ptr));
	script.Register("Script_Call", &utils::pawn_Script_Call);
	script.Register("Script_CallAll", &utils::pawn_Script_CallAll);
	script.Register("Script_CallTargeted", &utils::pawn_Script_CallOne);
	script.Register("format", &utils::pawn_format);
	script.Register("Script_GetID", &utils::pawn_Script_GetID);

	pawn_natives::AmxLoad(script.GetAMX());

	PawnPluginManager::Get()->AmxLoad(script.GetAMX());

	OnScriptInit(name);

	CheckNatives(script);
	script.Call("OnScriptInit");

	int err = script.Exec(nullptr, AMX_EXEC_MAIN);
	if (err != AMX_ERR_INDEX && err != AMX_ERR_NONE)
	{
		// If there's no `main` ignore it for now.
		PawnManager::Get()->core->printLn(aux_StrError(err));
	}
	// TODO: `AMX_EXEC_CONT` support.
	// Assume that all initialisation and header mangling is now complete, and that it is safe to
	// cache public pointers.
}

void PawnManager::CheckNatives(PawnScript & script)
{
	int
		count;
	script.NumNatives(&count);
	AMX_NATIVE_INFO
		func;
	while (count--)
	{
		script.GetNativeByIndex(count, &func);
		if (func.func == nullptr)
		{
			std::cerr << "Function not registered: " << func.name << std::endl;
		}
	}
}

void PawnManager::Load(std::string const & name, bool primary)
{
	if (scripts_.count(name))
	{
		return;
	}
	Spawn(name);
	if (primary)
	{
		entryScript = name;
	}
}

void PawnManager::Reload(std::string const & name)
{
	auto pos = scripts_.find(name);
	if (pos != scripts_.end()) {
		auto & script = *pos->second;

		script.Call("OnScriptExit");
		OnScriptExit(name);
		PawnPluginManager::Get()->AmxUnload(script.GetAMX());
		scripts_.erase(pos);
	}
	Spawn(name);
}

void PawnManager::Unload(std::string const & name)
{
	auto
		pos = scripts_.find(name);
	if (pos == scripts_.end())
	{
		return;
	}
	auto & script = *pos->second;
	script.Call("OnScriptExit");
	OnScriptExit(name);
	PawnPluginManager::Get()->AmxUnload(script.GetAMX());
	scripts_.erase(pos);
}

void PawnManager::SetBasePath(std::string const & path)
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

void PawnManager::SetScriptPath(std::string const & path)
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
