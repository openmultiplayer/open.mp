FROM ubuntu:22.04
ARG TARGET_CONTAINER_BUILD_ARCH

# Fix for some rootless builders
ENV DEBIAN_FRONTEND=noninteractive

# Base layer
RUN apt-get update && \
    apt-get install -y \
        cmake \
        ninja-build \
        clang-11 \
        python3-pip && \
    rm -rf /var/lib/apt/lists/* && \
    useradd -m user && \
    su user -c 'pip3 install --user conan'

# Arch-specific layer
RUN case "$TARGET_CONTAINER_BUILD_ARCH" in \
        x86_64|x86) \
            dpkg --add-architecture i386 && \
            apt-get update && \
            apt-get install -y \
                gcc-9-multilib \
                g++-9-multilib \
                libstdc++-11-dev:i386 \
            ;; \
        arm*) \
            apt-get update && \
            apt-get install -y \
                libstdc++6 \
                libc6 \
            ;; \
        *) \
            echo "Unsupported TARGET_CONTAINER_BUILD_ARCH: $TARGET_CONTAINER_BUILD_ARCH" >&2 \
            exit 1 \
            ;; \
    esac && \
    rm -rf /var/lib/apt/lists/*

USER user

ENV CC=/usr/bin/clang-11 \
    CXX=/usr/bin/clang++-11 \
    PATH=/home/user/.local/bin:${PATH}

COPY docker-entrypoint.sh /
CMD /docker-entrypoint.sh
