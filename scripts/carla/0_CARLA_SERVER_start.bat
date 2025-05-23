@echo off

SET /p CARLAPATH=<../settings/CARLA_PATH.txt
start %CARLAPATH%\CarlaUnreal.exe -windowed -resX=600 -resY=600 -quality-level=Low -carla-settings="CarlaSettings.ini"
