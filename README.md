# open.mp

![status](https://github.com/AmyrAhmady/oben.mb/workflows/Build/badge.svg)

## Structure

* SDK/ - Core SDK headers (stable between versions)
* SDK/NetCode/ - Netcode headers (RPC and packet read/write structures, NOT stable between versions)
* SDK/Server/Components/*/ - Components/plug-in SDK headers (stable between versions)
* lib/ - Various submodules and CMake helpers for Conan
* Server/Source - Core server implementation (NOT stable between versions, do NOT use headers outside the Source folder)
* Server/Components/*/ - Components/plug-in implementation (NOT stable between versions, do NOT use headers outside the component's folder)

## Tools

- [CMake 3.11+](https://cmake.org/)
- [Conan 1.33+](https://conan.io/)

## Tools on Windows

- [Visual Studio 2019+](https://www.visualstudio.com/)

Visual Studio needs the `Desktop development with C++` workload with the `MSVC v142`, `Windows 10 SDK` and `C++ Clang tools for Windows` components.

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
```

Open Visual Studio and build the solution.
