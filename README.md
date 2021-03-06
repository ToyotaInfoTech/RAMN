# RAMN

<img src="https://ramn.readthedocs.io/en/latest/_images/ramn_simple_setup.jpg" width="600">

RAMN (Resistant Automotive Miniature Network) is a miniature CAN/CAN-FD testbed of four Electronic Control Units (ECUs) consiting solely of Printed Circuit Boards. The ECUs can be programmed to emulate the same network traffic as [PASTA](https://github.com/pasta-auto/PASTA1.0), another project from our team. RAMN is powered over USB and is recognized as a standard CAN adapter (slcan or native socketCAN). It can be connected in closed-loop with the autonomous driving simulator [CARLA](https://github.com/carla-simulator/carla). What happens to the virtual vehicle has an impact on the physical CAN/CAN-FD bus, and vice-versa. RAMN can be expanded with many stackable expansions, ranging from external quadSPI memories to Trusted Platform Modules (TPMs). 

Please check the [Documentation](https://ramn.readthedocs.io/) for demonstrations and details.

## Firmware programming guidelines
Firmware is based on [FreeRTOS](https://www.freertos.org/) and [STM32 HAL](https://github.com/STMicroelectronics/STM32CubeL4) packages. Both are compliant with MISRA-C guidelines, often required for automotive software. To avoid overcomplicating the project, we are not following MISRA-C rules for the ECUs' applications - but we might consider doing so in the future.  

To keep the firmware accessible to beginners, configuration of peripherals (e.g. CAN baud rate, etc.) and RTOS definitions (threads, queues, etc.) are done using STM32CubeIDE's graphical interface. The code generation feature of STM32CudeIDE can be used at any time without inadvertently overwriting the ECUs' application code.

## Project structure
### Hardware folder
Contains design files, from KiCAD project files to gerber files and partial BOM (not including common components such as resistors and capacitors).

### Scripts folder

Contains various scripts to interact with the ECUs:
- Driving Simulator
- Diagnostics (UDS/KWP2000/XCP)
- Hardware Bootloader
- Connection to a virtual CAN Network

### Firmware folder
Contains firmware source code for RAMN's ECUs. All ECUs share a common firmware, select the target defining one of TARGET_ECUA, TARGET_ECUB, TARGET_ECUC or TARGET_ECUD.


## License
Please check the LICENSE.md file for more details. 

## Contact
Feel free to [contact us](mailto:camille.gay@toyota-tokyo.tech) if you have questions.
Videos available [here](https://twitter.com/ramn_auto).

Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
