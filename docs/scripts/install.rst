.. _configure_ramn_scripts:

Configuring RAMN's scripts
==========================

Follow the steps in :ref:`flashing_scripts` to install RAMN's scripts.

You can modify the following settings in ``scripts/settings/windows.ini`` (when using Windows) and ``scripts/settings/linux.ini`` (when using Linux):

- **PORT** specifies the serial port name used by scripts (e.g., COM1 or /dev/ttyACM0). You can use "AUTODETECT" to let the scripts automatically find a RAMN port.
- **ECUx_FIRMWARE_PATH** specifies the firmware files used to reprogram/verify the firmware of ECUs.
- **VERBOSE** specifies the verbose level of scripts (0 to only output errors, 4 to output everything including debug messages).
- **TIMEOUT** specifies the timeout value for reading from the serial port.
- **CAN_NAME** specifies the name of the **channel** for scripts that rely on CAN rather than serial.
- **CAN_TYPE** specifies the name of the **interface type** for the above.
- **CAN_REFRESH_RATE** specifies how often scripts should update the CAN bus (for scripts that use CAN).
- **VCAND_HARDWARE_PORT** is the default hardware port used by the :ref:`vcand` scripts.



