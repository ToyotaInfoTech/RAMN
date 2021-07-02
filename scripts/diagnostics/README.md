

# Diagnostics Tools

This folder contains various Python scripts to interact with RAMN's Diagnostics interface: UDS, KWP2000 and XCP.

## Usage

Modify either **settings/linux.ini** or **settings/windows.ini** based on your platform.

Scripts may be run without arguments.

## Example Scripts

### UDS

Following example scripts can be used to test UDS features:

- **UDS_DumpMemory.py** dumps RAMN's ECUs' readable areas of memory (RAM and FLASH)  into a single hex file, and various .bin files located in individual folders.
- **UDS_LoadTest.py** sends large ISO-TP frames back and forth between RAMN's ECUs to ensure that high speed communication is reliable.
- **UDS_PrintInfo.py** prints information of RAMN ECUs that can be obtained with ReadDataByIdentifier (VIN, Firmware Compile Time, Microcontroller Serial Number, Memory CRC, etc.).
- **UDS_Reprogram.py** reprograms RAMN's ECUs with the firmware files, which path is provided in the .ini fils.
- **UDS_Verify.py** verifies that the firmware files which path is provided in the .ini file, are correctly flashed in RAMN's ECUs.
- **UDS_WriteVIN.py** writes the VIN provided inside the python script in RAMN's ECUs.

### KWP2000

Following example scripts can be used to test KWP2000 features:

- **KWP_DisableNormalTransmission.py** disables periodic sending of CAN messages for RAMN ECUs.
- **KWP_EnableNormalTransmission.py** re-enables periodic sending of CAN messages for RAMN ECUs. Especially, it may prove useful if a previous script that turned off communications was interrupted before it was able to re-enable communications.
- **KWP_TesterPresent.py** sends and receives a Tester Present Frame.

### XCP

Following example scripts can be used to test XCP features:

- **XCP_DumpMemory.py** uses the SET_MTA and UPLOAD commands to dump readable areas of RAMN ECU's memory. Areas dumped are identical to those dumped by **UDS_DumpMemory.py**.
- **XCP_ReadECUInfo.py** uses XCP commands to readout XCP information from RAMN's ECUs.

### CAN

Following script is provided for convenience:

- **CAN_ResetRXFilter.py** resets the CAN filter of ECU A. Can be useful if a script that uses a specified filter was interrupted before it had to reset the filter itself.

## Other

*Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*