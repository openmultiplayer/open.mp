#pragma once

#include "core.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define SDK_EXPORT __declspec(dllexport)
#else
    #define SDK_EXPORT __attribute__((visibility("default")))
#endif

/// The plugin's entry point function
/// To be used like so: PLUGIN_ENTRY_POINT(ICore* core) { ... }
typedef IPlugin* (__cdecl* PluginEntryPoint_t)();
#define PLUGIN_ENTRY_POINT extern "C" SDK_EXPORT IPlugin* __cdecl PluginEntryPoint
