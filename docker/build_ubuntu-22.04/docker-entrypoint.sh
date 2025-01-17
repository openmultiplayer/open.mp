#!/bin/sh
[ -z $CONFIG ] && config=Release || config="$CONFIG"
[ -z $TARGET_BUILD_ARCH ] && target_build_arch=x86 || target_build_arch="$TARGET_BUILD_ARCH"

cd build \
&& \
cmake .. \
    -G Ninja \
    -DTARGET_BUILD_ARCH=$target_build_arch \
    -DCMAKE_BUILD_TYPE=$config \
&& \
cmake \
    --build . \
    --config $config \
    --parallel $(nproc)