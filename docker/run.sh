#!/bin/bash

# Available configs: Debug, [RelWithDebInfo], Release
[[ -z "$CONFIG" ]] \
&& config=RelWithDebInfo \
|| config="$CONFIG"
# Available versions: [22.04]
[[ -z "$UBUNTU_VERSION" ]] \
&& ubuntu_version=22.04 \
|| ubuntu_version="$UBUNTU_VERSION"


omp_path="build/Output/${config}/Server"

if [ ! -f "${omp_path}/omp-server" ]; then
    CONFIG=${config} UBUNTU_VERSION=${ubuntu_version} ./build.sh
fi

cp -r \
    ${omp_path} \
    run_ubuntu-${ubuntu_version}/open.mp \
;
if [ $? -ne 0 ]; then
    echo 'Error copying open.mp files for container build, exiting.'
    exit 1
fi

docker build \
    -t open.mp/run:ubuntu-${ubuntu_version} \
    run_ubuntu-${ubuntu_version}/ \
;
docker_error=$?
rm -rf run_ubuntu-${ubuntu_version}/open.mp

if [ $docker_error -ne 0 ]; then
    echo 'Error building open.mp container, exiting.'
    exit 1
fi

[ ! -d './data' ] && mkdir data

docker run \
    --rm \
    -ti \
    -w /data \
    -v $PWD/data:/data \
    -p 7777:7777 \
    -p 7777:7777/udp \
    open.mp/run:ubuntu-${ubuntu_version}
