# open.mp

![status](https://github.com/openmultiplayer/open.mp/workflows/Build/badge.svg)

## Structure

| Path | Content |
| ---- | ------- |
| `SDK/include` | Core SDK headers (stable between versions) |
| `SDK/include/Server/Components/*/` | Components/plug-in SDK headers (stable between versions) |
| `Shared/NetCode/` | Netcode headers (RPC and packet read/write structures, NOT stable between versions) |
| `Shared/Network/` | Network utility headers (NOT stable between versions) |
| `lib/` | Various submodules and third-party libraries |
| `Server/Source/` | Core server implementation (NOT stable between versions, do NOT use headers outside the Source folder) |
| `Server/Components/*/` | Components/plug-in implementation (NOT stable between versions, do NOT use headers outside the component's folder) |

## Concepts

| Name | Description |
| ---- | ------- |
| Entity | Something that can appear in the 3D world of the game |
| Pool | Container of something with limited amount of IDs |
| Component | Something that's conceptually different enough it can be separated into its own module |
| Extensible | Something to which extensions can be added to preserve ABI compatibility |
| Extension | Something which adds additional functionality to an extensible and preserves ABI compatibility |

## Tools

* [CMake 3.19+](https://cmake.org/)
* [Conan 1.57+](https://conan.io/) [(You can't use conan v2.x, so may need to download directly from github releases)](https://github.com/conan-io/conan/releases)

## Tools on Windows

* [Visual Studio 2019+](https://www.visualstudio.com/)

Visual Studio needs the `Desktop development with C++` workload with the `C++ Clang tools for Windows` component.

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
