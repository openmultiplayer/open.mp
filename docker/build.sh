#!/bin/bash

# Available configs: Debug, [RelWithDebInfo], Release
[[ -z "$CONFIG" ]] \
&& config=RelWithDebInfo \
|| config="$CONFIG"
# Available versions: 18.04, [20.04], 22.04
[[ -z "$UBUNTU_VERSION" ]] \
&& ubuntu_version=20.04 \
|| ubuntu_version="$UBUNTU_VERSION"
# Available options: [true], false
[[ -z "$BUILD_SHARED" ]] \
&& build_shared=1 \
|| build_shared="$BUILD_SHARED"
# Available options: [true], false
[[ -z "$BUILD_SERVER" ]] \
&& build_server=1 \
|| build_server="$BUILD_SERVER"
# Available options: true, [false]
[[ -z "$BUILD_TOOLS" ]] \
&& build_tools=0 \
|| build_tools="$BUILD_TOOLS"
# Available options: [x86], x86_64, armv4, armv4i, armv5el, armv5hf, armv6, armv7, armv7hf, armv7s, armv7k, armv8, armv8_32, armv8.3
[[ -z "$TARGET_BUILD_ARCH" ]] \
&& target_build_arch=x86 \
|| target_build_arch="$TARGET_BUILD_ARCH"
# Available container runtimes: [docker], podman, other 
[[ -z "$CONTAINER_RUNTIME" ]] \
&& container_runtime=docker \
|| container_runtime="$CONTAINER_RUNTIME"
[[ "$container_runtime" == "podman" ]] \
&& volume_suffix=":Z" \
|| volume_suffix=""
[[ "$container_runtime" == "podman" ]] \
&& userns_option="--userns=keep-id" \
|| userns_option=""

$container_runtime build \
    -t open.mp/build:ubuntu-${ubuntu_version} \
    build_ubuntu-${ubuntu_version}/ \
|| exit 1

folders=('build' 'conan2')
for folder in "${folders[@]}"; do
    if [[ ! -d "./${folder}" ]]; then
        mkdir ${folder}
    fi
    sudo chown -R 1000:1000 ${folder} || exit 1
done

$container_runtime run \
    --rm \
    -t \
    $userns_option \
    -w /code \
    -v $PWD/..:/code${volume_suffix} \
    -v $PWD/build:/code/build${volume_suffix} \
    -v $PWD/conan2:/home/user/.conan2${volume_suffix} \
    -e CONFIG=${config} \
    -e TARGET_BUILD_ARCH=${target_build_arch} \
    -e BUILD_SHARED=${build_shared} \
    -e BUILD_SERVER=${build_server} \
    -e BUILD_TOOLS=${build_tools} \
    -e OMP_BUILD_VERSION=$(git rev-list $(git rev-list --max-parents=0 HEAD) HEAD | wc -l) \
    -e OMP_BUILD_COMMIT=$(git rev-parse HEAD) \
    open.mp/build:ubuntu-${ubuntu_version}
