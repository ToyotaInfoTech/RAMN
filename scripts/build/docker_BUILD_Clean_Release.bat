@echo off
call "%~dp0_version.bat"
cd /d "%~dp0..\.."
docker run --rm -v ".:/workspace" xanderhendriks/stm32cubeide:%DOCKER_STM32CUBEIDE_TAG% /workspace/scripts/build/docker_BUILD_Clean_Release.sh
