#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include <sdk.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Console/console.hpp>
#include <Server/Components/Databases/databases.hpp>
#include <Server/Components/Dialogs/dialogs.hpp>
#include <Server/Components/GangZones/gangzones.hpp>
#include <Server/Components/Menus/menus.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <Server/Components/TextDraws/textdraws.hpp>
#include <Server/Components/TextLabels/textlabels.hpp>
#include <Server/Components/Timers/timers.hpp>
#include <Server/Components/Variables/variables.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>

#include <string>
#include <map>
#include <algorithm>
#include <memory>	

#include "../Script/Script.hpp"
#include "../Singleton.hpp"

class PawnManager : public Singleton<PawnManager>
{
public:
	FlatHashMap<String, std::unique_ptr<PawnScript>> scripts_;
	std::string entryScript = "";
	FlatHashMap<AMX*, PawnScript*> amxToScript_;
	ICore* core = nullptr;
	IConfig* config = nullptr;
	IPlayerPool* players = nullptr;
	IActorsComponent* actors = nullptr;
	ICheckpointsComponent* checkpoints = nullptr;
	IClassesComponent* classes = nullptr;
	IConsoleComponent* console = nullptr;
	IDatabasesComponent* databases = nullptr;
	IDialogsComponent* dialogs = nullptr;
	IGangZonesComponent* gangzones = nullptr;
	IMenusComponent* menus = nullptr;
	IObjectsComponent* objects = nullptr;
	IPickupsComponent* pickups = nullptr;
	ITextDrawsComponent* textdraws = nullptr;
	ITextLabelsComponent* textlabels = nullptr;
	ITimersComponent* timers = nullptr;
	IVariablesComponent* vars = nullptr;
	IVehiclesComponent* vehicles = nullptr;

	PawnManager();
	~PawnManager();

	void printPawnLog(const std::string & type, const std::string & message) {
		core->printLn("[PAWN-LOG] %s: %s", type.c_str(), message.c_str());
	}

	void OnScriptInit(const std::string & script);
	void OnScriptExit(const std::string & script);

	void SetBasePath(std::string const & path);
	void SetScriptPath(std::string const & path);

	void Load(std::string const & name, bool primary = false);
	void Reload(std::string const & name);
	void Unload(std::string const & name);

	template <typename ... T>
	cell CallAllInSidesFirst(char const* name, T ... args)
	{
		cell
			ret = 0;

		PawnScript * first = nullptr;
		for (auto& cur : scripts_) {
			if (cur.first == entryScript) {
				first = cur.second.get();
			}
			else {
				ret = cur.second->Call(name, args...);
			}
		}

		if (first != nullptr) {
			ret = first->Call(name, args...);
		}

		return ret;
	}

	template <typename ... T>
	cell CallAllInEntryFirst(char const* name, T ... args)
	{
		cell
			ret = 0;

		FlatHashMap<String, std::unique_ptr<PawnScript>>::const_iterator const& first = scripts_.find(entryScript);
		if (first != scripts_.end()) {
			ret = first->second->Call(name, args...);
		}

		for (auto& cur : scripts_) {
			if (cur.first == entryScript) {
				continue;
			}
			else {
				ret = cur.second->Call(name, args...);
			}
		}

		return ret;
	}

	template <typename ... T>
	cell CallInSidesWhile0(char const* name, T ... args)
	{
		cell
			ret = 0;

		for (auto& cur : scripts_) {
			if (cur.first == entryScript) {
				continue;
			}
			else {
				ret = cur.second->Call(name, args...);
				if (ret) {
					break;
				}
			}
		}

		return ret;
	}

	template <typename ... T>
	cell CallInSidesWhile1(char const* name, T ... args)
	{
		cell
			ret = 0;

		for (auto& cur : scripts_) {
			if (cur.first == entryScript) {
				continue;
			}
			else {
				ret = cur.second->Call(name, args...);
				if (!ret) {
					break;
				}
			}
		}

		return ret;
	}

	template <typename ... T>
	cell CallInSides(char const* name, T ... args)
	{
		cell
			ret = 0;

		for (auto& cur : scripts_) {
			if (cur.first == entryScript) {
				continue;
			}
			else {
				ret = cur.second->Call(name, args...);
			}
		}

		return ret;
	}

	template <typename ... T>
	cell CallInEntry(char const* name, T ... args)
	{
		cell
			ret = 0;

		FlatHashMap<String, std::unique_ptr<PawnScript>>::const_iterator const & first = scripts_.find(entryScript);
		if (first != scripts_.end()) {
			ret = first->second->Call(name, args...);
		}

		return ret;
	}

	template <typename ... T>
	cell CallAll(char const * name, T ... args)
	{
		cell
			ret = 0;
		for (auto & cur : scripts_)
		{
			ret = cur.second->Call(name, args...);
		}
		return ret;
	}

	template <typename ... T>
	cell CallAll(std::string const & name, T ... args)
	{
		cell
			ret = 0;
		for (auto & cur : scripts_)
		{
			ret = cur.second->Call(name, args...);
		}
		return ret;
	}

	template <typename ... T>
	cell CallWhile0(char const * name, T ... args)
	{
		cell
			ret = 0;
		for (auto & cur : scripts_)
		{
			ret = cur.second->Call(name, args...);
			if (ret)
				return ret;
		}
		return ret;
	}

	template <typename ... T>
	cell CallWhile0(std::string const & name, T ... args)
	{
		cell
			ret = 0;
		for (auto & cur : scripts_)
		{
			ret = cur.second->Call(name, args...);
			if (ret)
				return ret;
		}
		return ret;
	}

	template <typename ... T>
	cell CallWhile1(char const * name, T ... args)
	{
		cell
			ret = 0;
		for (auto & cur : scripts_)
		{
			ret = cur.second->Call(name, args...);
			if (!ret)
				return ret;
		}
		return ret;
	}

	template <typename ... T>
	cell CallWhile1(std::string const & name, T ... args)
	{
		cell
			ret = 0;
		for (auto & cur : scripts_)
		{
			ret = cur.second->Call(name, args...);
			if (!ret)
				return ret;
		}
		return ret;
	}

	AMX * AMXFromID(int id) const;
	int IDFromAMX(AMX *) const;

private:
	std::string
		scriptPath_,
		basePath_;
	int
		id_ = 0;

	friend cell AMX_NATIVE_CALL pawn_Script_CallAll(AMX * amx, cell const * params);
	friend int ComponentCallGM(char * name);
	friend int ComponentCallFS(char * name);

	void Spawn(std::string const & name);
	bool OnServerCommand(std::string const & cmd, std::string const & args);

	void CheckNatives(PawnScript & script);
};

