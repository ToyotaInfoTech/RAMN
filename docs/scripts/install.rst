Installing RAMN's scripts
=========================

.. _install_ramn_scripts:

Installing RAMN's scripts
-------------------------

Follow these steps to install RAMN's scripts:

- Download the content of `RAMN GitHub repository <https://github.com/ToyotaInfoTech/RAMN>`_. You can unzip the file that you can download by clicking *Code -> Download Zip*, or you can use the following command on Linux:

    .. code-block:: bash

                $ git clone https://github.com/ToyotaInfoTech/RAMN


- Open a command prompt in the *RAMN/script* folder (on Windows, open the folder in Windows Explorer and type "cmd" in the address bar) and execute the following command:

    .. code-block:: bash

            $ pip3 install -r requirements.txt


.. _configure_ramn_scripts:

Configuring RAMN's scripts
--------------------------

The default settings should work on any computer.

If you want, you can modify the following settings in *scripts/settings/windows.ini* (when using Windows) and *scripts/settings/linux.ini* (when using Linux):

- **PORT** specifies the serial port name used by scripts (e.g., COM1 or /dev/ttyACM0). You can use "AUTODETECT" to let the scripts automatically find a RAMN port.
- **ECUx_FIRMWARE_PATH** specifies the firmware files used to reprogram/verify the firmware of ECUs.
- **VERBOSE** specifies the verbose level of scripts (0 to only output errors, 4 to output everything including debug messages).
- **TIMEOUT** specifies the timeout value for reading from the serial port.
- **CAN_NAME** specifies the name of the **channel** for scripts that rely on CAN rather than serial.
- **CAN_TYPE** specifies the name of the **interface type** for the above.
- **CAN_REFRESH_RATE** specifies how often scripts should update the CAN bus (for scripts that use CAN).
- **VCAND_HARDWARE_PORT** is the default hardware port used by the :ref:`vcand` scripts.



