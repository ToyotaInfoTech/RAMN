#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script showing how to use Tester Present with KWP2000 on RAMN
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
    
    #Try Tester Present for ECUs B, C and D
    if ECUB.testerPresent():
        log("ECU B KWP SERVER ACTIVE",LOG_OUTPUT)
    if ECUC.testerPresent():
        log("ECU C KWP SERVER ACTIVE",LOG_OUTPUT)
    if ECUD.testerPresent():
        log("ECU D KWP SERVER ACTIVE",LOG_OUTPUT)

    ramn.close(reset=False) 
    
    click.pause()