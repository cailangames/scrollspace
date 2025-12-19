FROM docker.io/ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt install -y \
        wget \
        unzip \
        git \
        build-essential && \
    rm -rf /var/lib/apt/lists/*

RUN wget -O /tmp/gbdk-source.tar.gz https://github.com/gbdk-2020/gbdk-2020/archive/refs/tags/4.3.0.tar.gz 

RUN wget -O /tmp/sdcc.tar.gz https://github.com/gbdk-2020/gbdk-2020-sdcc/releases/download/sdcc-patched-gbdk-4.3.0/sdcc-14650-Linux-arm64.tar.gz 

RUN wget -O /tmp/hUGEDriver.zip https://github.com/SuperDisk/hUGEDriver/releases/download/v6.1.1/hUGEDriver-6.1.1.zip

RUN mkdir -p /opt/hUGEDriver-6

RUN tar -xzf /tmp/gbdk-source.tar.gz -C /tmp
RUN tar -xzf /tmp/sdcc.tar.gz -C /opt
RUN unzip /tmp/hUGEDriver.zip -d /opt/hUGEDriver-6


RUN cd /tmp/gbdk-2020-4.3.0 && \
    export SDCCDIR=/opt/sdcc && \
    make && \
    make install

RUN mkdir -p /home/root/scrollspace
WORKDIR /home/root/scrollspace

ENV PATH="/opt/gbdk/bin:${PATH}"
CMD ["bash"]
