/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include <Impl/events_impl.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Console/console.hpp>
#include <Server/Components/Databases/databases.hpp>
#include <Server/Components/Dialogs/dialogs.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <Server/Components/GangZones/gangzones.hpp>
#include <Server/Components/Menus/menus.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <Server/Components/Recordings/recordings.hpp>
#include <Server/Components/TextDraws/textdraws.hpp>
#include <Server/Components/TextLabels/textlabels.hpp>
#include <Server/Components/Timers/timers.hpp>
#include <Server/Components/Variables/variables.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/CustomModels/custommodels.hpp>
#include <Server/Components/NPCs/npcs.hpp>
#include <Impl/Utils/singleton.hpp>
#include <sdk.hpp>

#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <string>

#include "../PluginManager/PluginManager.hpp"
#include "../Script/Script.hpp"

using namespace Impl;

class PawnManager : public Singleton<PawnManager>, public PawnLookup
{
public:
	DynamicArray<IPawnScript*> scripts_;
	std::string mainName_ = "";
	PawnScript* mainScript_;
	FlatHashMap<AMX*, PawnScript*> amxToScript_;
	DefaultEventDispatcher<PawnEventHandler> eventDispatcher;
	PawnPluginManager pluginManager;

private:
	int gamemodeIndex_ = 0;
	int gamemodeRepeat_ = 1;
	DynamicArray<String> gamemodes_;
	DynamicArray<int> repeats_;
	TimePoint nextRestart_;
	Milliseconds restartDelay_;
	bool reloading_ = false;
	TimePoint nextSleep_;
	bool unloadNextTick_ = false;
	String nextScriptName_ = "";

	// To preserve main script `sleep` information between callbacks.
	struct
	{
		cell cip;
		cell frm;
		cell hea;
		cell stk;
		cell pri;
		cell alt;
		cell reset_stk;
		cell reset_hea;
	} sleepData_;

	DynamicArray<IPawnScript*>::const_iterator const findScript(String const& name) const
	{
		return std::find_if(scripts_.begin(), scripts_.end(), [name](IPawnScript* const it)
			{
				return reinterpret_cast<PawnScript*>(it)->name_ == name;
			});
	}

	void openAMX(PawnScript& script, bool isEntryScript, bool restarting = false);
	void closeAMX(PawnScript& script, bool isEntryScript);

public:
	PawnManager();
	~PawnManager();

	void printPawnLog(const std::string& type, const std::string& message)
	{
		core->printLn("[PAWN-LOG] %s: %s", type.c_str(), message.c_str());
	}

	void SetBasePath(std::string const& path);
	void SetScriptPath(std::string const& path);

	bool Load(std::string const& name, bool primary = false, bool restarting = false);
	bool Load(DynamicArray<StringView> const& mainScripts);
	bool Reload(std::string const& name);
	bool Unload(std::string const& name);
	bool Changemode(std::string const& name);
	void EndMainScript();

	void ProcessTick(Microseconds elapsed, TimePoint now);
	inline int getRestartMS() const
	{
		return (int)restartDelay_.count();
	}

	inline void setRestartMS(int ms)
	{
		if (nextRestart_ == TimePoint::min())
		{
			restartDelay_ = Milliseconds(ms);
		}
		else
		{
			Milliseconds delay = Milliseconds(ms);
			nextRestart_ = nextRestart_ - restartDelay_ + delay;
			restartDelay_ = delay;
		}
	}

	template <typename... T>
	cell CallAllInSidesFirst(char const* name, DefaultReturnValue defaultRetValue, T... args)
	{
		cell ret = static_cast<cell>(defaultRetValue);

		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, defaultRetValue, args...);
		}
		if (mainScript_)
		{
			ret = mainScript_->Call(name, defaultRetValue, args...);
		}

		return ret;
	}

	template <typename... T>
	cell CallAllInEntryFirst(char const* name, DefaultReturnValue defaultRetValue, T... args)
	{
		cell ret = static_cast<cell>(defaultRetValue);

		if (mainScript_)
		{
			ret = mainScript_->Call(name, defaultRetValue, args...);
		}
		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, defaultRetValue, args...);
		}

		return ret;
	}

	template <typename... T>
	cell CallInSidesWhile0(char const* name, T... args)
	{
		cell
			ret
			= 0;

		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_False, args...);
			if (ret)
			{
				break;
			}
		}

		return ret;
	}

	template <typename... T>
	cell CallInSidesWhile1(char const* name, T... args)
	{
		cell
			ret
			= 1;

		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_True, args...);
			if (!ret)
			{
				break;
			}
		}

		return ret;
	}

	template <typename... T>
	cell CallInSides(char const* name, DefaultReturnValue defaultRetValue, T... args)
	{
		cell ret = static_cast<cell>(defaultRetValue);

		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, defaultRetValue, args...);
		}

		return ret;
	}

	template <typename... T>
	cell CallInEntry(char const* name, DefaultReturnValue defaultRetValue, T... args)
	{
		cell ret = static_cast<cell>(defaultRetValue);

		if (mainScript_)
		{
			ret = mainScript_->Call(name, defaultRetValue, args...);
		}

		return ret;
	}

	template <typename... T>
	cell CallAll(char const* name, T... args)
	{
		cell
			ret
			= 0;
		if (mainScript_)
		{
			ret = mainScript_->Call(name, DefaultReturnValue_False, args...);
		}
		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_False, args...);
		}
		return ret;
	}

	template <typename... T>
	cell CallAll(std::string const& name, T... args)
	{
		cell
			ret
			= 0;
		if (mainScript_)
		{
			ret = mainScript_->Call(name, DefaultReturnValue_False, args...);
		}
		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_False, args...);
		}
		return ret;
	}

	template <typename... T>
	cell CallWhile0(char const* name, T... args)
	{
		cell
			ret
			= 0;
		if (mainScript_)
		{
			ret = mainScript_->Call(name, DefaultReturnValue_False, args...);
			if (ret)
				return ret;
		}
		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_False, args...);
			if (ret)
				return ret;
		}
		return ret;
	}

	template <typename... T>
	cell CallWhile0(std::string const& name, T... args)
	{
		cell ret = static_cast<cell>(DefaultReturnValue_False);
		if (mainScript_)
		{
			ret = mainScript_->Call(name, DefaultReturnValue_False, args...);
			if (ret)
				return ret;
		}
		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_False, args...);
			if (ret)
				return ret;
		}
		return ret;
	}

	template <typename... T>
	cell CallWhile1(char const* name, T... args)
	{
		cell ret = static_cast<cell>(DefaultReturnValue_True);

		if (mainScript_)
		{
			ret = mainScript_->Call(name, DefaultReturnValue_True, args...);
			if (!ret)
				return ret;
		}
		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_True, args...);
			if (!ret)
				return ret;
		}
		return ret;
	}

	template <typename... T>
	cell CallWhile1(std::string const& name, T... args)
	{
		cell ret = static_cast<cell>(DefaultReturnValue_True);

		if (mainScript_)
		{
			ret = mainScript_->Call(name, DefaultReturnValue_True, args...);
			if (!ret)
				return ret;
		}
		for (IPawnScript* cur : scripts_)
		{
			ret = cur->Call(name, DefaultReturnValue_True, args...);
			if (!ret)
				return ret;
		}
		return ret;
	}

	AMX* AMXFromID(int id) const;
	int IDFromAMX(AMX*) const;

	void OnServerCommandList(FlatHashSet<StringView>& commands);
	bool OnServerCommand(const ConsoleCommandSenderData& sender, std::string const& cmd, std::string const& args);

private:
	std::string
		scriptPath_,
		basePath_;
	int
		id_
		= 0;

	void CheckNatives(PawnScript& script);
};
