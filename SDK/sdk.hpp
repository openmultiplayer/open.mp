#pragma once

#include "core.hpp"

/// The plugin's entry point function
/// To be used like so: PLUGIN_ENTRY_POINT(ICore* core) { ... }
typedef IPlugin* (__cdecl* PluginEntryPoint_t)(ICore*);
#define PLUGIN_ENTRY_POINT extern "C" __declspec(dllexport) IPlugin* __cdecl PluginEntryPoint
