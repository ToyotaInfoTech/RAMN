# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Classes for accessing RAMN in python. Uses a serial port (e.g. COM or /dev/ttyACM)

import serial

from RAMN_Controller_Utils import *

class RAMN_Controller_Serial(object):

    def __init__(self):
        self.ser = serial.Serial(RAMN_Utils.RAMN_DEFAULT_PORT, timeout=0)
        self.ser.write(b'c1\r')
        self.inputs = RAMN_Inputs()
        self.lastSent = time.perf_counter()
        
    def update(self):
        line = None
        while self.ser.in_waiting >= 18:  
            line = self.ser.read(18)
            if line[0] == ord('u'): #regular line
                continue
            else:
                #got an error, flush the port
                print("got corrupted serial frame:")
                #Read data until next \r
                while True:
                    c = self.ser.read(1)
                    line += c
                    if c == b'\r': 
                        print(str(line))
                        break
                
        if line != None and len(line) > 1 and line[0] == ord('u'):
            self.inputs.update_from_serial(line.decode())

    def update_output(self,brake,accel,hand_brake,steer,shift,reverse,scal_vel, horn):
        brake_command = round(brake*0xFFF)&0xFFF
        accel_command = round(accel*0xFFF)&0xFFF
        if hand_brake:
            sidebrake_command = 0x01
        else:
            sidebrake_command = 0x00
        steering_command = (0x7FF + round(steer*0x7FF))&0x0FFF
        
        if reverse:
            val = 0xFF
        else:
            val = shift&0x7F 
            
        shift_command = val
        rpm_state = round((100 * scal_vel))
        horn_state = horn
        
        currentT = time.perf_counter()     
        #Send feedback for all messages at once
        if currentT - self.lastSent > 0.010: #Send every 10 ms
            self.ser.write(('u{:03x}{:03x}{:03x}{:03x}{:02x}{:02x}{:02x}\r'.format(brake_command, accel_command, rpm_state, steering_command, shift_command, horn_state , sidebrake_command)).encode())
            self.lastSent = time.perf_counter()

    def close(self):
        self.ser.close()
        
