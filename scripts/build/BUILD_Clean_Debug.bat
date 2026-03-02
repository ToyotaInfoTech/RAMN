@echo off

call "%~dp0_version.bat"
SET /p STM32CUBEIDEWORKSPACE=<"%~dp0..\settings\FIRMWARE_WORKSPACE.txt"
SET "OUTPUT_FOLDER=%~dp0..\firmware"
SET "PROJECT_NAME=RAMNV1"

if not exist "%OUTPUT_FOLDER%" mkdir "%OUTPUT_FOLDER%"

"%STM32CUBEIDEPATH%\stm32cubeidec.exe" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild "%PROJECT_NAME%/Debug" -import "%STM32CUBEIDEWORKSPACE%/%PROJECT_NAME%" -data "build_workspace" -D TARGET_ECUA -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
echo f | xcopy /f /y "%STM32CUBEIDEWORKSPACE%\%PROJECT_NAME%\Debug\%PROJECT_NAME%.hex" "%OUTPUT_FOLDER%\ECUA.hex"
echo f | xcopy /f /y "%STM32CUBEIDEWORKSPACE%\%PROJECT_NAME%\Debug\%PROJECT_NAME%.bin" "%OUTPUT_FOLDER%\ECUA.bin"

"%STM32CUBEIDEPATH%\stm32cubeidec.exe" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild "%PROJECT_NAME%/Debug" -import "%STM32CUBEIDEWORKSPACE%/%PROJECT_NAME%" -data "build_workspace" -D TARGET_ECUB -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
echo f | xcopy /f /y "%STM32CUBEIDEWORKSPACE%\%PROJECT_NAME%\Debug\%PROJECT_NAME%.hex" "%OUTPUT_FOLDER%\ECUB.hex"

"%STM32CUBEIDEPATH%\stm32cubeidec.exe" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild "%PROJECT_NAME%/Debug" -import "%STM32CUBEIDEWORKSPACE%/%PROJECT_NAME%" -data "build_workspace" -D TARGET_ECUB -D CHASSIS_LINEAR_POTENTIOMETER -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
echo f | xcopy /f /y "%STM32CUBEIDEWORKSPACE%\%PROJECT_NAME%\Debug\%PROJECT_NAME%.hex" "%OUTPUT_FOLDER%\ECUB_LINEAR.hex"

"%STM32CUBEIDEPATH%\stm32cubeidec.exe" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild "%PROJECT_NAME%/Debug" -import "%STM32CUBEIDEWORKSPACE%/%PROJECT_NAME%" -data "build_workspace" -D TARGET_ECUC -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
echo f | xcopy /f /y "%STM32CUBEIDEWORKSPACE%\%PROJECT_NAME%\Debug\%PROJECT_NAME%.hex" "%OUTPUT_FOLDER%\ECUC.hex"

"%STM32CUBEIDEPATH%\stm32cubeidec.exe" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild "%PROJECT_NAME%/Debug" -import "%STM32CUBEIDEWORKSPACE%/%PROJECT_NAME%" -data "build_workspace" -D TARGET_ECUD -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
echo f | xcopy /f /y "%STM32CUBEIDEWORKSPACE%\%PROJECT_NAME%\Debug\%PROJECT_NAME%.hex" "%OUTPUT_FOLDER%\ECUD.hex"
