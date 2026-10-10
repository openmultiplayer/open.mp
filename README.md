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

## Building

See [BUILD.md](BUILD.md) for source checkout, prerequisites, and build instructions for Windows, Linux, and macOS.
