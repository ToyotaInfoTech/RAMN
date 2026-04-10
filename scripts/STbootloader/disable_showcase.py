#!/usr/bin/env python

# Copyright (c) 2025 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Disable Showcase Mode on ECU A before flashing to prevent bus flooding.
# Sends a serial close command and a UDS RoutineControl stop (0x31 0x02 0x02 0x07).
# Safe no-op if RAMN is not connected or Showcase Mode is not active.

import serial
import serial.tools.list_ports
import time
import sys

def find_ramn_port(port_hint="AUTO"):
    if port_hint != "AUTO":
        return port_hint
    for p in serial.tools.list_ports.comports():
        if (p.vid == 0x483 and p.pid == 0x5740) or (p.vid == 0x1d50 and p.pid == 0x606f):
            return p.device
    return None

def disable_showcase(port_hint="AUTO"):
    port = find_ramn_port(port_hint)
    if port is None:
        return
    try:
        ser = serial.Serial(port, timeout=1)
        # Stop any active simulator/CARLA serial connection
        ser.write(b'c0\r')
        time.sleep(0.2)
        if ser.in_waiting:
            ser.read(ser.in_waiting)
        # Send UDS RoutineControl StopRoutine for autopilot routine 0x0207 to ECU A
        # slcan format: t + CAN_ID(7e0) + DLC(5) + ISO-TP SF(04) + UDS(31 02 02 07) + \r
        ser.write(b't7e050431020207\r')
        time.sleep(0.2)
        if ser.in_waiting:
            ser.read(ser.in_waiting)
        ser.close()
    except Exception:
        pass

if __name__ == "__main__":
    port_hint = sys.argv[1] if len(sys.argv) > 1 else "AUTO"
    disable_showcase(port_hint)
