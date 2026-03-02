# RAMN

<img src="https://ramn.readthedocs.io/en/latest/_images/ramn_simple_setup.jpg" width="600">

RAMN (Resistant Automotive Miniature Network) is a miniature CAN/CAN-FD testbed of four Electronic Control Units (ECUs) consiting solely of Printed Circuit Boards. The ECUs can be programmed to emulate the same network traffic as [PASTA](https://github.com/pasta-auto/PASTA1.0), another project from our team. RAMN is powered over USB and can be recognized as a standard CAN adapter. It can be connected in closed-loop with the autonomous driving simulator [CARLA](https://github.com/carla-simulator/carla). What happens to the virtual vehicle has an impact on the physical CAN/CAN-FD bus, and vice versa. RAMN can be expanded with many stackable expansions, ranging from external quadSPI memories to Trusted Platform Modules (TPMs).

Please check the [Documentation](https://ramn.readthedocs.io/) for demonstrations and details.

[![build all, clean release and debug](https://github.com/ToyotaInfoTech/RAMN/actions/workflows/build_all.yml/badge.svg)](https://github.com/ToyotaInfoTech/RAMN/actions/workflows/build_all.yml)

## Fabricating RAMN boards

This project is open-source and RAMN boards can be fabricated by most PCB assembly prototyping services.

We have setup 're-orders' at PCBWay. 
Please [see the docs 'PCB Ordering' for more details](https://ramn.readthedocs.io/page/hardware/Fabrication/ordering.html).
You can also request a prototype from any PCB manufacturer, using the Gerber files and BOM in the Hardware folder. 

Feel free to [contact us](mailto:camille.gay@toyota.global) if you need assistance to fabricate or sell RAMN boards.

Because component availability is subject to change, we are periodically updating the hardware.
The original RAMN hardware data is available in misc/RAMN_V1_reference_PCB.zip, and is recommended for academic purposes.

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
Contains firmware source code for RAMN's ECUs.

## License
Please check the LICENSE.md file for more details.

## Contact
Feel free to [contact us](mailto:camille.gay@toyota.global) if you have questions.
Videos available [here](https://twitter.com/ramn_auto).

Copyright (c) 2025 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
