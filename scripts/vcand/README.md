
# RAMN VCAN DAEMON

This folder contains experimental scripts to connect RAMN to a socketCAN virtual CAN Network, while keeping the original serial interface accessible.

This feature is only available on Linux. It is similar to [slcand](https://github.com/linux-can/can-utils/blob/master/slcand.c), but with additional features:
- Supports CAN-FD
- Can forward the original serial I/O to one or two virtual serial port(s) (e.g., */dev/pts/x*)


#  Usage


## CAN-FD Message forwarding

### 0_SetupVCAN.sh

If your OS does not already have a virtual CAN network, you can run **0_SetupVCAN.sh** to set one up (typically, **vcan0**). 
It should only be called once, and will likely required sudo.

### RAMN_VCAND.py

- Modify **settings/linux.ini** to specify the hardware serial port that should be forwarded (e.g., ```VCAND_HARDWARE_PORT = AUTODETECT```) and the virtual CAN network to forward the messages to (e.g.,  ```CAN_NAME = vcan0```)


After that, you should be able to run RAMN_VCAND.py:

```
$python3 RAMN_VCAND.py
```

CAN-FD Messages received by RAMN ECU A will be visible on vcan0.

## Serial Forwarding



### 1_SpawnTTY.sh
Run **1_SpawnTTY.sh** to spawn interconnected pseudo-terminals using socat. It should be called once per required forward virtual serial port. Typically, it will create two pseudo-terminals :
-  /dev/pts/1
-  /dev/pts/2

### RAMN_VCAND.py

- in **settings/linux.ini**, modify the variable *SERIAL_FORWARD_TARGET* to match the first pseudo-terminal spawned by the helper script (e.g., ```SERIAL_FORWARD_TARGET  = /dev/pts/1```).

- *(optional, if you still want to use CARLA and diagnostics scripts)* in **settings/linux.ini**, modify *PORT* to match the ***other*** pseudo-terminal (e.g., ```PORT  = /dev/pts/2```).


After that, you should be able to run RAMN_VCAND.py:

```
$python3 RAMN_VCAND.py
```
CAN-FD Messages will still be visible on the virtual CAN bus, and scripts should still be functional when /dev/pts/2 is provided in place of the original serial port (.e.g /dev/ttyACM0).

# Other

*Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.*