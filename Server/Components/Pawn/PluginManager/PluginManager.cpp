/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "PluginManager.hpp"
#include "../utils.hpp"
#include <filesystem>

static const StaticArray<StringView, 13> BrokenPlugins = {
    "YSF",
    "YSF_DL",
    "YSF_static",
    "YSF_DL_static",
    "pawnraknet",
    "pawncmd",
    "SKY",
    "FCNPC",
    "FCNPC-DL",
    "sampcac_server",
    "sampvoice",
    "rustext",
    "ASAN",
};

PawnPluginManager::PawnPluginManager()
    : pluginPath_("plugins/")
    , basePath_("./")
{
}

PawnPluginManager::~PawnPluginManager()
{
    for (auto& cur : plugins_) {
        cur.second->Unload();
    }
}

void PawnPluginManager::Load(std::string const& name)
{
    if (plugins_.count(name)) {
        return;
    }

    String pluginName = std::filesystem::path(name).stem().string();
    for (StringView brokenPlugin : BrokenPlugins) {
        if (pluginName == brokenPlugin) {
            core->logLn(LogLevel::Error,
                "The legacy plugin '%.*s' requires memory hacking to run and is therefore broken on open.mp, skipping it. There should be a replacement component supported by open.mp",
                PRINT_VIEW(brokenPlugin));
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
    if (pos == plugins_.end()) {
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
    utils::Canonicalise(basePath_ + pluginPath_ + name, canon);

    core->printLn("Loading plugin: %s", name.c_str());

    std::unique_ptr<PawnPlugin> ptr = std::make_unique<PawnPlugin>(canon, core);

    if (!ptr.get()->IsLoaded()) {
        // core->printLn("Unable to load plugin %s\n\n", name.c_str());
        return;
    }

    plugins_.emplace(name, std::move(ptr));
}

void PawnPluginManager::AmxLoad(AMX* amx)
{
    for (auto& cur : plugins_) {
        cur.second->AmxLoad(amx);
    }
}

void PawnPluginManager::AmxUnload(AMX* amx)
{
    for (auto& cur : plugins_) {
        cur.second->AmxUnload(amx);
    }
}

void PawnPluginManager::ProcessTick()
{
    for (auto& cur : plugins_) {
        cur.second->ProcessTick();
    }
}

void PawnPluginManager::SetBasePath(std::string const& path)
{
    if (path.length() == 0) {
        basePath_ = "/";
    } else if (path.back() == '/') {
        basePath_ = path;
    } else {
        basePath_ = path + '/';
    }
}

void PawnPluginManager::SetScriptPath(std::string const& path)
{
    if (path.length() == 0) {
        pluginPath_ = "/";
    } else if (path.back() == '/') {
        pluginPath_ = path;
    } else {
        pluginPath_ = path + '/';
    }
}
