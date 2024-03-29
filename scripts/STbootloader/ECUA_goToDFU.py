#!/usr/bin/env python

# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#this script sends serial command to ECU A to request a transition to DFU mode

import serial
import click
import platform 
import time
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
import utils.RAMN_Utils    
    
@click.command()
@click.argument('serial_port')
def ECUA_goToDFU(serial_port):
    click.echo("Putting ECU A into DFU Mode")
    
    if serial_port == "AUTO":
        serial_port = utils.RAMN_Utils.autoDetectRAMNPort()
    
    try:
        ser = serial.Serial(serial_port,timeout=10)
        click.echo("Opening port {}".format(serial_port)) 
        ser.write(b'DzZ\r')
        ser.flush()
        time.sleep(1)
        while ser.in_waiting != 0:
            r = ser.read(ser.in_waiting)  
        click.echo("Done")
    except Exception as e:
        click.echo("No COM Port (device probably in DFU mode)" + str(e))
        pass

if __name__ == '__main__':
    ECUA_goToDFU()
   