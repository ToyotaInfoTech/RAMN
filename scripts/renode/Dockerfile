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
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/renode
COPY --from=renode-builder /build/renode/output/bin/Release /opt/renode/
COPY --from=renode-builder /build/renode/scripts /opt/renode/scripts
COPY --from=renode-builder /build/renode/platforms /opt/renode/platforms

# Mark Renode root directory
RUN touch /opt/renode/.renode-root

# Create renode launcher wrapper
RUN printf '#!/usr/bin/env bash\nexec dotnet /opt/renode/Renode.dll "$@"\n' > /usr/local/bin/renode \
    && chmod +x /usr/local/bin/renode

ENV PATH="/opt/renode:${PATH}"

EXPOSE 29536
