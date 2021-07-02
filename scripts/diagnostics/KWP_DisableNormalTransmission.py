#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script showing how to use "Disable Normal Transmission" with KWP2000 on RAMN
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
             
if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, tp = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT)
    
    #Create ECU objects
    _,ECUB,ECUC,ECUD = getECUHandlersKWP(ramn,tp)
    
    ECUB.diagnosticSessionControl(0x85)
    ECUC.diagnosticSessionControl(0x85)
    ECUD.diagnosticSessionControl(0x85)
    
    #Try Disable Normal Transmission for ECUs B, C and D
    if ECUB.disablePeriodicSending() != None:
        log("ECU B Accepted Request",LOG_OUTPUT)
    if ECUC.disablePeriodicSending() != None:
        log("ECU C Accepted Request",LOG_OUTPUT)
    if ECUD.disablePeriodicSending() != None:
        log("ECU D Accepted Request",LOG_OUTPUT)

    ramn.close(reset=False) 
    
    click.pause()
