#pragma once


#ifdef OMP_EXPORTS
#define OMP_API __declspec(dllexport)
#else
#define OMP_API __declspec(dllimport)
#endif

#include "core.hpp"

#define PLUGIN_ENTRY_POINT extern "C" __declspec(dllexport) bool __cdecl PluginEntryPoint
