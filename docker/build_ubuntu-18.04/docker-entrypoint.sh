#!/bin/sh
[ -z $CONFIG ] && config=Release || config="$CONFIG"
[ -z $BUILD_SHARED ] && build_shared=Release || build_shared="$BUILD_SHARED"

cmake \
    -S . \
    -B build \
    -G Ninja \
    -DCMAKE_C_FLAGS=-m32 \
    -DCMAKE_CXX_FLAGS=-m32 \
    -DCMAKE_BUILD_TYPE=$config \
    -DSHARED_OPENSSL=$build_shared \
    -DSTATIC_STDCXX=true \
&&
cmake \
    --build build \
    --config $config \
    --parallel $(nproc)
