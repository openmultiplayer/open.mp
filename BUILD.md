# Building open.mp

## Get the source

Clone the repository with its submodules using HTTPS:

```bash
git clone --recursive https://github.com/openmultiplayer/open.mp
```

Or use SSH: `git clone --recursive git@github.com:openmultiplayer/open.mp`.

If you already cloned without `--recursive`, run `git submodule update --init --recursive` from the repository root. Git checks out the submodule revisions required by the repository.

## Windows

Install [CMake 3.19 or later](https://cmake.org/), [Conan 2.x](https://conan.io/) (`pip install conan` or `pip3 install conan`), and [Visual Studio 2019 or later](https://visualstudio.microsoft.com/). In Visual Studio, select the **Desktop development with C++** workload and the **C++ Clang tools for Windows** component.

From the directory containing the checkout, run:

```powershell
cd open.mp
mkdir build
cd build
cmake .. -A Win32 -T ClangCL
cmake --build . --config RelWithDebInfo
```

The server executable and components are written to `build/Output/RelWithDebInfo/Server/`.

## Linux (Docker)

Install Docker and make sure your user can run `docker`. The build script also uses `sudo` to set ownership of its build and Conan cache directories. CMake, Conan, and the compiler run inside the Docker image, so they do not need to be installed on the host.

Run the script from the repository's `docker` directory; its image paths and output directories are relative to that directory:

```bash
cd open.mp/docker
bash build.sh
```

The script builds the selected Ubuntu image, runs CMake and Ninja in a container, and keeps the build and Conan cache in `docker/build/` and `docker/conan2/` on the host. It also passes the current Git commit and build number into the container. With the defaults below, the server executable and `components/` directory are in `docker/build/Output/RelWithDebInfo/Server/`.

`build.sh` has no positional arguments. Set any of these environment variables before running it:

| Variable | Default | Effect |
| ---- | ---- | ---- |
| `CONFIG` | `RelWithDebInfo` | CMake build configuration: `Debug`, `RelWithDebInfo`, or `Release`. This also determines the directory under `docker/build/Output/`. |
| `UBUNTU_VERSION` | `20.04` | Ubuntu build image: `18.04`, `20.04`, or `22.04`. The script builds the corresponding `docker/build_ubuntu-*` image before running it. |
| `BUILD_SHARED` | `1` (true) | Sets CMake's `SHARED_OPENSSL` option. Use `0` (false) to link OpenSSL statically. |
| `BUILD_SERVER` | `1` (true) | Sets CMake's `BUILD_SERVER` option. Use `0` (false) to omit the server build. |
| `BUILD_TOOLS` | `0` (false) | Sets CMake's `BUILD_ABI_CHECK_TOOL` option. Use `1` (true) to build the ABI check tool. |
| `TARGET_BUILD_ARCH` | `x86` | Target architecture passed to CMake. `x86` is a 32-bit build; `x86_64` is a 64-bit build. Other values listed by the script are below. |

The script lists these additional `TARGET_BUILD_ARCH` values: `armv4`, `armv4i`, `armv5el`, `armv5hf`, `armv6`, `armv7`, `armv7hf`, `armv7s`, `armv7k`, `armv8`, `armv8_32`, and `armv8.3`. Setting a target architecture does not install a cross compiler; the selected image must have the required toolchain.

For example, to build a 64-bit Release server using the Ubuntu 22.04 image:

```bash
CONFIG=Release UBUNTU_VERSION=22.04 TARGET_BUILD_ARCH=x86_64 bash build.sh
```

That build's server output is in `docker/build/Output/Release/Server/`. If `BUILD_SERVER=0`, there will be no server executable in the output. If `BUILD_TOOLS=1`, the ABI check tool is written under `docker/build/Output/<CONFIG>/Tools/`.

## macOS

The macOS CI job builds for ARM using Apple Clang, Ninja, CMake, and Conan 2.x. Install the Xcode command line tools, Python 3, and Homebrew. CI pins CMake 3.31.8; Homebrew may install a different version. The commands below follow the CI configuration; Intel macOS is not covered by that job.

```bash
cd open.mp
brew install cmake ninja

mkdir -p build
python3 -m venv build/conan-venv
source build/conan-venv/bin/activate
python -m pip install --upgrade pip
python -m pip install conan

export SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"
export MACOSX_DEPLOYMENT_TARGET=11.0
export CFLAGS="-isysroot ${SDKROOT}"
export CXXFLAGS="-isysroot ${SDKROOT}"
export LDFLAGS="-isysroot ${SDKROOT}"

cmake -S . -B build -G Ninja \
  -DCMAKE_OSX_SYSROOT="${SDKROOT}" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET}" \
  -DTARGET_BUILD_ARCH=arm64 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DSHARED_OPENSSL=true \
  -DSTATIC_STDCXX=false \
  -DBUILD_SERVER=1 \
  -DBUILD_ABI_CHECK_TOOL=0

cmake --build build --config RelWithDebInfo --parallel "$(sysctl -n hw.logicalcpu)"
```

The server executable and components are written to `build/Output/RelWithDebInfo/Server/`. To match CI's static OpenSSL variant, set `-DSHARED_OPENSSL=false` when configuring.
