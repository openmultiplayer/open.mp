#!/bin/sh
[ ! -e build ] && mkdir build
[ -z $CONFIG ] && config=Release || config="$CONFIG"

(
    export \
        CC=/usr/bin/clang-10 \
        CXX=/usr/bin/clang++-10 \
    &&
    cd build &&
    cmake .. \
        -G Ninja \
        -DCMAKE_C_FLAGS=-m32 \
        -DCMAKE_CXX_FLAGS=-m32 \
        -DCMAKE_BUILD_TYPE=$config \
    &&
    cmake --build . --config $config
)
