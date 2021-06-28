#pragma once

#include "core.hpp"

/// The plugin's entry point function
/// To be used like so: PLUGIN_ENTRY_POINT(ICore* core) { ... }
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define SDK_EXPORT __declspec(dllexport)
#else
    #define SDK_EXPORT__attribute__((visibility("default")))
#endif
#define PLUGIN_ENTRY_POINT extern "C" __declspec(dllexport) bool __cdecl PluginEntryPoint
