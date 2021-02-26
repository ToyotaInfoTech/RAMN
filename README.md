# RAMN

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_simple_setup.jpg?raw=true" width="600">

RAMN (Resistant Automotive Miniature Network) is a miniature CAN/CAN-FD testbed of four Electronic Control Units (ECUs) consiting solely of Printed Circuit Boards. The ECUs can be programmed to emulate the same network traffic as [PASTA](https://github.com/pasta-auto/PASTA1.0), another project from our team. RAMN is powered over USB and is recognized as a standard CAN adapter (slcan or native socketCAN). It can be connected in closed-loop with the autonomous driving simulator [CARLA](https://github.com/carla-simulator/carla). What happens to the virtual vehicle has an impact on the physical CAN/CAN-FD bus, and vice-versa. RAMN can be expanded with many stackable expansions, ranging from external quadSPI memories to Trusted Platform Modules (TPMs). Please check the [media folder](https://github.com/ToyotaInfoTech/RAMN/tree/main/media) for more details.

## Status
We uploaded better hardware design files on 2021/02/26, and are planning to test the new prototypes by the end of April.
We are currently working on improving the software, which is currently still a beta state.

## Hardware design guidelines
RAMN aims to promote education and research in automotive systems. To stay close to automotive electronics, we use AEC-Qxxx grade 0 components for the ECU network board. However, the ECUs' microcontrollers (STM32L4/STM32L5 series) and connectors are not automotive grade. The board is designed to operate at a temperature range of -40 to 125 degrees Celsius. **This does not mean the board is strictly automotive-grade, or that it can be used in a real automotive system**. To keep the board small and affordable, many protections (e.g. ESD protection, pull-up/pull-down resistors, etc.) are omitted, and the board would not meet reliabilily levels required in safety-critical systems. Expansion boards (sensors, actuators, debuggers, screens, TPM, external memories, etc.) are not designed using automotive grade components. 

All boards are designed using only two layers, with all components on the same side. The boards have large manufacturing tolerances (e.g. large track width and distance), and we restrict components to those with visible external pins (unless there really is not a viable alternative). All those elements ensure that the PCBs can be manufactured and assembled at a low cost.

## Firmware programming guidelines
Firmware is based on [FreeRTOS](https://www.freertos.org/) and [STM32 HAL](https://github.com/STMicroelectronics/STM32CubeL4) packages. Both are compliant with MISRA-C guidelines, often required for automotive software. To avoid overcomplicating the project, we are not following MISRA-C rules for the ECUs' applications - but we might consider doing so in the future.  

To keep the firmware accessible to beginners, configuration of peripherals (e.g. CAN baud rate, etc.) and RTOS definitions (threads, queues, etc.) are done using STM32CubeIDE's graphical interface. The code generation feature of STM32CudeIDE can be used at any time without inadvertently overwriting the ECUs' application code.

## Project structure
### Hardware folder
Contains design files, from KiCAD project files to gerber files and partial BOM (not including common components such as resistors and capacitors).

### Scripts folder
Contains python scripts for 
- Reprogramming ECUs (canboot folder)
- Connecting the board to [CARLA](https://github.com/carla-simulator/carla) (carla folder)

### Firmware folder
Contains firmware of ECUs, with two different project. 
- Firmware of ECU connected to USB (ECU A: Gateway)
- Firmware of ECUs not connected to USB (ECU B, C, D: Powertrain, Chassis, Body)

Firmware of ECUs not connected to USB share the same peripheral configuration (use a C \#define switch to choose the role of an ECU)

### Media folder
Contains presentation documents, pictures and videos.

## License
Please check the LICENSE.md file for more details. 

## Contact
Feel free to [contact us](mailto:camille.gay@toyota-tokyo.tech) if you have questions.
Videos available [here](https://twitter.com/ramn_auto).

Copyright (c) 2020 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
