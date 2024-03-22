#!/usr/bin/env python
# Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is a script that uses UDS (over USB) to display a custom image on ECU A.
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
             
#Path of the image           
IMAGE_PATH="FILES/image.png"
             
if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, tp = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT)
    
    #Create ECU objects
    ECUA,ECUB,ECUC,ECUD = getECUHandlersUDS(ramn,tp)
    
    #Go into programming mode
    #ECUA.setUpForProgramming()
    
    log("This module requires that you install the PIL python module ($pip install Pillow)", LOG_WARNING)
    log("Loading image located at {}".format(IMAGE_PATH),LOG_OUTPUT)   
    ECUA.displayImage(IMAGE_PATH)
            
    #Close the programming session (re-enable periodic sending)
    ECUA.close(reset=False)
    ramn.close(reset=False) 
    
    click.pause()
