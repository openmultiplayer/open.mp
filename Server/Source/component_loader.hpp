#pragma once

#include <types.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    struct IUnknown;
    #include <Windows.h>
    #define COMPONENT_OPEN(path) LoadLibrary(path)
    #define COMPONENT_GET_ADDR GetProcAddress
    #define COMPONENT_FREE FreeLibrary
#else
    #include <dlfcn.h>
    #define COMPONENT_OPEN(path) dlopen(path, RTLD_LAZY | RTLD_LOCAL)
    #define COMPONENT_GET_ADDR dlsym
    #define COMPONENT_FREE dlclose
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define COMPONENT_EXT ".dll"
#elif __APPLE__
    #define COMPONENT_EXT ".dylib"
#else
    #define COMPONENT_EXT ".so"
#endif

IComponent* loadComponent(Core& core, const std::filesystem::path& path) {
    core.printLn("Loading component %s", path.filename().u8string().c_str());
    auto componentLib = COMPONENT_OPEN(path.u8string().c_str());
    if (componentLib == nullptr) {
        core.printLn("\tFailed to load component.");
        return nullptr;
    }
    ComponentEntryPoint_t OnComponentLoad = reinterpret_cast<ComponentEntryPoint_t>(COMPONENT_GET_ADDR(componentLib, "ComponentEntryPoint"));
    if (OnComponentLoad == nullptr) {
        core.printLn("\tFailed to load component.");
        COMPONENT_FREE(componentLib);
        return nullptr;
    }
    IComponent* component = OnComponentLoad();
    if (component != nullptr) {
        core.printLn("\tSuccessfully loaded component %s with UUID %016llx", component->componentName().data(), component->getUUID());
        return component;
    }
    else {
        core.printLn("\tFailed to load component.");
        COMPONENT_FREE(componentLib);
        return nullptr;
    }
}

DynamicArray<IComponent*> loadComponents(Core &core, const std::filesystem::path &path) {
    DynamicArray<IComponent*> res;
    std::filesystem::create_directory(path);

    Span<const StringView> components = core.config.getStrings("components");
    if (components.empty()) {
        for (auto& p : std::filesystem::directory_iterator(path)) {
            if (p.path().extension() == COMPONENT_EXT) {
                IComponent* component = loadComponent(core, p);
                if (component) {
                    res.push_back(component);
                }
            }
        }
    }
    else {
        for (const StringView component : components) {
            auto file = std::filesystem::path(path) / component;
            if (!file.has_extension()) {
                file.replace_extension(COMPONENT_EXT);
            }

            if (std::filesystem::exists(file)) {
                IComponent* component = loadComponent(core, file);
                if (component) {
                    res.push_back(component);
                }
            }
        }
    }

    return res;
}
