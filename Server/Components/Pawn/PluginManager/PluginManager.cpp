/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <ghc/filesystem.hpp>
#include "PluginManager.hpp"
#include "../utils.hpp"

struct BrokenPluginMessageData
{
	StringView name;
	StringView message;
};

static const StaticArray<BrokenPluginMessageData, 23> BrokenPlugins = {
	{
		{ "YSF", "It requires memory hacking to run and is therefore broken on open.mp, we already added many built-in features from YSF to open.mp and the rest are coming" },
		{ "YSF_DL", "It requires memory hacking to run and is therefore broken on open.mp, we already added many built-in features from YSF to open.mp and the rest are coming" },
		{ "YSF_static", "It requires memory hacking to run and is therefore broken on open.mp, we already added many built-in features from YSF to open.mp and the rest are coming" },
		{ "YSF_DL_static", "It requires memory hacking to run and is therefore broken on open.mp, we already added many built-in features from YSF to open.mp and the rest are coming" },
		{ "SKY", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "FCNPC", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "FCNPC-DL", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "sampcac_server", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "ASAN", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "samp-custom-query-flood-check", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "AntiVehicleSpawn", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "mcmd", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "KeyListener", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "chandlingsvr", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },
		{ "samp_akm", "It requires memory hacking to run and is therefore broken on open.mp. There should be a replacement component supported by open.mp" },

		{ "pawnraknet", "There is an open.mp compatible version you can find here: https://github.com/katursis/Pawn.RakNet/releases , make sure to download x.x.x-omp version." },
		{ "pawncmd", "There is an open.mp compatible version you can find here: https://github.com/katursis/Pawn.CMD/releases , make sure to download x.x.x-omp version." },
		{ "sampvoice", "There is an open.mp compatible version you can find here: https://github.com/AmyrAhmady/sampvoice/releases , make sure to download x.x.x-omp version." },

		{ "nativechecker", "It is not needed anymore since open.mp has built in native checking mechanism when a script is being loaded" },
		{ "samp-compat", "It is not needed anymore since open.mp has built in compat mechanism between 0.3.7 and 0.3DL versions" },
		{ "LFN", "It is not needed anymore since open.mp has support for longer function names, just compile your scripts with our compiler" },
		{ "raktimefix", "It is not needed anymore since open.mp has no stability issues on the latest linux systems" },
		{ "bscrashfix", "It is not needed anymore since open.mp has no raknet layer issues causing to crashes" },
	}
};

PawnPluginManager::PawnPluginManager()
	: pluginPath_("plugins/")
	, basePath_("./")
{
}

PawnPluginManager::~PawnPluginManager()
{
	for (auto& cur : plugins_)
	{
		cur.second->Unload();
	}
}

void PawnPluginManager::Load(std::string const& name)
{
	if (plugins_.count(name))
	{
		return;
	}

	String pluginName = ghc::filesystem::path(name).stem().string();
	for (BrokenPluginMessageData brokenPlugin : BrokenPlugins)
	{
		if (pluginName == brokenPlugin.name)
		{
			core->logLn(LogLevel::Error,
				"Skipping legacy plugin '%.*s'; %.*s",
				PRINT_VIEW(brokenPlugin.name), PRINT_VIEW(brokenPlugin.message));
			return;
		}
	}

	Spawn(name);
}

void PawnPluginManager::Unload(std::string const& name)
{
	auto
		pos
		= plugins_.find(name);
	if (pos == plugins_.end())
	{
		return;
	}
	auto& plugin = *pos->second;
	plugin.Unload();
	plugins_.erase(pos);
}

void PawnPluginManager::Spawn(std::string const& name)
{
	// if the user just supplied a script name, add the extension
	// otherwise, don't, as they may have supplied a full abs/rel path.
	// std::string ext = utils::endsWith(name, ".amx") ? "" : ".amx";

	std::string canon;
#ifndef WIN32
	size_t pos = name.rfind(".so");
	// You would think this could be done in one comparison, but the path may
	// only be two characters long.
	if (pos == std::string::npos || pos + 3 != name.length())
	{
		// Append the extension.
		utils::Canonicalise(basePath_ + pluginPath_ + name + ".so", canon);
	}
	else
#endif // WIN32
	{
		utils::Canonicalise(basePath_ + pluginPath_ + name, canon);
	}

	core->printLn("Loading plugin: %s", name.c_str());

	std::unique_ptr<PawnPlugin> ptr = std::make_unique<PawnPlugin>(canon, core);

	if (!ptr.get()->IsLoaded())
	{
		// core->printLn("Unable to load plugin %s\n\n", name.c_str());
		return;
	}

	plugins_.emplace(name, std::move(ptr));
}

void PawnPluginManager::AmxLoad(AMX* amx)
{
	for (auto& cur : plugins_)
	{
		cur.second->AmxLoad(amx);
	}
}

void PawnPluginManager::AmxUnload(AMX* amx)
{
	for (auto& cur : plugins_)
	{
		cur.second->AmxUnload(amx);
	}
}

void PawnPluginManager::ProcessTick()
{
	for (auto& cur : plugins_)
	{
		cur.second->ProcessTick();
	}
}

void PawnPluginManager::SetBasePath(std::string const& path)
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

void PawnPluginManager::SetScriptPath(std::string const& path)
{
	if (path.length() == 0)
	{
		pluginPath_ = "/";
	}
	else if (path.back() == '/')
	{
		pluginPath_ = path;
	}
	else
	{
		pluginPath_ = path + '/';
	}
}
