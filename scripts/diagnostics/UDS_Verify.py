#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script showing how to use RequestUpload and TransferData to verify that all ECUs of RAMN have been correctly programmed.
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
import os
        
#Function to verify that provided firmware is correctly flashed in the ECU.
def verifyECU(ecu,firmwarePath):
    if os.path.exists(firmwarePath): 
        log("Found ECU {} Firmware Path: {}".format(ecu.name,firmwarePath),LOG_DATA)
    else:
        log("Did not find ECU {} Firmware Path: {}, skipping".format(ecu,firmwarePath),LOG_DATA)
        return False
        
    if ecu.verify(firmwarePath): 
        log("ECU {} Firmware Verify Succeeded".format(ecu.name),LOG_OUTPUT)
        return True
    else:
        log("ECU {} Firmware Verify Failed".format(ecu.name),LOG_ERROR)
        return False
        
           
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

    #Verify firmware of each ECU
    log("Verifying Firmware of ECUs",LOG_OUTPUT)
    allVerified = True
    if not verifyECU(ECUA, ECUA_Firmware_Path): allVerified = False
    if not verifyECU(ECUB, ECUB_Firmware_Path): allVerified = False
    if not verifyECU(ECUC, ECUC_Firmware_Path): allVerified = False
    if not verifyECU(ECUD, ECUD_Firmware_Path): allVerified = False
    
    #Close the programming session (re-enable periodic sending)
    ECUA.close(reset=False)
    ECUB.close(reset=False)
    ECUC.close(reset=False)
    ECUD.close(reset=False)
    ramn.close(reset=False)

    if allVerified:
            log("Firmware of all ECUs successfully verified",LOG_OUTPUT)
    else:
        log("ECU Firmware verify FAILED",LOG_ERROR)    
        
    click.pause()
