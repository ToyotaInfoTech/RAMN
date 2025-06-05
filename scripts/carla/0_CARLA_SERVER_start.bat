@echo off

SET /p CARLAPATH=<../settings/CARLA_PATH.txt

IF EXIST "%CARLAPATH%\CarlaUnreal.exe" (
    SET EXE=CarlaUnreal.exe
) ELSE IF EXIST "%CARLAPATH%\CarlaUE4.exe" (
    SET EXE=CarlaUE4.exe
) ELSE (
    ECHO Error: Neither CarlaUnreal.exe nor CarlaE4.exe found in %CARLAPATH%
    PAUSE
    EXIT /B 1
)

start "" "%CARLAPATH%\%EXE%" -windowed -resX=600 -resY=600 -quality-level=Low -carla-settings="CarlaSettings.ini"
