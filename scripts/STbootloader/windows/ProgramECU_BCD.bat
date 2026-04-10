@echo off

SET RAMN_PORT=AUTO

SET ECU_FIRMWARE_PATH=%~dp0\..\..\firmware

REM Disable Showcase Mode on ECU A before flashing to prevent bus flooding.
REM Safe no-op if RAMN is not connected or Showcase Mode is not active.
python "%~dp0\..\disable_showcase.py" %RAMN_PORT% 2>NUL

python "%~dp0\..\canboot.py" %RAMN_PORT% B -i "%ECU_FIRMWARE_PATH%\ECUB.hex" -e -p -v

python "%~dp0\..\canboot.py" %RAMN_PORT% C -i "%ECU_FIRMWARE_PATH%\ECUC.hex" -e -p -v

python "%~dp0\..\canboot.py" %RAMN_PORT% D -i "%ECU_FIRMWARE_PATH%\ECUD.hex" -e -p -v --reset

pause