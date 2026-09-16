FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
ARG TARGET_CONTAINER_PLATFORM

# Base layer
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget \
        ca-certificates \
        tzdata \
        openssl \
        python3 \
        libatomic1 \
        libstdc++6 \
        libssl3 && \
    rm -rf /var/lib/apt/lists/* && \
    useradd -m user

# Arch-specific layer
RUN case "$TARGET_CONTAINER_PLATFORM" in \
        linux/amd64) \
            dpkg --add-architecture i386 && \
            apt-get update && \
            apt-get install -y --no-install-recommends \
                libstdc++6:i386 \
                libssl3:i386 \
            ;; \
    esac && \
    rm -rf /var/lib/apt/lists/*

USER user
WORKDIR /home/user

COPY open.mp /opt/open.mp
CMD /opt/open.mp/omp-server
