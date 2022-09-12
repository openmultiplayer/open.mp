FROM ubuntu:18.04
RUN \
    dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y \
        gpg \
        wget \
    && \
    wget -O- https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | \
    tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' | \
    tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    apt-get update && \
    apt-get install -y \
        cmake \
        ninja-build \
        clang-10 \
        python3-pip \
        gcc-8-multilib \
        g++-8-multilib \
        libstdc++6:i386 \
        libc6:i386 \
    && \
    useradd -m user && \
    su user -c 'pip3 install --user conan'

USER user

ENV CC=/usr/bin/clang-10 \
    CXX=/usr/bin/clang++-10 \
    PATH=~/.local/bin:${PATH}

COPY docker-entrypoint.sh /
CMD /docker-entrypoint.sh
