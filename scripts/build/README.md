
# Build tools

## Content

This folder contains various scripts for headless builds of RAMN ECUs' firmware. Script names correspond to the build settings.

## Usage

Modify files in the **settings** folder to match your environment. Especially, make sure **FIRMWARE_WORKSPACE.txt** contains the path to the workspace where the firmware source code folder is located.

Then, run one of the script to build the firmware for all ECUs. Recommended script is **BUILD_Clean_Release.bat**.

## Requirements

STM32CubeIDE needs to be installed on the computer running the script. It is expected that STM32CubeIDE is installed in the default folder. If is not the case, the variable *STM32CUBEIDEPATH* of each script should be updated with the path to the actual install folder.


*Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*