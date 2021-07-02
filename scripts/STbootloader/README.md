
# Canboot

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/docs/gif/reprog.gif?raw=true" width="1000">

Canboot is a tool written in python to reprogram RAMN's ECUs over CAN using STM32's built-in CAN or CAN-FD bootloader. 

It can be used to reprogram ECUs B, C, and D. 

ECU A can be reprogrammed using DFU over USB.

## Usage

Usage is described in the help command of the script.


```
$python canboot.py --help
```

Helper scripts are located in **linux** and **windows** folder, to reprogram ECUs with the firmware files located  in the **firmware** folder. They require to input the serial port manually.

## Details

Microcontrollers of the STM32L5 family can be reprogrammed over CAN-FD based on STMicroelectronics [AN5405](https://www.st.com/resource/en/application_note/dm00660346-fdcan-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf).  

Microcontrollers of the STM32L4 family can be reprogrammed over CAN based on STMicroelectronics [AN3154](st.com/resource/en/application_note/cd00264321-can-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf). Script for classic CAN is located in the BETA folder.

Microcontrollers of the STM32L4/STM32L5 can be reprogrammed over USB (DFU) based on STMicroelectronics [AN3156](https://www.st.com/resource/en/application_note/cd00264379-usb-dfu-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf).


## DFU (ECU A)


Script ECUA_goToDFU.py can be used to ask ECU A to go into DFU mode.

```
$python ECUA_goToDFU.py <serial_port>
```

ECU A can then be reprogrammed on Linux with dfu-util, or on Windows with STM32_Programmer_CLI.exe

### Linux


```
$dfu-util  -d 0x0483:0xdf11 -c1 -a0 -D "ECUA.hex"
```

### Windows

```
$STM32_Programmer_CLI.exe -c port=usb1 pid=0xdf11 vid=0x0483 -d ..\..\firmware\ECUA.hex --verify --start 0x08000000 
```

## Other

*Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*