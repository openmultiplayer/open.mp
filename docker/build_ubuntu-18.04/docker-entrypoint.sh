#!/bin/sh
[ -z $CONFIG ] && config=Release || config="$CONFIG"
[ -z $BUILD_SHARED ] && build_shared=1 || build_shared="$BUILD_SHARED"
[ -z $BUILD_SERVER ] && build_server=1 || build_server="$BUILD_SERVER"
[ -z $BUILD_TOOLS ] && build_tools=0 || build_tools="$BUILD_TOOLS"
[ -z $TARGET_BUILD_ARCH ] && target_build_arch=x86 || target_build_arch="$TARGET_BUILD_ARCH"

cd build \
&&
cmake .. \
    -G Ninja \
    -DTARGET_BUILD_ARCH=$target_build_arch \
    -DCMAKE_BUILD_TYPE=$config \
    -DSHARED_OPENSSL=$build_shared \
    -DSTATIC_STDCXX=true \
    -DBUILD_SERVER=$build_server \
    -DBUILD_ABI_CHECK_TOOL=$build_tools \
&&
cmake \
    --build . \
    --config $config \
    --parallel $(nproc)
