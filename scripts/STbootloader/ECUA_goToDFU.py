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
    
@click.command()
@click.argument('serial_port')
def ECUA_goToDFU(serial_port):
    click.echo("Putting ECU A into DFU Mode")
    try:
        click.echo("Opening port {}".format(serial_port)) 
        ser = serial.Serial(serial_port,timeout=10)
        ser.write(b'DzZ\r')
        ser.flush()
        time.sleep(2)
        while ser.in_waiting != 0:
            r = ser.read(ser.in_waiting)
        time.sleep(2)    
        click.echo("Done")
    except:
        pass

   