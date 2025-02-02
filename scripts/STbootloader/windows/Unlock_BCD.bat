@echo off

SET RAMN_PORT=AUTO

python ..\canboot.py %RAMN_PORT% B -wu -ru

python ..\canboot.py %RAMN_PORT% C -wu -ru

python ..\canboot.py %RAMN_PORT% D -wu -ru --reset

