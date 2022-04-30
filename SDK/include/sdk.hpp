#pragma once

#include "core.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define SDK_EXPORT __declspec(dllexport)
#else
#define SDK_EXPORT __attribute__((visibility("default")))
#endif

/// The component's entry point function
/// To be used like so: COMPONENT_ENTRY_POINT(ICore* core) { ... }
typedef IComponent*(__CDECL* ComponentEntryPoint_t)();
#define COMPONENT_ENTRY_POINT extern "C" SDK_EXPORT IComponent* __CDECL ComponentEntryPoint
