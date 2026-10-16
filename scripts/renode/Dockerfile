# Multi-stage Dockerfile for Renode Simulator at commit 9d398ff4
# Stage 1: Build Renode from source
FROM mcr.microsoft.com/dotnet/sdk:8.0 AS renode-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    make \
    cmake \
    automake \
    autoconf \
    libtool \
    g++ \
    python3 \
    python3-pip \
    libgtk2.0-0 \
    libglib2.0-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
RUN git clone https://github.com/renode/renode.git && cd renode && git checkout 9d398ff439f43710e889c63c7b7fb3a0ed97a0c0

WORKDIR /build/renode
RUN ./build.sh --net

# Stage 2: Runtime image
FROM mcr.microsoft.com/dotnet/runtime:8.0-bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libglib2.0-0 \
    libgtk2.0-0 \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/renode
COPY --from=renode-builder /build/renode/output/bin/Release /opt/renode/
COPY --from=renode-builder /build/renode/scripts /opt/renode/scripts
COPY --from=renode-builder /build/renode/platforms /opt/renode/platforms

# Pre-download STM32L552.svd at build time so Renode never downloads it from the web at runtime
ADD https://dl.antmicro.com/projects/renode/svd/STM32L552.svd /opt/renode/platforms/cpus/STM32L552.svd
RUN sed -i 's|@https://dl.antmicro.com/projects/renode/svd/STM32L552.svd|@/opt/renode/platforms/cpus/STM32L552.svd|g' /opt/renode/platforms/cpus/stm32l552.repl

# Mark Renode root directory
RUN touch /opt/renode/.renode-root

# Create renode launcher wrapper
RUN printf '#!/usr/bin/env bash\nexec dotnet /opt/renode/Renode.dll "$@"\n' > /usr/local/bin/renode \
    && chmod +x /usr/local/bin/renode \
    && cp /usr/local/bin/renode /opt/renode/renode

# Copy simulation script and superserver proxy
COPY ramn_4ecu.resc /opt/renode/ramn_4ecu.resc
COPY simulator_proxy.py /opt/renode/simulator_proxy.py
COPY entrypoint.sh /opt/renode/entrypoint.sh
RUN chmod +x /opt/renode/entrypoint.sh /opt/renode/simulator_proxy.py

ENV PATH="/opt/renode:${PATH}" \
    DOTNET_gcServer=0 \
    DOTNET_TieredPGO=0 \
    DOTNET_ReadyToRun=1

EXPOSE 29536

ENTRYPOINT ["/opt/renode/entrypoint.sh"]
