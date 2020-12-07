# Copyright (c) 2020 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Classes for accessing RAMN in python 

import serial
import time
import random

class RAMN_Inputs(object):
    def __init__(self):
        #Controls
        self.brake_control = 0
        self.accel_control = 0
        self.parkingbrake_control = 0
        self.steering_control = 0
        self.shift_control = 0
        self.reverse = False
        
        #Commands
        self.horn_command = 0
        
    def update(self,hexval):
        self.brake_control = int(hexval[1:5],16)/0xFFF
        self.accel_control = int(hexval[5:9],16)/0xFFF
        self.steering_control = (0x7FF - int(hexval[9:13],16))/(0x800)
        
        s = int(hexval[13:15],16)
        val =  s - 0x100 if  s > 127 else  s
        
        self.shift_control = abs(val)
        self.reverse = (val < 0)
    
        self.horn_command = int(hexval[15:17],16)
        
        val = int(hexval[17:19],16)
        if val != 0:
            self.parkingbrake_control = True
        else:
            self.parkingbrake_control = False
   
class RAMN_Outputs(object):
    def __init__(self):
        #Commands
        self.brake_command = 0
        self.accel_command = 0
        self.parkingbrake_command = 0 
        self.steering_command = 0
        self.shift_command = 0
        
        #Controls
        self.horn_state = 0
        self.rpm_state = 0

class RAMN_Controller(object):

    def __init__(self,port="COM3"):
        self.ser = serial.Serial(port, timeout=0)
        self.ser.write(b'c\r')
        self.ser.read(1) #read the ack
        self.inputs = RAMN_Inputs()
        self.outputs = RAMN_Outputs()
        self.lastSent = 0
        
    def update(self):
        line = None
        while self.ser.in_waiting != 0:  
            line = self.ser.read(20).decode()
            if line != None and line[0] == "u":
                continue
            else:
                #got an error, flush the port
                print("got corrupted serial frame")
                self.ser.reset_input_buffer()
                
        if line != None and len(line) > 1 and line[0] == "u":
            self.inputs.update(line)

    def update_output(self,brake,accel,hand_brake,steer,shift,reverse,scal_vel, horn):
        self.outputs.brake_command = round(brake*0xFFF)&0xFFF
        self.outputs.accel_command = round(accel*0xFFF)&0xFFF
        if hand_brake:
            self.outputs.parkingbrake_command = 0x01
        else:
            self.outputs.parkingbrake_command = 0x00
        self.outputs.steering_command = (0x7FF - round(steer*0x7FF))&0x0FFF
        
        if reverse:
            val = (0x100-shift)&0xFF
        else:
            val = shift&0x7F
            
        self.outputs.shift_command = val
        self.outputs.rpm_state = round((100 * scal_vel))
        self.outputs.horn_state = horn
        currentT = time.perf_counter()
        
        self.ser.write(('u{:04x}{:04x}{:04x}{:04x}{:02x}{:02x}{:02x}\r'.format(self.outputs.brake_command,self.outputs.accel_command,self.outputs.rpm_state,self.outputs.steering_command,self.outputs.shift_command,self.outputs.horn_state ,self.outputs.parkingbrake_command)).encode())
       
    def send_update(self):
        canid = 0x1A
        r = random.randint(0,0xFFFFFFFF)
        data = [(self.outputs.brake_command>>8)&0xFF, self.outputs.brake_command&0xFF,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
        msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
        self.sendbus.send(msg)
        
        canid = 0x2F
        r = random.randint(0,0xFFFFFFFF)
        data = [(self.outputs.accel_command>>8)&0xFF, self.outputs.accel_command&0xFF,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
        msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
        self.sendbus.send(msg)
        
        canid = 0x43
        r = random.randint(0,0xFFFFFFFF)
        data = [(self.outputs.rpm_state>>8)&0xFF, self.outputs.rpm_state&0xFF,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
        msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
        self.sendbus.send(msg)
        
        canid = 0x58
        r = random.randint(0,0xFFFFFFFF)
        data = [(self.outputs.steering_command>>8)&0xFF, self.outputs.steering_command&0xFF,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
        msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
        self.sendbus.send(msg)
        
        canid = 0x6D
        r = random.randint(0,0xFFFFFFFF)
        data = [self.outputs.shift_command&0xFF, 0,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
        msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
        self.sendbus.send(msg)
        
        canid = 0xA2
        r = random.randint(0,0xFFFFFFFF)
        data = [self.outputs.horn_state&0xFF, 0,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
        msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
        self.sendbus.send(msg)
        
        canid = 0x1C9
        r = random.randint(0,0xFFFFFFFF)
        data = [self.outputs.parkingbrake_command&0xFF,0,0,0] + [(r  >> i & 0xff) for i in (24,16,8,0)]
        msg = can.Message(arbitration_id=canid, data=data,is_extended_id=False)
        self.sendbus.send(msg)

    def close(self):
        self.ser.close()
        
if __name__ == "__main__":
    print("Test")
    r = RAMN_Controller()

    while True:
        r.update()
        print(r.inputs.brake_control)  

    r.close()
    print("Done")
    