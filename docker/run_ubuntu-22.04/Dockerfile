FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN \
    dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        wget \
        libstdc++6:i386 \
        tzdata \
    && \
    wget \
        http://security.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2.16_i386.deb \
        -P /tmp \
    && \
    dpkg -i /tmp/libssl1.1_1.1.1f-1ubuntu2.16_i386.deb && \
    wget \
        http://security.ubuntu.com/ubuntu/pool/main/o/openssl/openssl_1.1.1f-1ubuntu2.16_i386.deb \
        -P /tmp \
    && \
    dpkg -i /tmp/openssl_1.1.1f-1ubuntu2.16_i386.deb && \
    rm \
        /tmp/libssl1.1_1.1.1f-1ubuntu2.16_i386.deb \
        /tmp/openssl_1.1.1f-1ubuntu2.16_i386.deb \
    && \
    useradd -m user

USER user
WORKDIR /home/user

COPY open.mp /opt/open.mp
CMD /opt/open.mp/omp-server
