@echo off

SET RAMN_PORT=COM3

SET ECU_FIRMWARE_PATH=..\..\firmware

python canboot.py %RAMN_PORT% B -i %ECU_FIRMWARE_PATH%\ECUB.hex -e -p -v -ru -wu

python canboot.py %RAMN_PORT% C -i %ECU_FIRMWARE_PATH%\ECUC.hex -e -p -v -ru -wu

python canboot.py %RAMN_PORT% D -i %ECU_FIRMWARE_PATH%\ECUD.hex -e -p -v -ru -wu --reset