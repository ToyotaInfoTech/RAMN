#!/bin/bash

RAMN_PORT=AUTO

ECU_FIRMWARE_PATH=../../firmware

# Disable autopilot/fuzzer on ECU A before flashing to prevent bus flooding.
# Safe no-op if RAMN is not connected or autopilot is not active.
python3 ../disable_autopilot.py $RAMN_PORT 2>/dev/null || true

python3 ../canboot.py $RAMN_PORT B -i $ECU_FIRMWARE_PATH/ECUB.hex -e -p -v

python3 ../canboot.py $RAMN_PORT C -i $ECU_FIRMWARE_PATH/ECUC.hex -e -p -v

python3 ../canboot.py $RAMN_PORT D -i $ECU_FIRMWARE_PATH/ECUD.hex -e -p -v --reset
