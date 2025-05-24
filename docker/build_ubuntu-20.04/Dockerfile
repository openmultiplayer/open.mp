FROM ubuntu:20.04
RUN \
    dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y \
        gpg \
        wget \
    && \
    apt-get install -y \
        ninja-build \
        clang-10 \
        python3-pip \
        gcc-9-multilib \
        g++-9-multilib \
        libstdc++-10-dev:i386 \
    && \
    wget https://cmake.org/files/v3.20/cmake-3.20.6-linux-x86_64.tar.gz  && \
    tar zxvf cmake-3.20.6-linux-x86_64.tar.gz && \
    mv cmake-3.20.6-linux-x86_64       /opt/cmake-3.20.6 && \
    ln -sf  /opt/cmake-3.20.6/bin/*    /usr/bin/ && \
    useradd -m user && \
    su user -c 'pip3 install --user -v "conan==1.57.0"'

USER user

ENV CC=/usr/bin/clang-10 \
    CXX=/usr/bin/clang++-10 \
    PATH=~/.local/bin:${PATH}

COPY docker-entrypoint.sh /
CMD /docker-entrypoint.sh
