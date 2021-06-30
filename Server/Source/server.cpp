#include <iostream>
#include <filesystem>
struct IUnknown;
#include <Windows.h>
#include "core_impl.hpp"
#include <nlohmann/json.hpp>
#include <sdk.hpp>

std::vector<IPlugin*> loadPlugins(Core &core, const std::filesystem::path &path) {
    std::vector<IPlugin*> res;
    std::filesystem::create_directory(path);
    for (auto& p : std::filesystem::directory_iterator(path)) {
        auto ext = p.path().extension();
        if (p.path().extension() == ".dll") {
            core.printLn("Loading plugin %s", p.path().filename().u8string().c_str());
            HMODULE pluginLib = LoadLibrary(p.path().u8string().c_str());
            if (pluginLib == nullptr) {
                core.printLn("\tFailed to load plugin.");
                continue;
            }
            PluginEntryPoint_t OnPluginLoad = reinterpret_cast<PluginEntryPoint_t>(GetProcAddress(pluginLib, "PluginEntryPoint"));
            if (OnPluginLoad == nullptr) {
                core.printLn("\tFailed to load plugin.");
                FreeLibrary(pluginLib);
                continue;
            }
            IPlugin* plugin = OnPluginLoad(&core);
            if (plugin != nullptr) {
                core.printLn("\tSuccessfully loaded with UUID %016llx", plugin->getUUID());
                res.push_back(plugin);
            }
            else {
                core.printLn("\tFailed to load plugin.");
                FreeLibrary(pluginLib);
            }
        }
    }

    return res;
}

int main()
{
    Core* core = new Core();
    std::vector<IPlugin*> plugins = loadPlugins(*core, "components");
    core->printLn("Loaded %i plugin(s)", plugins.size());
    core->addPlugins(plugins);
    core->eventDispatcher.dispatch(&CoreEventHandler::onInit);
    core->run();
}
