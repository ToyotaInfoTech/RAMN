#!/bin/bash
set -e

echo "Compiling Default Shared Library..."
gcc -shared -fPIC -DCPYTHON_TESTING -D__MAIN_H -I scripts/tests/mocks/ -I firmware/RAMNV1/Core/Inc/ -o scripts/tests/librbd_can_db.so firmware/RAMNV1/Core/Src/ramn_can_database.c

echo "Compiling J1939 Shared Library..."
gcc -shared -fPIC -DCPYTHON_TESTING -DENABLE_J1939_MODE -D__MAIN_H -I scripts/tests/mocks/ -I firmware/RAMNV1/Core/Inc/ -o scripts/tests/librbd_can_db_j1939.so firmware/RAMNV1/Core/Src/ramn_can_database.c

echo "Build successful."
