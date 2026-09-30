FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN \
    dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        wget \
        libstdc++6:i386 \
        libatomic1:i386 \
        tzdata \
        python3 \
        openssl \
        libssl3 \
        libssl3:i386 \
        libssl-dev \
    && \
    useradd -m user

COPY healthcheck.py /usr/local/bin/healthcheck.py
RUN chmod +x /usr/local/bin/healthcheck.py

USER user
WORKDIR /home/user

COPY open.mp /opt/open.mp
CMD /opt/open.mp/omp-server

HEALTHCHECK --start-period=3m --interval=30s --timeout=5s --retries=3 CMD /usr/local/bin/healthcheck.py
