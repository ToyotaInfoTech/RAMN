#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This is an example script showing how to use SET MTA and UPLOAD with XCP on RAMN
import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
from pathlib import Path

#The block size used for UPLOAD commands
BLOCK_SIZE=0x7

#Dump specified areas.
def dumpECUMemory(ecu,hexfilename,binprefix):
        areas=ECUreadableRange.values() #Edit the "ECUreadableRange" variable to add/remove memory areas.
        ih = intelhex.IntelHex()
        log("Reading ECU {} Info".format(ecu.name),LOG_OUTPUT)
        if ecu.connect() != None: log("ECU {} Accepted Connection".format(ecu),LOG_DEBUG)
        for area in ECUreadableRange.keys():
            start = ECUreadableRange[area][0]
            end = ECUreadableRange[area][1]
            size = end-start
            log("Reading area {} : 0x{:08x} to 0x{:08x} ".format(area,start,end),LOG_DATA)
            d = ecu.dumpArea(start,end,blockSize=BLOCK_SIZE)
            if d != None:
                if len(d) == (size):
                    for i in range(size):
                        ih[start+i] = d[i]  
                    with open(binprefix + "_" + area + ".bin","wb") as f:
                            f.write(bytes(d))
                else:
                    print(d)
                    log("Dump of {} Failed, Insuffient data received".format(area),LOG_ERROR)
            else:
                log("Dump of {} Failed, no data received".format(area),LOG_ERROR)
        ih.tofile(hexfilename, format='hex')

from utils.RAMN_Diag_Main import *
             
if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, _ = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT,filterID=b'M00000550', filterMask=b'm000007F0')
    
    #Create ECU objects
    _, ECUB,ECUC,ECUD = getECUHandlersXCP(ramn)
    
    #Create folders to hold dump files
    Path("DUMP").mkdir(parents=True, exist_ok=True)
    Path("DUMP/BIN").mkdir(parents=True, exist_ok=True)
    
    #Dump all readable memory area.
    dumpECUMemory(ECUB,"DUMP/DUMP_ECUB.hex", "DUMP/BIN/ECUB")
    dumpECUMemory(ECUC,"DUMP/DUMP_ECUC.hex", "DUMP/BIN/ECUC")
    dumpECUMemory(ECUD,"DUMP/DUMP_ECUD.hex", "DUMP/BIN/ECUD")    
    
    ramn.close(reset=False) 
    
    click.pause()
