#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script showing how to use RequestDownload, RequestUpload, TransferData and RoutineControl directions to reprogram RAMN's ECU
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
import os

VERIFY_AFTER_PROGRAMMING = False

#Function to reprogram an ECU with specified firmware.
def reprogramECU(ecu,firmwarePath,requestSwap=True):
    if os.path.exists(RAMN_Utils.ECUB_FIRMWARE_PATH): 
        log("Found ECU {} Firmware Path: {}".format(ecu.name,firmwarePath),LOG_DATA)
    else:
        log("Did not find ECU {} Firmware Path: {}, skipping".format(ecu,firmwarePath),LOG_DATA)
        return
        
    if ecu.reprogram(firmwarePath,swap=requestSwap):
        log("Reprogramming of ECU {} Succeeded".format(ecu.name),LOG_OUTPUT)
    else:
        log("Reprogramming of ECU {} Failed".format(ecu.name),LOG_ERROR)
    if requestSwap:
        ecu.setUpForProgramming() #put back in reprogramming mode
        
#Function to verify that provided firmware is correctly flashed in the ECU.
def verifyECU(ecu,firmwarePath):
    if os.path.exists(firmwarePath): 
        log("Found ECU {} Firmware Path: {}".format(ecu.name,firmwarePath),LOG_DATA)
    else:
        log("Did not find ECU {} Firmware Path: {}, skipping".format(ecu,firmwarePath),LOG_DATA)
        return
        
    if ecu.verify(firmwarePath): 
        log("ECU {} Firmware Verify Succeeded".format(ecu.name),LOG_OUTPUT)
    else:
        log("ECU {} Firmware Verify Failed".format(ecu.name),LOG_ERROR)
        
if __name__ == '__main__':

    #Path of firmware files (by default read from settings files)
    ECUA_Firmware_Path = RAMN_Utils.ECUA_FIRMWARE_PATH
    ECUB_Firmware_Path = RAMN_Utils.ECUB_FIRMWARE_PATH
    ECUC_Firmware_Path = RAMN_Utils.ECUC_FIRMWARE_PATH
    ECUD_Firmware_Path = RAMN_Utils.ECUD_FIRMWARE_PATH

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
  
    #Reprogram every ECU except A
    log("Reprogramming ECUs B C D",LOG_OUTPUT)
    reprogramECU(ECUB, ECUB_Firmware_Path)
    reprogramECU(ECUC, ECUC_Firmware_Path)
    reprogramECU(ECUD, ECUD_Firmware_Path)

    #Verify firmware of each ECU
    if VERIFY_AFTER_PROGRAMMING:
        log("----------------------------",LOG_OUTPUT)
        log("Verifying Software ECUs (Except ECU A)",LOG_OUTPUT)
        verifyECU(ECUB, ECUB_Firmware_Path)
        verifyECU(ECUC, ECUC_Firmware_Path)
        verifyECU(ECUD, ECUD_Firmware_Path)

    log("Reprogramming ECU A",LOG_OUTPUT)
    ECUA.setUpForProgramming()
    reprogramECU(ECUA, ECUA_Firmware_Path,requestSwap=False)
    
    #Ask for a bank swap (will reset ECU A)
    log("Requesting Bank swap for ECU A, will reset",LOG_OUTPUT)
    try:
        ECUA.routineControl(0x01,0xFF01, recvAnswer=False)
        time.sleep(2)
        ramn.close(reset=True) 
    except:
        log("Serial Exception occurred: ECU A likely reset ( = successful reprogram)",LOG_OUTPUT)
        
    click.pause()    