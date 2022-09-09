open.mp Linux build image
=========================

How to use:

1. `docker build -t open.mp/build:ubuntu-20.04 .`
1. `docker run --rm -ti -v /path/to/omp/sources:/omp -w /omp open.mp/build:ubuntu-20.04`
1. ???
1. Profit! Built open.mp is available in /path/to/omp/sources/build
1. Note: You can use `-e CONFIG=Debug` or `-e CONFIG=RelWithDebInfo` in the `docker run` call to change the build type
