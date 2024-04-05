# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Utility Classes for accessing RAMN in python. 

import serial
import time
import sys
sys.path.append("..")
from utils.RAMN_Utils import *

class RAMN_Inputs(object):
    def __init__(self):
    
        #Controls
        self.brake_control = 0
        self.accel_control = 0
        self.sidebrake_control = 0
        self.steering_control = 0
        self.shift_control = 0
        self.reverse = False
        
        #Commands
        self.lights = 0
        self.horn_command = 0
        self.enginekey = 0
        
    def update_from_serial(self,hexval):
        STR_BRAKE = hexval[1:4]
        STR_ACCEL = hexval[4:7]
        STR_STEER = hexval[7:10]
        STR_SHIFT = hexval[10:12]
        STR_LIGHTS = hexval[12:14]
        STR_SIDEBRAKE = hexval[14:15]
        STR_HORN = hexval[15:16]
        STR_ENGINEKEY = hexval[16:17]
        
        self.brake_control = int(STR_BRAKE,16)/0xFFF
        self.accel_control = int(STR_ACCEL,16)/0xFFF

        self.steering_control = (int(STR_STEER,16) - 0x7FF)/0x800
        
        s = int(STR_SHIFT,16)
        
        if s == 0xFF:
            self.reverse = True
            self.shift_control = 1
        else:
            self.shift_control = s
            self.reverse = False
        
        self.lights = int(STR_LIGHTS,16)
        
        val = int(STR_SIDEBRAKE,16)
        if val != 0:
            self.sidebrake_control = True
        else:
            self.sidebrake_control = False
        
        self.horn_command = int(STR_HORN,16)
        self.STR_ENGINEKEY = int(STR_ENGINEKEY,16)    
        
    def update_from_CAN(self,msg):
        if len(msg.data) < 4:
            return
            
        if  msg.arbitration_id == 0x24:    #Brake
            self.brake_control = int.from_bytes(msg.data[0:2],"big")/0xFFF
            
        elif msg.arbitration_id == 0x39:    #Accel
            self.accel_control = int.from_bytes(msg.data[0:2],"big")/0xFFF
    
        elif msg.arbitration_id == 0x62:    #Steering
            self.steering_control = ((int.from_bytes(msg.data[0:2],"big")) - 0x7FF)/0x800
            
        elif msg.arbitration_id == 0x77:    #Shift
            s = int.from_bytes(msg.data[0:1],"big")  
  
            if s == 0xFF:
                self.reverse = True
                self.shift_control = 1
            else:
                self.shift_control = s
                self.reverse = False
                
        elif msg.arbitration_id == 0x1D3:   #Sidebrake
            
            val = int.from_bytes(msg.data[0:2],"big")
            if val != 0:
                self.sidebrake_control = True
            else:
                self.sidebrake_control = False
    
        

