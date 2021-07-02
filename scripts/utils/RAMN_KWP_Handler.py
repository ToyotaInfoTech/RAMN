#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Collection of functions to make it more intuitive to access KWP features of RAMNs ECU over python.
#Currently a simplified implementation.

from utils.RAMN_ISOTP_Handler   import *

#Class that handles KWP communication for one ECU
class RAMN_KWP_Handler:
    def __init__(self,ramn,tp,txid,rxid,name="Unknown"):
        self.ramn = ramn
        self.txid=txid
        self.rxid=rxid
        self.tp = tp 
        self.ramn = ramn
        self.name = name
    
    #Send a Raw payload over KWP
    def sendRawData(self,toSend,timeout=RAMN_Utils.DEFAULT_TIMEOUT,recvAnswer=True):
        if isinstance(toSend,list):
            toSend = bytes(toSend)
        if len(toSend) > 0xFFF:
            log("Requested a payload which size is too big: {}".format(len(toSend)),LOG_ERROR)
            return None
                
        log("SEND: " + ''.join("{:02x}".format(i) for i in toSend),LOG_DEBUG)

        self.tp.sendFrame(toSend, self.txid, self.rxid)    
        tstart = time.time()
        while timeout == 0 or (time.time() - tstart < timeout):

            payload = self.tp.update(recvAnswer,timeout)
            if recvAnswer:
                if payload != None:
                    log("RECV: " + ''.join("{:02x}".format(i) for i in payload),LOG_DEBUG)
                    return payload
            else:
                if self.tp.isTxOver():
                    return []
            return None
     
    #Send a KWP command with specified parameters
    def sendCommand(self, commandID, params=[],timeout=RAMN_Utils.DEFAULT_TIMEOUT, recvAnswer=True):
        r = self.sendRawData([commandID] + params,timeout, recvAnswer)
        if recvAnswer:
            if r == None:
                log("Timeout",LOG_ERROR)
                return 'b'
            elif len(r) == 0:
                log("Received Empty Data",LOG_ERROR)
                return b''
            elif r[0] == commandID + 0x40:
                return list(r[1:])
            elif r[0] == 0x7F: #negative command
                log("Received Negative RESPONSE: " + hex(r[2]) + " for COMMAND " + hex(r[1]) + " ",LOG_ERROR)
            else: log("Unexpected KWP RESPONSE: " + hex(r[0]),LOG_ERROR)
            return None
        return []
        
    #Sends a tester present command and wait for answer
    def testerPresent(self):
        if self.sendCommand(0x3E,[0x01]) != None : 
            log("ECU responsed to Tester Present (TX:0x{:03x} RX:0x{:03x})".format(self.txid,self.rxid),LOG_DATA)
            return True
        else: 
            log("ECU did not respond to Tester Present",LOG_ERROR)
            return False
    
    #Goes to specified diagnostic session
    def diagnosticSessionControl(self, session):
        return self.sendCommand(0x10,[session])
        
    #Request that ECU stops sending periodic messages
    def disablePeriodicSending(self):
        return self.sendCommand(0x28,[0x01])
        
    #Request that ECU resumes sending periodic messages    
    def enablePeriodicSending(self):
        return self.sendCommand(0x29,[0x01])
    
  