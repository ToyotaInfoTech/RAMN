Canboot is a tool written in python to reprogram RAMN's ECU over CAN using STM32's built-in CAN bootloader. Check STMicroelectronics [AN3154](st.com/resource/en/application_note/cd00264321-can-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf) for more details.  

It can be used to reprogram ECUs B, C, and D. 
It requires ECU A to be programmed with a compatible firmware. ECU A can be reprogrammed using DFU over USB.

Usage is described in the help command on the program.

'''
python canboot.py --help
'''

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/media/pictures/ramn_reprogram.jpg?raw=true" width="1000">

*Copyright (c) 2020 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*