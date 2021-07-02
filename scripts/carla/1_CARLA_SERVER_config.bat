@echo off

SET /p CARLAPATH=<../settings/CARLA_PATH.txt

python %CARLAPATH%\PythonAPI\util\config.py -h
python %CARLAPATH%\PythonAPI\util\config.py --list
python %CARLAPATH%\PythonAPI\util\config.py -m Town01


timeout 500
