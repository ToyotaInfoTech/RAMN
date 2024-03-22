#!/usr/bin/env python
# Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is a script that uses UDS (over USB) to load a custom chip8 game in ECU A.
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
             
#Path of the game file         
GAME_PATH="FILES/br8kout.ch8"
             
if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, tp = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT)
    
    #Create ECU objects
    ECUA,ECUB,ECUC,ECUD = getECUHandlersUDS(ramn,tp)
    
    #Go into programming mode
    #ECUA.setUpForProgramming()
    
    log("Loading game located at {}".format(GAME_PATH),LOG_OUTPUT)   
    ECUA.loadChip8(GAME_PATH)
            
    #Close the programming session (re-enable periodic sending)
    ECUA.close(reset=False)
    ramn.close(reset=False) 
    
    click.pause()
