# Scripts

## Content

This folder contains various scripts to interact with RAMN over USB.

- the **build** folder contains scripts to automatically compile firmware for all 4 ECUs.
- the **carla** folder contains scripts to connect RAMN to the driving simulator CARLA.
- the **diagnostics** folder contains scripts to interact with RAMN's ECU over UDS, KWP and XCP.
- the **firmware** folder contains most recently built firmware files.
- the **settings** folder contains environment settings (e.g., serial port name, paths, etc.).
- the **STBootloader** folder contains scripts to interact with STM32 hardware bootloader.
- the **tests** folder contains scripts to perform reliability tests.
- the **vcand** folder contains scripts scripts to connect RAMN to a socketCAN virtual network (Linux only).

## Usage

Modify files in the **settings** folder to match your environment. After that, it should be possible to run the scripts without any argument.

*Copyright (c) 2025 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*