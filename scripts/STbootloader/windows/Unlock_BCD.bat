@echo off

SET RAMN_PORT=AUTO

rem You should use -wu if you used write protection

python ..\canboot.py %RAMN_PORT% B -ru

python ..\canboot.py %RAMN_PORT% C -ru

python ..\canboot.py %RAMN_PORT% D -ru --reset

