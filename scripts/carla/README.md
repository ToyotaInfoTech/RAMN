
# Connection to CARLA

This folder contains scripts to connect RAMN in closed-loop with autonomous driving simulator [CARLA](https://github.com/carla-simulator/carla). Scripts in this folder are derived from those in [CARLA PythonAPI/examples](https://github.com/carla-simulator/carla/tree/master/PythonAPI/examples/) folder. 

<img src="https://github.com/ToyotaInfoTech/RAMN/blob/main/docs/gif/carla.gif?raw=true" width="1000">

## Usage

Modify files in the **settings** folder to match your environment. 

- **CARLA_PATH.txt** should contain the path to where CARLA is installed. 
- Depending on whether you are running on Windows or Linux, edit either **windows.ini** or **linux.ini**. Most importantly, the *PORT* variable should be set to the serial port attributed by your OS to RAMN (e.g., COM1 or /dev/ttyACM0). You may use the keyword *AUTODETECT* to let the script guess RAMN's port based on the VID and PID of currently connected USB devices.


## CARLA Server Start

On Windows, you may launch a CARLA server either by running **0_CARLA_SERVER_start.bat**, or by running the following command in a command prompt :
```
$CarlaUE4.exe -windowed -resX=600 -resY=600 -quality-level=Low -carla-settings="CarlaSettings.ini"
```
You may adjust the resolution (*-resX* and *-resY* arguments) as well as the quality (from *Low* to *Epic*) based on the performances of your machine.
On recent versions, CarlaUE4.exe might be called CarlaUnreal.exe.

## CARLA Server configuration

*(Optional)* You may change various settings (e.g. map, number of cars, etc.) with the **1_CARLA_SERVER_config.bat** script. Modify the script to match your settings. Refer to [CARLA's config.py](https://github.com/carla-simulator/carla/blob/master/PythonAPI/util/config.py) for details.

## Manual Control


### Serial Mode

The following command will launch the simulator with manual controls using the serial interface specified in the settings folder (also available by executing **2_CARLA_RAMN_manual_serial.bat**).

```
$python RAMN_CARLA_Manual.py 
```

### CAN Mode

The following command will launch the simulator using the CAN interface specified in the settings folder (also available by executing **4_CARLA_RAMN_manual_CAN.bat**). 
```
$python RAMN_CARLA_Manual.py --CAN
```
Make sure the settings variables *CAN_NAME* and *CAN_TYPE* match your environment, e.g., 'CAN_NAME = slcan0' and 'CAN_TYPE = socketcan' if you are using slcand to turn RAMN into a Linux CAN interface with the following command: 

```
$sudo slcand -o -c /dev/ttyACM0 slcan0 && sudo ip link set up slcan0
```

When using the CAN interface, it is possible to run the simulator without monopolizing the serial interface. This means that it is possible to observe the CAN bus using RAMN's slcan interface while the simulator is running. However, controls are usually slower than in serial mode. You may adjust the  *CAN_REFRESH_RATE* with a value that suits your machine (values too low will result in the CAN adapter "not being able to catch up", while values too high will result in rough controls).


## Automatic Control

### Serial Mode

The following command will launch the simulator with automatic controls using the serial interface specified in the settings folder (also available by executing **3_CARLA_RAMN_auto_serial.bat**).

```
python RAMN_CARLA_Automatic.py -l 
```

Use the option -l to run the simulator in loop mode (by default, program will exit when target point is reached).

### CAN Mode

The following command will launch the same simulator as above, but using the CAN interface specified in the settings folder (also available by executing **5_CARLA_RAMN_auto_CAN.bat**). Refer to the Manual Control section for more details about CAN mode.
```
$python RAMN_CARLA_Automatic.py --CAN -l
```
Use the option -l to run the simulator in loop mode (by default, program will exit when target point is reached).

## References

Please check the following paper for more information about CARLA.   

_CARLA: An Open Urban Driving Simulator_<br>Alexey Dosovitskiy, German Ros,
Felipe Codevilla, Antonio Lopez, Vladlen Koltun; PMLR 78:1-16
[[PDF](http://proceedings.mlr.press/v78/dosovitskiy17a/dosovitskiy17a.pdf)]
[[talk](https://www.youtube.com/watch?v=xfyK03MEZ9Q&feature=youtu.be&t=2h44m30s)]

## Other

*Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*
