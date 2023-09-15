@echo off

rem see https://github.com/xanderhendriks/action-build-stm32cubeide#stm32-cube-ide-versions
SET DOCKER_STM32CUBEIDE_VERSION=7.0

cd %~dp0\..\..\
docker run --rm -v .:/workspace xanderhendriks/stm32cubeide:%DOCKER_STM32CUBEIDE_VERSION% /workspace/scripts/build/docker_BUILD_Release.sh
