#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is a script to exchange large ISO-TP data at maximum speed to ensure firmware is RAMN's working at high loads.
#The "echo" function is a custom RoutineControl implemented by RAMN's ECUs, designed to send back whatever data was provided as input.
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
             
#Number of time echo function is tested on each ECU             
FRAME_COUNT=10

#Payload Size for the echo Function. Must be below 0xFFB, since the "routine control" header already takes 4 bytes.
ECHO_PAYLOAD_SIZE = 0xFFB             
             
#Function that repeats an ECHO operation for specified amount of time, with payload of specified size.
#Returns number of echo that failed.
UPLINKDOWNLINK   = 0   
DOWNLINK         = 1    
UPLINK           = 2
def repeat_ping(ecu,frameCount, payloadSize,direction=UPLINKDOWNLINK):     
    frameDropped = 0
    log("Pinging ECU {}".format(ecu.name),LOG_OUTPUT)   
    if (direction == 0) or (direction == 1):
        data=[i&0xFF for i in range(payloadSize)]
    else:
        data = int16ToList(0xFFB)
    for i in range(0,frameCount):
        if ecu.echo(data=data,direction = direction):
            log("Echo Succeeded",LOG_DATA)
        else:
            frameDropped += 1
            log("Echo Failed",LOG_ERROR)   
    return frameDropped
             
if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, tp = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT)
    
    #Create ECU objects
    ECUA,ECUB,ECUC,ECUD = getECUHandlersUDS(ramn,tp)
    
    #Go into programming mode
    ECUA.setUpForProgramming()
    ECUB.setUpForProgramming()
    ECUC.setUpForProgramming()
    ECUD.setUpForProgramming()
  
    #Perform load test for each ECU
    frameDropped = 0
    
    log("testing both Uplink and Downlink", LOG_OUTPUT)
    frameDropped += repeat_ping(ECUA, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINKDOWNLINK)
    frameDropped += repeat_ping(ECUB, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINKDOWNLINK)
    frameDropped += repeat_ping(ECUC, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINKDOWNLINK)
    frameDropped += repeat_ping(ECUD, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINKDOWNLINK)
    
    log("testing Downlink", LOG_OUTPUT)
    frameDropped += repeat_ping(ECUA, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=DOWNLINK)
    frameDropped += repeat_ping(ECUB, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=DOWNLINK)
    frameDropped += repeat_ping(ECUC, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=DOWNLINK)
    frameDropped += repeat_ping(ECUD, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=DOWNLINK)
    
    log("testing Uplink", LOG_OUTPUT)
    frameDropped += repeat_ping(ECUA, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINK)
    frameDropped += repeat_ping(ECUB, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINK)
    frameDropped += repeat_ping(ECUC, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINK)
    frameDropped += repeat_ping(ECUD, FRAME_COUNT, ECHO_PAYLOAD_SIZE,direction=UPLINK)
            
    #Close the programming session (re-enable periodic sending)
    ECUA.close(reset=False)
    ECUB.close(reset=False)
    ECUC.close(reset=False)
    ECUD.close(reset=False)
    ramn.close(reset=False) 
    
    if frameDropped > 0:
        log("Load test FAILED: {} frames dropped".format(frameDropped),LOG_ERROR)
    else:
        log("Success: 0 frame dropped",LOG_OUTPUT)
    
    click.pause()
