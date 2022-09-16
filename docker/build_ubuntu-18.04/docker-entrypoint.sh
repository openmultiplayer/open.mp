#!/bin/sh
[ -z $CONFIG ] && config=Release || config="$CONFIG"
[ -z $BUILD_SHARED ] && build_shared=1 || build_shared="$BUILD_SHARED"
[ -z $BUILD_SERVER ] && build_server=1 || build_server="$BUILD_SERVER"
[ -z $BUILD_TOOLS ] && build_tools=0 || build_tools="$BUILD_TOOLS"

cmake \
    -S . \
    -B build \
    -G Ninja \
    -DCMAKE_C_FLAGS=-m32 \
    -DCMAKE_CXX_FLAGS=-m32 \
    -DCMAKE_BUILD_TYPE=$config \
    -DSHARED_OPENSSL=$build_shared \
    -DSTATIC_STDCXX=true \
    -DBUILD_SERVER=$build_server \
    -DBUILD_ABI_CHECK_TOOL=$build_tools \
&&
cmake \
    --build build \
    --config $config \
    --parallel $(nproc)
