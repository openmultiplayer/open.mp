# open.mp

![status](https://github.com/openmultiplayer/open.mp/workflows/Build/badge.svg)

## Structure

| Path | Content |
| ---- | ------- |
| `SDK/` | Core SDK headers (stable between versions) |
| `SDK/Server/Components/*/` | Components/plug-in SDK headers (stable between versions) |
| `Shared/NetCode/` | Netcode headers (RPC and packet read/write structures, NOT stable between versions) |
| `lib/` | Various submodules and CMake helpers for Conan |
| `Server/Source/` | Core server implementation (NOT stable between versions, do NOT use headers outside the Source folder) |
| `Server/Components/*/` | Components/plug-in implementation (NOT stable between versions, do NOT use headers outside the component's folder) |

## Tools

* [CMake 3.19+](https://cmake.org/)
* [Conan 1.57+](https://conan.io/) (Can't use conan v2.x)

## Tools on Windows

* [Visual Studio 2019+](https://www.visualstudio.com/)

Visual Studio needs the `Desktop development with C++` workload with the `MSVC v143`, `Windows 11 SDK`, `C++ CMake tools for Windows` and `C++ Clang tools for Windows` components.

## Sources

```bash
# With HTTPS:
git clone --recursive https://github.com/openmultiplayer/open.mp
# With SSH:
git clone --recursive git@github.com:openmultiplayer/open.mp
```

Note the use of the `--recursive` argument, because this repository contains submodules.

## Building on Windows

```bash
cd open.mp
mkdir build
cd build
cmake .. -A Win32 -T ClangCL
cmake --build . --config RelWithDebInfo
```
