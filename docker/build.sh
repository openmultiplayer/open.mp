#!/bin/bash

# Available configs: Debug, [RelWithDebInfo], Release
[[ -z "$CONFIG" ]] \
&& config=RelWithDebInfo \
|| config="$CONFIG"
# Available versions: 18.04, [20.04], 22.04
[[ -z "$UBUNTU_VERSION" ]] \
&& ubuntu_version=20.04 \
|| ubuntu_version="$UBUNTU_VERSION"
# Available options: 18.04, [20.04], 22.04
[[ -z "$BUILD_SHARED" ]] \
&& build_shared=1 \
|| build_shared="$BUILD_SHARED"


docker build \
    -t open.mp/build:ubuntu-${ubuntu_version} \
    build_ubuntu-${ubuntu_version}/ \
|| exit 1

folders=('build' 'conan')
for folder in "${folders[@]}"; do
    if [[ ! -d "./${folder}" ]]; then
        mkdir ${folder} &&
        chown 1000:1000 ${folder} || exit 1
    fi
done

docker run \
    --rm \
    -t \
    -w /code \
    -v $PWD/..:/code \
    -v $PWD/build:/code/build \
    -v $PWD/conan:/home/user/.conan \
    -e CONFIG=${config} \
    -e BUILD_SHARED=${build_shared} \
    -e OMP_BUILD_VERSION=$(git rev-list $(git rev-list --max-parents=0 HEAD) HEAD | wc -l) \
    -e OMP_BUILD_COMMIT=$(git rev-parse HEAD) \
    open.mp/build:ubuntu-${ubuntu_version}
