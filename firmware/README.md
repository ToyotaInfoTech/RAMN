# RAMN Firmware

This folder contains the firmware for RAMN V1. 
Documentation is available [here](https://ramn.readthedocs.io/en/latest/).

RAMNV1 contains the original firmware for RAMN ECUs.

RAMNV1_CTF contains the same firmware, but with a RAM layout arranged so that it can be used as a template for automotive CTF (Capture The Flag) events.

## About CTF Firmware Template

The STM32L5 can only block access to SRAM2, not SRAM1, at least not without permanently disabling JTAG. 
The CTF firmware is arranged so that only SRAM2, which can be protected, is used by the ECU. 
Memory protection can be activated using the RDP option bytes ("0xDC": non-permanent protection) or the canboot.py script (-rp option). Protection can be released at any time, but will result in the erasure of FLASH and RAM data.

This makes it possible to implement various embedded CTF challenges. Because JTAG is not disabled, PWN challenges can be implemented as well. Because SRAM2 can be protected, flags cannot trivially be recovered by dumping ECU memory.

CTF firmware can only be used for ECU B, C, and D. ECU settings use large timeout values to make RAMN more accessible to beginners.


Copyright (c) 2022 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
