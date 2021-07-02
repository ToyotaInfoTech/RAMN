@echo off

SET /p CARLAPATH=<../settings/CARLA_PATH.txt
start %CARLAPATH%\CarlaUE4.exe -windowed -resX=600 -resY=600 -quality-level=Low -carla-settings="CarlaSettings.ini"
