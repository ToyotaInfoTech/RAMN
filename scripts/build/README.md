
# Build tools

## Content

This folder contains various scripts for headless builds of RAMN ECUs' firmware.

### Script naming convention

- `BUILD_Clean_{Release,Debug}.{sh,bat}` — Clean-build all ECUs locally
- `BUILD_{Release,Debug}.{sh,bat}` — Incremental-build all ECUs locally
- `docker_BUILD_Clean_{Release,Debug}.{sh,bat}` — Clean-build all ECUs via Docker
- `docker_BUILD_{Release,Debug}.{sh,bat}` — Incremental-build all ECUs via Docker
- `_build_ecu.sh` — Inner build script (called by the above)
- `_version.sh` / `_version.bat` — Centralized STM32CubeIDE version config

## Usage

### Docker builds (recommended)

Run one of the `docker_BUILD_*.sh` (Linux/macOS) or `docker_BUILD_*.bat` (Windows) scripts.
Recommended: **docker_BUILD_Clean_Release.sh**.

Override the Docker image version via environment variable:

    DOCKER_STM32CUBEIDE_TAG=7.0 bash scripts/build/docker_BUILD_Clean_Release.sh

### Local builds

Requires STM32CubeIDE installed locally. For Windows, modify **_version.bat** to point to your
STM32CubeIDE install path, and ensure **../settings/FIRMWARE_WORKSPACE.txt** contains the workspace path.

For Linux/macOS, ensure `stm32cubeide` and `headless-build.sh` are in your PATH, then
run one of the `BUILD_*.sh` scripts.

## Version configuration

All scripts share version settings from `_version.sh` (Docker/Linux) and `_version.bat` (Windows).
Update these files when upgrading STM32CubeIDE.


*Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*