@echo off

SET RAMN_PORT=AUTO

SET ECU_FIRMWARE_PATH=%~dp0\..\..\firmware

python "%~dp0\..\canboot.py" %RAMN_PORT% B -i "%ECU_FIRMWARE_PATH%\ECUB.hex" -e -p -v

python "%~dp0\..\canboot.py" %RAMN_PORT% C -i "%ECU_FIRMWARE_PATH%\ECUC.hex" -e -p -v

python "%~dp0\..\canboot.py" %RAMN_PORT% D -i "%ECU_FIRMWARE_PATH%\ECUD.hex" -e -p -v --reset
