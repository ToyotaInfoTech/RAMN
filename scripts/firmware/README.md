
# Firmware


This folder contains firmware files for RAMNs ECUs. By default, this folder is used as output for the **build** scripts, and as input by the reprogram/verify **diagnostics** scripts.

## Variant firmware

Pre-built `.hex` files for firmware variants are available in subdirectories:

| Directory | Description |
|-----------|-------------|
| `gsusb/`  | GS_USB (candleLight) mode enabled on ECU A |
| `kwp/`    | KWP2000 protocol enabled |
| `uart/`   | UART enabled (CDC & USB disabled) |
| `watchdog/` | Hardware watchdog enabled |
| `no_uds/` | UDS and reprogramming disabled |
| `no_extras/` | J1979, MiniCTF, CHIP8, screen, SPI disabled |
| `no_debug/` | USB debug, joystick, screen, CHIP8, runtime stats disabled |
| `no_dynamic_bitrate/` | Dynamic CAN bitrate switching disabled |

Each subdirectory contains `ECUA.hex` through `ECUD.hex` for that variant configuration.
These files are automatically updated by CI on every push to `main`.

*Copyright (c) 2021 STMicroelectronics. All rights reserved.*
*Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*

**This software component is licensed by ST under BSD 3-Clause license, the "License"; You may not use this file except in compliance with the License. You may obtain a copy of the License at: opensource.org/licenses/BSD-3-Clause**