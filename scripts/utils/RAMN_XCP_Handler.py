#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Collection of functions to make it more intuitive to access XCP features of RAMNs ECU over python.
#Partially incorrect implementation for compatibility with some tools.

from utils.RAMN_Utils   import *

#Class that handles XCP communication for one ECU
class RAMN_XCP_Handler:
    def __init__(self,ramn,txid,rxid,name="Unknown"):
        self.ramn = ramn
        self.txid=txid
        self.rxid=rxid
        self.ramn = ramn
        self.name = name
    
    #Send a Raw payload over XCP
    def sendRawData(self,toSend,timeout=RAMN_Utils.DEFAULT_TIMEOUT,recvAnswer=True):
        if isinstance(toSend,list):
            toSend = bytes(toSend)
        if len(toSend) > 0x8:
            log("Requested a payload which size is too big: {}".format(len(toSend)),LOG_ERROR)
            return None
                
        log("SEND: " + ''.join("{:02x}".format(i) for i in toSend),LOG_DEBUG)

        dlc = len(toSend)
        
        hexstr = "t{:03x}{:01x}".format(self.txid,dlc)  + ''.join("{:02x}".format(i) for i in toSend)
        self.ramn.sendCommand(hexstr)
        res, canid =  self.ramn.getNextCANMessage()
        if res != None:
            if canid == self.rxid:
                log("RECV: " + ''.join("{:02x}".format(i) for i in res),LOG_DEBUG)
                if res[0] != 0xFF:
                    log("XCP ERROR CODE " + hex(res[1]),LOG_ERROR)
        return res
            
    def connect(self):
        return self.sendRawData([0xFF,0x00])
        
    def setMTA(self,addr):
        return self.sendRawData([0xF6,0x00,0x00,0x00] + int32ToList(addr))
        
    def upload(self,size):
        return self.sendRawData([0xF5,size&0xFF])[1:]
        
    def getID(self):
        res =  self.sendRawData([0xFA,0x00])
        if res != None:
            payloadSize = res[4]
            log("ECU ID Name is {} bytes, requesting upload".format(payloadSize),LOG_DEBUG)
            
            res =  self.upload(payloadSize)
            if res != None:
                log("ECU ID is reported as: " + res.decode())
                return res
                
    def dumpArea(self,start,end,blockSize=7):
        size = end - start
        if size > 0:
            res = self.setMTA(start)
            dump = b''
            if res != None:
                if len(res) == 1 and res[0] == 0xFF: #Positive Answer
                    while True:
                        chunkSize = min(size,blockSize)
                        dump += self.upload(chunkSize)
                        size -= chunkSize
                        if size == 0:
                            break
                return dump
            else:
                log("Could not set MTA", LOG_ERROR)
        return None
        
        
