#include "PluginManager.hpp"
#include "../utils.hpp"

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
    //std::string ext = utils::endsWith(name, ".amx") ? "" : ".amx";

    std::string canon;
    utils::Canonicalise(basePath_ + pluginPath_ + name, canon);

    core->printLn("Loading plugin: %s", name.c_str());

    std::unique_ptr<PawnPlugin> ptr = std::make_unique<PawnPlugin>(canon, core);

    if (!ptr.get()->IsLoaded()) {
        core->printLn("Unable to load plugin %s\n\n", name.c_str());
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
