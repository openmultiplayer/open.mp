#pragma once

#include <map>
#include <memory>
#include <string>

#include "../Plugin/Plugin.h"
#include "../Singleton.hpp"

class PawnPluginManager {
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
