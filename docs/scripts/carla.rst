.. _carla_tutorial:

CARLA
=====

`CARLA <https://carla.org/>`_ is an autonomous driving simulator based on Unreal Engine 4.
This page explains how to install CARLA and use it with RAMN.
Although we only tested the scripts on Windows, they should be usable on Linux with minor adjustments.
Note that the simulator requires that you have a sufficiently powerful GPU.

.. image:: ../gif/carla.gif
   :align: center


.. _install_carla:

Installing CARLA
----------------

Follow these steps to install CARLA on Windows:

- Download and unzip the latest release of CARLA, from their `Github repository <https://github.com/carla-simulator/carla/releases>`_. The latest version that we tested is Release 0.9.15.
- Check that the simulator works by launching *WindowsNoEditor/CarlaUE4.exe*. This will likely require that you approve the app with Microsoft's SmartScreen. If a 3D view opens, you can close the simulator and move on to the next step. If you get an error message, install the `latest version of Windows DirectX <https://www.microsoft.com/en-us/download/details.aspx?id=35>`_.
- Install the latest Python **3.8 version (not a later version such as 3.12)** from Python Software foundation's `webpage <https://www.python.org/downloads/windows/>`_. **Check the "add python.exe to PATH" option**. The latest version that we tested is `3.8.10 <https://www.python.org/downloads/release/python-3810/>`_. If you do not know which file you should use, try "Windows installer (64-bit)".
- Install CARLA's pythonAPI by opening a command prompt (press Windows+R and type "cmd") and executing:

    .. code-block:: bash

            $ pip3 install carla numpy pygame

    If you also want to execute self-driving examples, execute the following command:

    .. code-block:: bash

            $ pip3 install shapely networkx

- Try CARLA's *manual_control.py* and *automatic_control.py* examples. First, execute *WindowsNoEditor/CarlaUE4.exe* and leave the window open (that is your CARLA server). Then, open a command prompt in *WindowsNoEditor/PythonAPI/examples* (open the folder in Windows Explorer and type "cmd" in the address bar) and try the first example:

        .. code-block:: bash

            $ python manual_control.py

    You should be able to control the car manually with the WASD keys of your keyboard.
    Then, try the second example:

    .. code-block:: bash

            $ python automatic_control.py

    You should be able to see the self-driving algorithm in action.

If you have issues with the latest CARLA release, try executing it with the following options:

.. code-block:: bash

    $ CarlaUE4.exe -dx11 -windowed -quality-level=Low

If this does not resolve your issues, try downloading an earlier version of CARLA. RAMN scripts should be compatible with release 0.9.12.
If you do so, you will also need to install CARLA's API with the corresponding version, e.g.:

.. code-block:: bash

    $ pip3 install carla==0.9.12

From there, you should have a functional environment to experiment with CARLA.
If you encounter problems up to this point, there is a problem with your CARLA installation, not RAMN.
Check CARLA's `Quickstart guide <https://carla.readthedocs.io/en/latest/start_quickstart/>`_ for support.

If you want, you can download additional assets (maps, etc.) by following the `instructions here <https://carla.readthedocs.io/en/latest/start_quickstart/#import-additional-assets>`_.
You can also edit CARLA's default settings (weather, etc.) by following `this page <https://carla.readthedocs.io/en/stable/configuring_the_simulation/>`_.

If you have not already, you also need to install RAMN's scripts (see :ref:`flashing_scripts`).

.. _ramn_carla_scripts:

Configuring your environment
----------------------------

- Open *RAMN/script/settings/CARLA_PATH.txt* and replace its content with the path to the folder where *CARLAUE4.exe* is located.
- (Optional) Modify *0_CARLA_SERVER_start.bat* with your preferred settings (resolution, etc.). Read `CARLA's documentation <https://carla.readthedocs.io/en/latest/adv_rendering_options/>`_ for more information about CARLA's options. Specify the quality of graphics using :code:`-quality-level=Epic` (best graphics) or :code:`-quality-level=Low` (best performances). Specify the resolution of the server using :code:`-windowed -ResX=N -ResY=N`.

- (Optional) Modify *CarlaSettings.ini* to edit default settings (weather, etc.)
- (Optional) Modify *1_CARLA_SERVER_config.bat* to provide a shortcut to execute CARLA's *config.py* as you want (e.g., to `load another map <https://carla.readthedocs.io/en/0.9.15/tuto_first_steps/#loading-a-map>`_).

Executing RAMN's scripts for CARLA
----------------------------------

First, start a CARLA server:

- Execute *0_CARLA_SERVER_start.bat*.
- (Optional) Execute *1_CARLA_SERVER_config.bat* to update the server's configuration.

You only need to start one server per session. You will be able to execute the following scripts as long as the window stays active.
You can for example execute the basic RAMN examples:

- Execute *2_CARLA_RAMN_manual_serial.bat* to connect RAMN to CARLA and drive the vehicle manually using RAMN's controls.
- Execute *3_CARLA_RAMN_auto_serial.bat* to connect RAMN to CARLA's self-driving algorithm.

If you get an error, verify that your RAMN's serial port is not being used by another application.

When you use the self-driving algorithm, the controls will be decided by RAMN.
If the physical controls are at their neutral position (e.g., bottom position for brake and accelerator potentiometers), the instructions from CARLA's self-driving algorithm will be applied. Otherwise, the instructions from the physical controls will be applied.

When using the self-driving algorithm, if the steering wheel is not centered, the "Check Engine" LED will light up to let you know that RAMN is currently ignoring CARLA's instructions for the steering wheel and is applying the analog controls instead.
If the LED does not turn off when you center the steering wheel, it may be because you need to reflash the board with a different firmware (see :ref:`flashing`).

You can also connect CARLA to RAMN using a CAN adapter (internal or external) instead of the USB serial connection. To do this:

- Modify the scripts' settings to specify your CAN interface (see :ref:`configure_ramn_scripts`)
- Execute *4_CARLA_RAMN_manual_CAN.bat* to connect RAMN to CARLA and drive the vehicle manually.
- Execute *5_CARLA_RAMN_auto_CAN.bat* to connect RAMN to CARLA with the self-driving algorithm.

The CAN scripts have less features and typically worse performances than the serial one, so avoid using them unless you have a specific use case for them.

.. warning::
    When using the CARLA scripts, the serial port of RAMN will not be available for other applications. If you want to interact with the CAN bus, it is recommended that you connect an external CAN adapter. On Linux, you can use the :ref:`vcand` script to multiplex the serial port and observe the CAN bus even when the CARLA scripts are in use.

How the self-driving algorithm works
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

RAMN's CAN messages are classified as either "command" or "control":

- "Command" CAN messages correspond to "control requests" or "targets" from a self-driving algorithm.
- "Control" CAN messages correspond to "actually applied controls" by ECUs.

If the self-driving algorithm is OFF, many command messages won't be seen, and you will therefore observe fewer CAN messages.

By default, ECUs listen to command CAN messages, and immediately apply them as control CAN messages, except if the associated controls are not in their neutral position (brake and accelerator at 0%, steering wheel at its middle position).
For example, if a command CAN messages says "Brake 50%" and the physical potentiometer is at 0%, then the control CAN message will say "Brake 50%".
However, if a command CAN message says "Brake 0%" but the physical potentiometer says "Brake 100%", then the control CAN message will say "Brake 100%".

ECUs can also be reprogrammed to implement a control loop, such as a proportional–integral–derivative controller (PID).
RAMN's GitHub repository features `an example of PID control <https://github.com/ToyotaInfoTech/RAMN/blob/main/misc/PID_example.pdf>`_.
