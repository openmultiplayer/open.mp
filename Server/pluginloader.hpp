#pragma once

#include "impl/core_impl.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #include <Windows.h>
    #define PLUGIN_OPEN(path) LoadLibrary(path)
    #define PLUGIN_GET_ADDR GetProcAddress
    #define PLUGIN_FREE FreeLibrary
#else
    #include <dlfcn.h>
    #define PLUGIN_OPEN(path) dlopen(path, RTLD_LAZY | RTLD_LOCAL)
    #define PLUGIN_GET_ADDR dlsym
    #define PLUGIN_FREE dlclose
#endif

typedef bool (*PluginEntryPoint_t)(ICore* iface);

int loadPlugins(Core &core, const std::filesystem::path &path) {
    std::filesystem::create_directory(path);
    int res = 0;
    for (auto& p : std::filesystem::directory_iterator(path)) {
        auto ext = p.path().extension();
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        if (p.path().extension() == ".dll") {
        #else 
        if (p.path().extension() == ".so") {
        #endif
            core.printLn("Loading plugin %s", p.path().filename().u8string().c_str());
            auto plugin = PLUGIN_OPEN(p.path().u8string().c_str());
            if (plugin == nullptr) {
                core.printLn("\tFailed to load plugin.");
                continue;
            }
            PluginEntryPoint_t OnPluginLoad = reinterpret_cast<PluginEntryPoint_t>(PLUGIN_GET_ADDR(plugin, "PluginEntryPoint"));
            if (OnPluginLoad == nullptr) {
                core.printLn("\tFailed to load plugin.");
                PLUGIN_FREE(plugin);
                continue;
            }
            if (OnPluginLoad(&core)) {
                core.printLn("\tSuccessfully loaded plugin.");
                ++res;
            }
            else {
                core.printLn("\tFailed to load plugin.");
                PLUGIN_FREE(plugin);
            }
        }
    }

    return res;
}