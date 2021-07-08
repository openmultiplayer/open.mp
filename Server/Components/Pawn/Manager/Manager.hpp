#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include "sdk.hpp"

#include <string>
#include <map>
#include <algorithm>
#include <memory>	

#include "../Script/Script.hpp"
#include "../Singleton.hpp"

class PawnManager : public Singleton<PawnManager>
{
public:
	std::map<std::string, std::unique_ptr<PawnScript>> scripts_;
	std::string entryScript = "";
	ICore * serverCore;

	PawnManager();
	~PawnManager();

	void SetServerCoreInstance(ICore * core) {
		serverCore = core;
	};

	void printPawnLog(const std::string & type, const std::string & message) {
		serverCore->printLn("[PAWN-LOG] %s: %s", type.c_str(), message.c_str());
	}

	void OnScriptInit(const std::string & script);
	void OnScriptExit(const std::string & script);

	void SetBasePath(std::string const & path);
	void SetScriptPath(std::string const & path);

	void Load(std::string const & name, bool primary = false);
	void Reload(std::string const & name);
	void Unload(std::string const & name);

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
	friend int PluginCallGM(char * name);
	friend int PluginCallFS(char * name);

	void Spawn(std::string const & name);
	bool OnServerCommand(std::string const & cmd, std::string const & args);

	void CheckNatives(PawnScript & script);
};

