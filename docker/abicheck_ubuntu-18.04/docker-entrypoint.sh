#!/bin/bash
[ -z $CONFIG ] && config=Release || config="$CONFIG"
[ -z $BUILD_SHARED ] && build_shared=1 || build_shared="$BUILD_SHARED"
[ -z $BUILD_SERVER ] && build_server=1 || build_server="$BUILD_SERVER"
[ -z $BUILD_TOOLS ] && build_tools=0 || build_tools="$BUILD_TOOLS"
[ -z $NIGHTLY_REPO ] && nightly_repo="openmultiplayer/open.mp" || nightly_repo="$NIGHTLY_REPO"

set -e

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

cd build/Output/*/Tools
echo "Fetching ref artifacts"
url=$(curl "https://nightly.link/$NIGHTLY_REPO/workflows/build/master" | grep -oP "(?<=\")https://nightly.link/$NIGHTLY_REPO/workflows/build/master/open.mp-linux-x86-dynssl.*\.zip(?=\")")
curl -L $url -o master.zip
for z in master.zip; do unzip "$z"; mv "$(unzip -Z1 $z)" "master.tar.xz"; done
echo "Extracting ref artifacts"
ls /ref
mkdir -p ref && tar -xvf "/ref/$REF_ARCHIVE" -C ref
echo "Extracting master artifacts"
mkdir -p master && tar -xvf "./master.tar.xz" -C master

binaries=(
    "omp-server"
    components/*.so
)

echo "Collecting files"
cd master/Server
files=$(echo ${binaries[@]})
cd ../..

set +e
ret=0

for file in $files; do
    echo "Processing $file"
    diff --minimal <(./abi-check "master/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*") <(./abi-check "ref/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*") > /dev/null
    if [ $? -ne 0 ]; then
        sdiff -s --minimal <(./abi-check "master/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*") <(./abi-check "ref/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*")
        echo "Possible ABI break in $file; trying with no names"
        res=$(diff --minimal <(./abi-check "master/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*" --no-names) <(./abi-check "ref/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*" --no-names))
        if [ $? -ne 0 ]; then
            # This is done to skip additions only by making sure lines in master differ
            echo "$res" | grep -G -m 1 '^<' > /dev/null
            if [ $? -eq 0 ]; then
                sdiff -s --minimal <(./abi-check "master/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*" --no-names) <(./abi-check "ref/Server/$file" --incl ".*/SDK/include/.*" --excl ".*/Impl/.*" --no-names)
                ret=1
                echo "ABI break in $file; setting ret to $ret"
            fi
        fi
    fi
done

exit $ret
