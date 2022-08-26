#!/bin/sh
[ -z $CONFIG ] && config=Release || config="$CONFIG"

cmake \
    -S . \
    -B build \
    -G Ninja \
    -DCMAKE_C_FLAGS=-m32 \
    -DCMAKE_CXX_FLAGS=-m32 \
    -DCMAKE_BUILD_TYPE=$config \
&&
cmake \
    --build build \
    --config $config \
    --parallel $(nproc)
