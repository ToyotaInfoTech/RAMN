# RAMN

<img src="https://ramn.readthedocs.io/en/latest/_images/ramn_simple_setup.jpg" width="600">

RAMN (Resistant Automotive Miniature Network) is a miniature CAN/CAN-FD testbed of four Electronic Control Units (ECUs) consiting solely of Printed Circuit Boards. The ECUs can be programmed to emulate the same network traffic as [PASTA](https://github.com/pasta-auto/PASTA1.0), another project from our team. RAMN is powered over USB and can be recognized as a standard CAN adapter (slcan). It can be connected in closed-loop with the autonomous driving simulator [CARLA](https://github.com/carla-simulator/carla). What happens to the virtual vehicle has an impact on the physical CAN/CAN-FD bus, and vice versa. RAMN can be expanded with many stackable expansions, ranging from external quadSPI memories to Trusted Platform Modules (TPMs). 

Please check the [Documentation](https://ramn.readthedocs.io/) for demonstrations and details.
If RAMN is missing a feature that you need, or is incompatible with a tool that you are using, feel free to [contact us](mailto:camille.gay@toyota.global) to request an update.

[![build all, clean release and debug](https://github.com/BenGardiner/RAMN/actions/workflows/build_all.yml/badge.svg)](https://github.com/BenGardiner/RAMN/actions/workflows/build_all.yml) [^1]

[^1]: pointing temporarily at a fork while this org's policies prevent automated builds)

## Obtaining a board

Currently, the only method to obtain a board is to request a prototype from a PCB manufacturer, using the gerber files and BOM in the Hardware folder. The cost should typically be around 200 USD per set.
Feel free to [contact us](mailto:camille.gay@toyota.global) if you need assistance to fabricate RAMN boards, or want a link to a shared order.

## Project structure
### Hardware folder
Contains design files, from KiCAD project files to Gerber files and BOM.

### Scripts folder

Contains various scripts to interact with the ECUs:
- Driving Simulator
- Diagnostics (UDS/KWP2000/XCP)
- Hardware Bootloader
- Connection to a virtual CAN Network

### Firmware folder
Contains firmware source code for RAMN's ECUs. All ECUs share a common firmware, select the target by defining one of TARGET_ECUA, TARGET_ECUB, TARGET_ECUC or TARGET_ECUD.

## License
Please check the LICENSE.md file for more details. 

## Contact
Feel free to [contact us](mailto:camille.gay@toyota.global) if you have questions.
Videos available [here](https://twitter.com/ramn_auto).

Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
