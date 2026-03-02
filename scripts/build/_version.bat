@echo off
rem Centralized STM32CubeIDE version configuration.
rem
rem Docker tag mapping (see https://github.com/xanderhendriks/action-build-stm32cubeide#stm32-cube-ide-versions):
rem   Docker tag = STM32CubeIDE minor version - 3
rem   e.g. STM32CubeIDE 1.10.1 -> 7.0, STM32CubeIDE 1.18.0 -> 15.0
rem
rem Update these variables when upgrading STM32CubeIDE.

SET "STM32CUBEIDEPATH=C:\ST\STM32CubeIDE_1.18.0\STM32CubeIDE"
SET DOCKER_STM32CUBEIDE_TAG=15.0
