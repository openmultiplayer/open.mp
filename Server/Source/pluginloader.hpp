#pragma once

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



std::vector<IPlugin*> loadPlugins(Core &core, const std::filesystem::path &path) {
    std::vector<IPlugin*> res;
    std::filesystem::create_directory(path);
    for (auto& p : std::filesystem::directory_iterator(path)) {
        auto ext = p.path().extension();
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        if (p.path().extension() == ".dll") {
        #else 
        if (p.path().extension() == ".so") {
        #endif
            core.printLn("Loading plugin %s", p.path().filename().u8string().c_str());
            HMODULE pluginLib = PLUGIN_OPEN(p.path().u8string().c_str());
            if (pluginLib == nullptr) {
                core.printLn("\tFailed to load plugin.");
                continue;
            }
            PluginEntryPoint_t OnPluginLoad = reinterpret_cast<PluginEntryPoint_t>(PLUGIN_GET_ADDR(pluginLib, "PluginEntryPoint"));
            if (OnPluginLoad == nullptr) {
                core.printLn("\tFailed to load plugin.");
                PLUGIN_FREE(pluginLib);
                continue;
            }
            IPlugin* plugin = OnPluginLoad(&core);
            if (plugin != nullptr) {
                core.printLn("\tSuccessfully loaded with UUID %016llx", plugin->getUUID());
                res.push_back(plugin);
            }
            else {
                core.printLn("\tFailed to load plugin.");
                PLUGIN_FREE(pluginLib);
            }
        }
    }

    return res;
}