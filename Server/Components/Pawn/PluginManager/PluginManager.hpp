/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <map>
#include <memory>
#include <string>

#include "../Plugin/Plugin.h"

using namespace Impl;

class PawnPluginManager
{
public:
	FlatHashMap<String, std::unique_ptr<PawnPlugin>> plugins_;
	ICore* core = nullptr;

	PawnPluginManager();
	~PawnPluginManager();

	void SetBasePath(std::string const& path);
	void SetScriptPath(std::string const& path);

	void Load(std::string const& name);
	void Unload(std::string const& name);

	void AmxLoad(AMX* amx);
	void AmxUnload(AMX* amx);

	void ProcessTick();

private:
	std::string
		pluginPath_,
		basePath_;

	void Spawn(std::string const& name);
};
