#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This module handles USB Communication with RAMN's ECU A.

import serial
from utils.RAMN_Utils import *

class RAMN_USB_Handler():
    def __init__(self,port):
        self.port = port
        self.ser = None
        
    def readline(self,timeout=0):
        while True:
            line = b''
            #Slower when timeout is active
            if timeout != 0:
                self.ser.timeout = timeout
            line = self.ser.read_until(b'\r')
            if line == b'':
                return
            if len(line) <= 1 or line[-1] != ord('\r'):
                pass
            else:
                if not line[0] == ord('d'):
                    return line[:-1] # remove the trailing \r
                else:
                    log("Received debug message: " + str(line.decode()),LOG_ERROR)
    
    def open(self,autoOpen=True,emptyBuffer=True):
        self.ser = serial.Serial(self.port)
        #Turn off simulator, if on
        self.sendCommand(b'c0') #make sure CARLA is off
        self.ser.reset_input_buffer()
        if autoOpen:
            self.sendCommand(b'O')
   
    def sendCommand(self,c,end=b'\r'):
        if isinstance(c,list):
            c = bytes(c)
        if isinstance(c,str):
            c = c.encode()
        self.ser.write(c + end)
         
    def getNextCANMessage(self,timeout=0):
        c = self.readline(timeout=timeout)
        canid = None
        payload = None
        if c != None:
            if chr(c[0]) == 't':
                if len(c) >= 5:
                    canid = int(c[1:4],16)
                    if chr(c[-1]) == 'i':
                        c = c[:-1] #Remove trailing i if found
                    payload = bytes.fromhex(c[5:].decode())
              
        return payload, canid
        
    def flush(self):
        self.ser.flush()                     #Flush serial 
        while (self.ser.out_waiting != 0):
            time.sleep(0.1)
        while (self.ser.in_waiting != 0):
            self.ser.read(self.ser.in_waiting)
            time.sleep(0.01)
    
    def sendUDSCommandUSB(self,command,timeout=0,recvAnswer=True):
        if command != None:
            if len(command) > 0:
                hexStr = ''.join("{:02x}".format(i) for i in command)
                self.sendCommand('%' + "{:03x}".format(len(command)) + hexStr)
                if recvAnswer:
                    answer = self.readline(timeout=timeout)
                    if answer[0] == ord('%'):
                        size = int(answer[1:4],16)
                        if size*2 == len(answer)-4: #Check correct payload size
                            return bytes.fromhex(answer[4:].decode())
                    log("Got Unexpected Answer. CAN messages should be OFF during USB diagnostics ({})".format(answer),LOG_ERROR)
                        
                else: return []    
        return None
    
      
    def close(self,reset=True,autoReopen=False):
        #reset Filter
        self.sendCommand(b'M00000000') #Reset CAN filter
        self.sendCommand(b'm00000000') 
        self.sendCommand(b'M000') #Reset CAN filter
        self.sendCommand(b'm000')
        self.sendCommand(b'w') #Apply CAN changes

        
        if reset:
            self.sendCommand(b'n')
        elif autoReopen:
            self.sendCommand(b'O')
        else:
            self.sendCommand(b'C') #Close port
        self.flush()  
        
        time.sleep(0.2)
        self.ser.close()
        log("Successfully terminated RAMN Connection", LOG_OUTPUT)
