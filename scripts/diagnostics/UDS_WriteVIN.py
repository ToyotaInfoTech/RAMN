#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script showing how to use the WriteDataByIdentifier and ReadDataByIdentifier to read/write the VIN of RAMN's ECU
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *

#VINs to write for each ECU
VIN_ECUA = "VIN123456789ABC_A"
VIN_ECUB = "VIN123456789ABC_B"
VIN_ECUC = "VIN123456789ABC_C"
VIN_ECUD = "VIN123456789ABC_D"

#Function to write VIN in specified ECU
def writeVIN(ecu, vin):
    if ecu.writeVIN(vin) != None:
        log("ECU {}: VIN WRITE SUCCESSFUL ({})".format(ecu.name,vin),LOG_OUTPUT)
    else:
        log("ECU {}: VIN WRITE FAILED".format(ecu.name),LOG_ERROR)

#Function to check that specified VIN has been correctly written in specified ECU.
def verifyVIN(ecu,expectedVIN):
    vin = ecu.readVIN()
    if vin != None:
        log("ECU {}: {}".format(ecu.name,vin),LOG_OUTPUT)
        if vin == expectedVIN:
            log("VIN Read/Write Verified", LOG_DATA)
        else:
            log("VINs Read/Write do not match. Expected {}, got {}".format(expectedVIN,vin), LOG_ERROR)
    else:
        log("ECU {}: NO VIN COULD BE READ".format(ecu.name),LOG_OUTPUT)

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
  
    #Write VINs
    log("Writing VIN to ECUs",LOG_OUTPUT)
    writeVIN(ECUA,VIN_ECUA)
    writeVIN(ECUB,VIN_ECUB)
    writeVIN(ECUC,VIN_ECUC)  
    writeVIN(ECUD,VIN_ECUD)      

    #Verify VINs
    log("----------------------------",LOG_OUTPUT)
    log("Verifying VIN",LOG_OUTPUT)
    verifyVIN(ECUA,VIN_ECUA)
    verifyVIN(ECUB,VIN_ECUB)
    verifyVIN(ECUC,VIN_ECUC)
    verifyVIN(ECUD,VIN_ECUD)

    #Close the programming session (re-enable periodic sending)
    ECUA.close(reset=False)
    ECUB.close(reset=False)
    ECUC.close(reset=False)
    ECUD.close(reset=False)
    ramn.close(reset=False) 
    
    click.pause()