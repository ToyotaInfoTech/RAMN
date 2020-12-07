RAMN (Resistant Automotive Miniature Network) is a miniature CAN/CAN-FD testbed of four Electronic Control Units (ECUs).
Those ECUs can be programmed to emulate the same network traffic as [PASTA](https://github.com/pasta-auto/PASTA1.0), another project from our team.
Demonstrations are uploaded [here](https://twitter.com/ramn_auto).

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_simple_setup.jpg?raw=true" width="1000">

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_board.jpg?raw=true" width="500">   <img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_blockdiagram.jpg?raw=true" width="500">

RAMN is powered by USB and is recognized as a standard CAN adapter (slcan or native socketCAN). No other tools are needed to get started.

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_CAN_interface.jpg?raw=true" width="1000">

RAMN can be connected in closed-loop with the autonomous driving simulator [CARLA](https://github.com/carla-simulator/carla). What happens in the virtual world has an impact on the physical CAN/CAN-FD bus, and vice-versa.

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_simulator.jpg?raw=true" width="1000">

RAMN can be expanded with many stackable expansions:
- screens (OLED, TFT, etc.).
- sensors/actuators (powertrain, chassis, body).
- debugger breakout board (with JTAG connection).
- Trusted Platform Module (TPM).
- external quadSPI memories.

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_advanced_setup.jpg?raw=true" width="1000">

All ECUs can be programmed from the USB port using the microcontrollers' built-in bootloader. Debuggers and flash progammers are **not** needed to get started.

*Copyright (c) 2020 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*

