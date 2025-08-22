# PAST CTFs DATA

This folder contains data to replay past CTFs that used RAMN as a platform for some of their challenges.

Write-ups are available in [RAMN's documentation](https://ramn.readthedocs.io/en/latest/ctf_writeups.html). 

# Recommended Order

If you are new to CTFs, we recommend that you tackle past CTFs in the following order:

- Automotive CTF Japan 2024 (Mostly Easy to Medium).
- Automotive CTF 2024 (Mostly Medium to Difficult).
- CHV2024 (Mostly Difficult to Very Difficult).
- ESV2024 (Mostly Very Difficult, and not automotive related).

# Required Tools

A few challenges may require external hardware to complete (such as a CAN adapter, a UART/SPI/I2C/JTAG adapter, or a logic analyzer).
You can identify such challenges by their tags (e.g., "Hardware", "UART", "I2C", etc.).

# How To Flash CTF Firmware

Replace ``scripts/firmware`` with the ``firmware`` folder inside the CTF folder that you want to replay, then reflash the board using the scripts in ``scripts/STbootloader``.
On Linux, you may need to convert the ECUA.hex file to a .bin file (e.g., using ``objcopy -I ihex -O binary ECUA.hex ECUA.bin``).

The CTF firmware for ECU A may not have the abilitity to flash other ECUs, therefore **you must flash ECU B/C/D first, then flash ECU A**
(ECU A must be flashed with the original firmware to enable it to flash other ECUs).

# How To Revert RAMN to its Original Firmware

Firmware files have been recompiled in order to allow ECU A to go back to DFU mode and allow USB reflashing (a feature that was disabled during actual events).
To reset your RAMN board to the original firmware, revert ``scripts/firmware`` to its original content, then use the scripts in  ``scripts/STbootloader`` to reflash the ECUs.
**You must reflash ECU A first**.

**The ESV firmware does not start in slcan mode - you must first open a USB serial terminal and type ``selfdestruct``, which will put ECU A back in DFU mode**.

# About memory protection

To prevent flag dumping, STM32's (temporary) memory protection was typically activated for every ECU during the events. 
You can enable memory protection with the following commands, but note that **this is not required.** 

```
python ..\canboot.py AUTO B -rp
python ..\canboot.py AUTO C -rp
python ..\canboot.py AUTO D -rp --reset
```

You can use ``scripts/STbootloader/windows/Unlock_BCD.bat`` to remove that protection.
A JTAG adapter is required if you want to enable/disable memory protection for ECU A.


