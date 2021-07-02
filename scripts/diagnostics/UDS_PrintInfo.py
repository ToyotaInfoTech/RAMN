#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script showing how to use ReadDataByIdentifier to retrieve information from RAMN's ECUs
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
             
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
  
    #Print info of each ECU
    ECUA.printInfo()
    ECUB.printInfo()
    ECUC.printInfo()
    ECUD.printInfo()
    
    #Close the programming session (re-enable periodic sending)
    ECUA.close(reset=False)
    ECUB.close(reset=False)
    ECUC.close(reset=False)
    ECUD.close(reset=False)
    ramn.close(reset=False) 
    
    click.pause()