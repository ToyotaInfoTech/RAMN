@echo off

SET RAMN_PORT=AUTO

rem You should use -wu if you used write protection

echo If you used the AUTOLOCK feature, you may get an error message even though the memory unlock was successful (check by verifying that the firmware was erased and that the ECU is not responsive anymore)

python ..\canboot.py %RAMN_PORT% B -ru

python ..\canboot.py %RAMN_PORT% C -ru

python ..\canboot.py %RAMN_PORT% D -ru --reset

pause

