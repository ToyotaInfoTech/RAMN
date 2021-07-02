#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Resets the receiving filter of ECU A. Can be useful if a diag script was interrupted before the filter could be reset.
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
from pathlib import Path

if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, tp = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT)
   
    #Close the filter correctly
    ramn.close(reset=False,autoReopen=True) 
    
    click.pause()
