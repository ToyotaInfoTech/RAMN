#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from utils.RAMN_USB_Handler import *

#This is a simple class to communicate over ISO-TP
#It is a simplified, non-standard compliant implementation, meant for simple testing of RAMN ECUs features over ISO-TP

#Class that handles formatting of outgoing ISO-TP messages
class ISOTPTXFormatter():

    def __init__(self,data,txid,rxid):
        if len(data) > 0xFFF:
            log("ISO TP message request too large",LOG_ERROR)
            return None
        self.data = data
        self.txid = txid
        self.rxid = rxid
        self.index = 0
        self.seq = 1
        self.targetBS = 0
        self.targetST = 0
        self.receivedFC = False
        self.blockCounter = 0
           
    def isFinished(self):
        return self.index >= len(self.data)
        
    def getNextTXMsg(self):
        result = None
        data = self.data
        
        #Format next Message
        if self.index >= len(data):
            #No more data to send, should not reach here
            result = None                
        elif len(data) <= 7:
            #Single Frame is enough for sending full payload
            result = "t{:03x}{:1x}{:02x}".format(self.txid,len(data)+1,len(data)) + listToHex(data)
            self.index += len(data)
            self.receivedFC = True #Consider FC Received
        else:
            if self.index < 6:
                #first Frame
                result = "t{:03x}81{:03x}".format(self.txid,len(data)) + listToHex(data[0:6])
                self.index += 6
                self.receivedFC = False
                self.blockCounter = 0
            else:
                #Consecutive Frame, only send if Flow Control Frame was received
                if self.receivedFC:
                    size = min(7,len(data)-self.index)
                    result = "t{:03x}{:1x}2{:1x}".format(self.txid,size+1,self.seq) + listToHex(data[self.index:self.index+size])
                    self.seq = (self.seq + 1)&0xF
                    self.index += size
                    self.blockCounter += 1
                    
        #Wait specified time     
        if self.targetST != 0:
            time.sleep(self.targetST/1000)
            
        #If reached block size, wait for next Flow Control frame   
        if self.targetBS != 0:
            if self.blockCounter == self.targetBS:
                self.receivedFC = False
                self.blockCounter = 0 #reset Block Counter
            
        return result         

#Class that handles receiving of incoming ISO-TP messages
class ISOTPRXFormatter():

    def __init__(self,bs,st):
        self.bs = bs
        self.st = st
        self.data = []
        self.expectedSize = 0x1000
        self.index = 0
        self.expectedSeq = 1
        self.isFC = False
        self.isFirstFrame = False
        self.targetST = 0
        self.targetBS = 0
        self.blockCounter = 0
  
    def isFinished(self):
        return self.index >= (self.expectedSize)
        
    #Function to check whether we should transmit a Flow Control Frame or not
    def mustSendFC(self):
        if self.isFirstFrame:
            return True
        if self.bs == 0:
            return False
        if (self.blockCounter % self.bs) == 0:
            self.blockCounter = 0
            return True
        return False
     
    #Add an incoming message to be processed by the ISO-TP receiver module
    def addMessage(self,payload):
        self.isFC = False
        self.isFirstFrame = False
        if len(payload) > 0:
            c = payload[0]
            if (c & 0xF0) == 0x00: #Single Frame
                self.expectedSize =   c & 0x0F
                
                if self.expectedSize+1 <= len(payload):
                    self.data += payload[1:]
                    self.index += len(payload)-1
                    
            elif (c & 0xF0) == 0x10: #First Frame
                self.expectedSize =   ((payload[0]& 0x0F) << 8) + payload[1]
                self.data += payload[2:]
                self.index += len(payload)-2
                self.isFirstFrame = True
                
            elif (c & 0xF0) == 0x20: #Consecutive Frame
                if self.expectedSeq != c&0xF:
                    log("Received Wrong Sequence Number in ISO TP : Expected {:01x}, got {:1x}".format(self.expectedSeq, c&0xF),LOG_ERROR)
                else:
                    self.expectedSeq = ((c&0xF) + 1)%0x10
                    self.data += payload[1:]
                    self.index += len(payload)-1
                    self.blockCounter += 1
                    
            elif (c & 0xF0) == 0x30: #Flow Control
                self.isFC = True  
                if len(payload) > 1:
                    self.targetBS = payload[1]
                if len(payload) > 2:
                    self.targetST = payload[2]
                    if (payload[2] >= 0xF1) and (payload[2] <0xF9):
                        self.targetST = 1  #Consider microseconds delay as 1ms
            else:
                log("Received invalid ISO-TP header", LOG_ERROR)
        
#Class that handles sending and receiving of ISO-TP frames. The "update" function must be called periodically.
class RAMN_ISOTP_Handler:

    def __init__(self,r,p,bs=0,st=0):
        self.pairs = p
        self.ramn = r
        self.rxList = {}
        self.txList = {}
        self.bs=bs
        self.st=st

    #Function to request the sending of a full ISO-TP payload
    def sendFrame(self,payload, tx,rx):
        if tx in self.txList.keys():
            log("Received ISO TP Request but current transmission not over", LOG_WARNING)
            self.txList.pop(tx)
        self.txList[tx] = ISOTPTXFormatter(payload, txid = tx, rxid = rx)      

    def isTxOver(self):
        return len(self.txList.keys()) == 0
    
    #Function to update the ISO-TP Engine
    def update(self,recvAnswer=True,timeout=0):
        result = None 
        txOverList = []
        expectingFC=False
        
        #Process Outgoing messages
        for k in self.txList.keys():
            while (True):
                msg = self.txList[k].getNextTXMsg()
                if not self.txList[k].receivedFC:
                    expectingFC = True
                if msg == None:
                    break
                self.ramn.sendCommand(msg)
                if self.txList[k].isFinished():
                    txOverList.append(k)
                    break
        for k in txOverList:
            self.txList.pop(k)
            
        #Process incoming messages with expected (Response or Flow Control frames)
        if recvAnswer or expectingFC:
            payload, canid = self.ramn.getNextCANMessage(timeout=timeout)
            
            if payload != None:
                if canid in self.pairs.keys():
                    #Received Message with ID we must receive
                    if canid not in self.rxList:
                        #new Message
                        self.rxList[canid] = ISOTPRXFormatter(bs=self.bs,st=self.st)
                    self.rxList[canid].addMessage(payload) 
                    
                    #Send Flow Control Frames if required
                    if self.rxList[canid].mustSendFC():
                        self.ramn.sendCommand("t{:03x}330{:02x}{:02x}".format(self.pairs[canid],self.bs,self.st))
                        
                    #Process Incoming Flow Control frames    
                    if self.rxList[canid].isFC: 
                        if self.pairs[canid] in self.txList.keys():
                            self.txList[self.pairs[canid]].receivedFC = True
                            self.txList[self.pairs[canid]].targetBS = self.rxList[canid].targetBS
                            self.txList[self.pairs[canid]].targetST = self.rxList[canid].targetST
                    
                    #If a message was reconstructed, send it back
                    if self.rxList[canid].isFinished():
                        #end of transfer
                        result = self.rxList[canid].data
                        self.rxList.pop(canid) #Remove message
                        
            return result
                