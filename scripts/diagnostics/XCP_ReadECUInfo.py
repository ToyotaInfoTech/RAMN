#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script that reads XCP information from the RAMN ECUs
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
             
if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, _ = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT,filterID=b'M550', filterMask=b'm7F0',filterIDExtended=b'M00000550', filterMaskExtended=b'm000007F0')
    
    #Create ECU objects
    _, ECUB,ECUC,ECUD = getECUHandlersXCP(ramn)
    
    if ECUB.connect() != None:
        log("ECU B Accepted Connection",LOG_OUTPUT)
        ECUB.getID()
        
    if ECUC.connect() != None:
        log("ECU C Accepted Connection",LOG_OUTPUT)
        ECUC.getID()
        
    if ECUD.connect() != None:
        log("ECU D Accepted Connection",LOG_OUTPUT)
        ECUD.getID()    
        
    ramn.close(reset=False) 
    
    click.pause()
    
