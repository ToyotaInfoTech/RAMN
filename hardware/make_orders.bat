@echo off

if not exist "build" (
    mkdir "build" || exit /b
)

rem makes build\ramn_and_pods.kicad_pcb
python scripts/pods_panel.py || exit /b

if not exist "orders\0_ramn" (
    mkdir "orders\0_ramn" || exit /b
)
kikit fab pcbway --no-drc V1_revB\0_ramn\ramn.kicad_pcb orders\0_ramn\ || exit /b
echo "*** Next, MANUALLY make centroid and BOM file for orders\0_ramn\ ***"

if not exist "orders\pods_panel" (
    mkdir "orders\pods_panel" || exit /b
)
kikit fab pcbway --no-drc build\pods_panel.kicad_pcb orders\pods_panel\ || exit /b
echo "*** Next MANUALLY make centroid and BOM file for orders\pods_panel\ ***"