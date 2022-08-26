FROM ubuntu:18.04
RUN \
    apt-get update && \
    apt-get install -y \
        gpg \
        wget \
        software-properties-common \
    && \
    wget -O- https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | \
    tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' | \
    tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get install -y \
        cmake \
        ninja-build \
        clang-10 \
        python3-pip \
        gcc-9-multilib \
        g++-9-multilib \
    && \
    useradd -m user && \
    su user -c 'pip3 install --user conan'

USER user

ENV CC=/usr/bin/clang-10 \
    CXX=/usr/bin/clang++-10 \
    PATH=~/.local/bin:${PATH}

ADD docker-entrypoint.sh /
CMD /docker-entrypoint.sh
