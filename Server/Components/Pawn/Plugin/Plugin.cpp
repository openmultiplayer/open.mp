/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "Plugin.h"
#include "../Manager/Manager.hpp"

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

int PluginCallGM(char* name);
int PluginCallFS(char* name);
void PluginLogprintf(const char* fmt, ...);

static const void
	*AMX_FUNCTIONS[]
	= {
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
	  },
	*PLUGIN_FUNCTIONS[] = {
		reinterpret_cast<void*>(&PluginLogprintf),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		reinterpret_cast<void*>(&AMX_FUNCTIONS),
		reinterpret_cast<void*>(&PluginCallFS),
		reinterpret_cast<void*>(&PluginCallGM),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr, // netgame
		nullptr, // rakserver
		nullptr, // loadfs
		nullptr, // unloadfs
		nullptr, // console
		nullptr,
	};

template <typename T>
inline static T FindSym(PLUGIN_MODULE dll, char const* const sym)
{
#ifdef WIN32
	return reinterpret_cast<T>(::GetProcAddress(dll, sym));
#else
	return reinterpret_cast<T>(dlsym(dll, sym));
#endif
}

PawnPlugin::PawnPlugin(std::string const& path, ICore* core)
	: serverCore(core)
{
#ifdef WIN32
	// Windows doesn't need the extension.
	pluginHandle_ = LoadLibraryA(path.c_str());
#else
	pluginHandle_ = dlopen(path.c_str(), RTLD_LAZY);
#endif // WIN32

	if (pluginHandle_ == nullptr)
	{
		std::string failMsg_;
#ifdef WIN32
		{
			LPSTR errorText;
			if (FormatMessageA(
					FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPSTR)&errorText,
					0,
					NULL))
			{
				failMsg_ = errorText;
				failMsg_.erase(failMsg_.length() - 2, 2); // remove \r\n
				LocalFree((HLOCAL)errorText);
			}
		}
#else
		failMsg_ = dlerror();
#endif
		if (failMsg_.empty())
		{
			failMsg_ = "Unknown error";
		}
		serverCore->printLn("Could not load plugin:\n%s", failMsg_.c_str());
		return;
	}

	supports_f Supports_ = FindSym<supports_f>(pluginHandle_, "Supports");
	load_f Load_ = FindSym<load_f>(pluginHandle_, "Load");

	if (!Load_ || !Supports_)
	{
		if (FindSym<void*>(pluginHandle_, "ComponentEntryPoint"))
		{
			serverCore->printLn("This file is an open.mp component. Please move it to components/ folder.");
		}
		else
		{
			serverCore->printLn("This file is not a SA-MP plugin.");
		}
		return;
	}

	// First, get the plugin's features.
	unsigned int flags_ = 0;
	flags_ = Supports_();

	if ((flags_ & 0x0000FFFF) == 0x00000200)
	{
		// Only try get the tick function if it is declared as in use.
		if (flags_ & 0x00020000)
		{
			ProcessTick_ = FindSym<process_tick_f>(pluginHandle_, "ProcessTick");
		}
	}

	Unload_ = FindSym<unload_f>(pluginHandle_, "Unload");

	// Zero value means it failed to load
	if (!Load_(PLUGIN_FUNCTIONS))
	{
		serverCore->printLn("Plugin failed to initialize.");
		return;
	}

	if (flags_ & 0x00010000)
	{
		// dont care about too much if they're missing, these get nullptr-checked anyways
		AmxLoad_ = FindSym<amx_load_f>(pluginHandle_, "AmxLoad");
		AmxUnload_ = FindSym<amx_unload_f>(pluginHandle_, "AmxUnload");
	}

	loaded_ = true;
}

PawnPlugin::~PawnPlugin()
{
	if (loaded_)
	{
#ifdef WIN32
		::FreeLibrary(pluginHandle_);
#else
		dlclose(pluginHandle_);
#endif
	}
}

void PawnPlugin::AmxLoad(AMX* amx)
{
	if (AmxLoad_)
	{
		AmxLoad_(amx);
	}
}

void PawnPlugin::AmxUnload(AMX* amx)
{
	if (AmxUnload_)
	{
		AmxUnload_(amx);
	}
}

void PawnPlugin::ProcessTick()
{
	if (ProcessTick_)
	{
		ProcessTick_();
	}
}

void PawnPlugin::Unload()
{
	if (Unload_)
	{
		Unload_();
	}
}

int PluginCallGM(char* name)
{
	auto manager = PawnManager::Get();
	if (manager->mainScript_)
	{
		return manager->mainScript_->Call(name, DefaultReturnValue_False);
	}
	return 0;
}

int PluginCallFS(char* name)
{
	cell
		ret
		= 0;
	auto manager = PawnManager::Get();
	for (auto cur : manager->scripts_)
	{
		ret = cur->Call(name, DefaultReturnValue_False);
		if (!ret)
			return ret;
	}
	return ret;
}

void PluginLogprintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	PawnManager::Get()->core->vprintLn(fmt, args);
	va_end(args);
}
