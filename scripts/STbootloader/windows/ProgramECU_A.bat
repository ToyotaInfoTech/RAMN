@echo off

SET RAMN_PORT=AUTO

SET STM32PROG_PATH="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
SET ECU_FIRMWARE_PATH=%~dp0\..\..\firmware\ECUA.hex

%STM32PROG_PATH% -c port=usb1 pid=0xdf11 vid=0x0483 -d "%ECU_FIRMWARE_PATH%" --verify --start 0x08000000

timeout /t 10 > NUL
