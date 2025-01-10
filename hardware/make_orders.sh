#!/bin/bash

if [ ! -d "build" ]; then
    mkdir "build" || exit 1
fi

python scripts/pods_panel.py || exit 1

if [ ! -d "orders/0_ramn" ]; then
    mkdir -p "orders/0_ramn" || exit 1
fi
kikit fab pcbway --no-drc V1_revB/0_ramn/ramn.kicad_pcb orders/0_ramn/ || exit 1
echo "*** Next, MANUALLY make centroid and BOM file for orders/0_ramn/ ***"

if [ ! -d "orders/pods_panel" ]; then
    mkdir -p "orders/pods_panel" || exit 1
fi
kikit fab pcbway --no-drc build/pods_panel.kicad_pcb orders/pods_panel/ || exit 1
echo "*** Next MANUALLY make centroid and BOM file for orders/pods_panel/ ***"