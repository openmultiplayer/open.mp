#include <iostream>
#include <filesystem>
struct IUnknown;
#include <Windows.h>
#include "core_impl.hpp"
#include <nlohmann/json.hpp>
#include <exports.hpp>

typedef bool (__cdecl *PluginEntryPoint_t)(ICore* iface);

int loadPlugins(Core &core, const std::filesystem::path &path) {
    std::filesystem::create_directory(path);
    int res = 0;
    for (auto& p : std::filesystem::directory_iterator(path)) {
        auto ext = p.path().extension();
        if (p.path().extension() == ".dll") {
            core.printLn("Loading plugin %s", p.path().filename().u8string().c_str());
            HMODULE plugin = LoadLibrary(p.path().u8string().c_str());
            if (plugin == nullptr) {
                core.printLn("\tFailed to load plugin.");
                continue;
            }
            PluginEntryPoint_t OnPluginLoad = reinterpret_cast<PluginEntryPoint_t>(GetProcAddress(plugin, "PluginEntryPoint"));
            if (OnPluginLoad == nullptr) {
                core.printLn("\tFailed to load plugin.");
                FreeLibrary(plugin);
                continue;
            }
            if (OnPluginLoad(&core)) {
                core.printLn("\tSuccessfully loaded plugin.");
                ++res;
            }
            else {
                core.printLn("\tFailed to load plugin.");
                FreeLibrary(plugin);
            }
        }
    }

    return res;
}

int main()
{
    Core* core = new Core();
    int loaded = loadPlugins(*core, "components");
    core->printLn("Loaded %i plugin(s)", loaded);
    core->eventDispatcher.dispatch(&CoreEventHandler::onInit);
    core->run();
}
