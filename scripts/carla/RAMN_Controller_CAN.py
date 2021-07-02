# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Classes for accessing RAMN in python. Use a CAN interface (e.g. can0).

from RAMN_Controller_Utils import *
import can
import random

class RAMN_Controller_CAN(object):

    def __init__(self):
        self.bus = can.interface.Bus(RAMN_Utils.CAN_NAME, bustype=RAMN_Utils.CAN_TYPE)
        self.inputs = RAMN_Inputs()
        self.lastSent = 0
          
    def update(self):
        msg = self.bus.recv(timeout=0)
        while msg != None:
            self.inputs.update_from_CAN(msg)
            msg = self.bus.recv(timeout=0)

    def update_output(self,brake,accel,hand_brake,steer,shift,reverse,scal_vel, horn):
        currentT = time.perf_counter()     
            
        #Send feedback for all messages at once
        if currentT - self.lastSent > RAMN_Utils.CAN_REFRESH_RATE/1000:
            self.lastSent = time.perf_counter()
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
            
            canid = 0x1A
            r = random.randint(0,0xFFFFFFFF)
            data = [brake_command&0xFF,(brake_command>>8)&0xFF,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
            msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
            self.bus.send(msg)
            
            canid = 0x2F
            r = random.randint(0,0xFFFFFFFF)
            data = [accel_command&0xFF,(accel_command>>8)&0xFF, 0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
            msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
            self.bus.send(msg)
            
            canid = 0x43
            r = random.randint(0,0xFFFFFFFF)
            data = [rpm_state&0xFF,(rpm_state>>8)&0xFF, 0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
            msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
            self.bus.send(msg)
            
            canid = 0x58
            r = random.randint(0,0xFFFFFFFF)
            data = [steering_command&0xFF,(steering_command>>8)&0xFF,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
            msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
            self.bus.send(msg)
            
            canid = 0x6D
            r = random.randint(0,0xFFFFFFFF)
            data = [shift_command&0xFF,0, 0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
            msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
            self.bus.send(msg)
            
            canid = 0x1C9
            r = random.randint(0,0xFFFFFFFF)
            data = [0,sidebrake_command&0xFF,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
            msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
            self.bus.send(msg)

    def close(self):
        self.bus.shutdown()
