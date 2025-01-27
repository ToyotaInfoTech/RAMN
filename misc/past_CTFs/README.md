# PAST CTFs DATA

This folder contains data to replay past CTFs using RAMN as a platform for some of their challenges.

Write-ups are available in [RAMN's documentation](https://ramn.readthedocs.io/en/latest/ctf_writeups.html). 

Firmware files have been recompiled in order to reenable firmware reprogramming features, and may not be exactly identical to the actual firmware files used during the events. 

To prevent flag dumping, STM32's (temporary) memory protection was typically activated for every ECU during the events. 

A JTAG debugger may be required to revert RAMN to the standard firmware after flashing a CTF firmware.

