#pragma once

#include "core.hpp"

/// The plugin's entry point function
/// To be used like so: PLUGIN_ENTRY_POINT(ICore* core) { ... }
#define PLUGIN_ENTRY_POINT extern "C" __declspec(dllexport) bool __cdecl PluginEntryPoint
