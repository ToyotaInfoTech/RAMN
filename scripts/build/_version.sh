#!/bin/bash
# Centralized STM32CubeIDE version configuration.
#
# Docker tag mapping (see https://github.com/xanderhendriks/action-build-stm32cubeide#stm32-cube-ide-versions):
#   Docker tag = STM32CubeIDE minor version - 3
#   e.g. STM32CubeIDE 1.10.1 → 7.0, STM32CubeIDE 1.18.0 → 15.0
#
# Override by setting DOCKER_STM32CUBEIDE_TAG before sourcing this file
# or before calling any docker_*.sh script.

# Parse default docker tag from _version.bat (single source of truth)
_version_bat="$(dirname "${BASH_SOURCE[0]}")/_version.bat"
if [ -f "${_version_bat}" ]; then
	_default_tag=$(sed -n 's/^SET DOCKER_STM32CUBEIDE_TAG=//p' "${_version_bat}" | tr -d '\r')
else
	_default_tag="15.0"
fi
DOCKER_STM32CUBEIDE_TAG="${DOCKER_STM32CUBEIDE_TAG:-${_default_tag}}"
unset _default_tag _version_bat

# STM32CubeIDE >= 1.18.0 (docker tag >= 15.0) replaced -import with -importAll
_major="${DOCKER_STM32CUBEIDE_TAG%%.*}"
if [ "${_major}" -ge 15 ] 2>/dev/null; then
	export STM32_IMPORT_FLAG="-importAll"
else
	export STM32_IMPORT_FLAG="-import"
fi
unset _major
