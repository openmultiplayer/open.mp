FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN \
    dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        wget \
        libstdc++6:i386 \
        tzdata \
        openssl \
        libssl3 \
        libssl-dev \
    && \
    useradd -m user

USER user
WORKDIR /home/user

COPY open.mp /opt/open.mp
CMD /opt/open.mp/omp-server
