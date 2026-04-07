#!/bin/bash
set -e

# Base includes and compiler flags
# We block almost all headers that we don't want to use from the firmware
MOCK_FLAGS="-D__RAMN_UTILS_H_ -DINC_RAMN_UTILS_H_ \
            -DINC_RAMN_MEMORY_H_ \
            -DINC_RAMN_EEPROM_H_ \
            -DINC_RAMN_CANFD_H_ \
            -DINC_RAMN_J1979_H_ \
            -DINC_RAMN_ISOTP_H_ \
            -DINC_RAMN_DBC_H_ \
            -DINC_RAMN_CRC_H_ \
            -DINC_RAMN_TRNG_H_ \
            -DINC_RAMN_SCREEN_MANAGER_H_ \
            -DINC_RAMN_SIMULATOR_H_ \
            -D__MAIN_H \
            -include scripts/tests/mocks/force_include.h"

INCLUDES="-I scripts/tests/mocks/ -I firmware/RAMNV1/Core/Inc/"
CFLAGS="-shared -fPIC -DCPYTHON_TESTING -DUDS_ACCEPT_FUNCTIONAL_ADDRESSING"

# Create dummy headers if they don't exist
touch scripts/tests/mocks/task.h
touch scripts/tests/mocks/semphr.h
touch scripts/tests/mocks/eeprom_emul.h
touch scripts/tests/mocks/eeprom_emul_conf.h

echo "Compiling Serdes Shared Libraries..."
gcc $CFLAGS -D__MAIN_H $INCLUDES -o scripts/tests/librbd_can_db.so firmware/RAMNV1/Core/Src/ramn_can_database.c
gcc $CFLAGS -DENABLE_J1939_MODE -D__MAIN_H $INCLUDES -o scripts/tests/librbd_can_db_j1939.so firmware/RAMNV1/Core/Src/ramn_can_database.c

echo "Compiling ECU-specific Diagnostic Shared Libraries..."

DIAG_SRCS="firmware/RAMNV1/Core/Src/ramn_uds.c \
           firmware/RAMNV1/Core/Src/ramn_kwp2000.c \
           firmware/RAMNV1/Core/Src/ramn_xcp.c \
           firmware/RAMNV1/Core/Src/ramn_isotp.c \
           firmware/RAMNV1/Core/Src/ramn_customize.c \
           firmware/RAMNV1/Core/Src/ramn_j1939.c \
           scripts/tests/mocks/ramn_diag_mocks.c"

for ecu in A B C D; do
    echo "  Building ECU $ecu (Standard)..."
    gcc $CFLAGS -DTARGET_ECU$ecu -DENABLE_UDS -DENABLE_KWP -DENABLE_XCP -DENABLE_ISOTP $MOCK_FLAGS $INCLUDES \
        -o scripts/tests/librbd_ecu${ecu}_std.so $DIAG_SRCS
    
    echo "  Building ECU $ecu (J1939)..."
    gcc $CFLAGS -DTARGET_ECU$ecu -DENABLE_J1939_MODE -DENABLE_UDS -DENABLE_KWP -DENABLE_XCP -DENABLE_ISOTP $MOCK_FLAGS $INCLUDES \
        -o scripts/tests/librbd_ecu${ecu}_j1939.so $DIAG_SRCS
done

echo "Build successful."
