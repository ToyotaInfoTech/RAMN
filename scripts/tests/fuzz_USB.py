#!/usr/bin/env python
# Copyright (c) 2025 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import random
import secrets

#This is a script that tests the USB interface (serial interface layer only) of ECU A.
#Note that you may lose control of ECU A screen during fuzzing for various reasons which are not  crashes (change of baudrate, change of CAN mask, etc...)
#It is preferable to stay on the "STATS" screen during fuzzing to better check on progress.

import sys
sys.path.append("..")
from utils.RAMN_Diag_Main import *
             
       
NUMBER_OF_REPEATS                   = 100            
FUZZ_COMMAND_MAX_SIZE               = 8195  

LOCAL_USB_COMMAND_BUFFER_SIZE       = 0x200     # For CLI commands
USB_APP_RX_DATA_SIZE                = 2048      # APP_RX_DATA_SIZE
USB_COMMAND_BUFFER_SIZE             = (8195)  # Expected size of USB command buffer
USB_RX_BUFFER_SIZE                  = 15000     # Expected RX buffer size 
LARGE_COMMAND_SIZE                  = 0x10000   # Max size to test

           
if __name__ == '__main__':
    #Select Verbose level
    RAMN_Utils.setVerboseLevel(RAMN_Utils.DEFAULT_VERBOSE)
    
    #Create a RAMN USB Handler object
    ramn, tp = getRAMNHandlers(RAMN_Utils.RAMN_DEFAULT_PORT)
    
    log("Fuzzing USB slcan interface", LOG_OUTPUT)
    
    log("Sending large packets", LOG_OUTPUT)
    
    #Test conditions close to important buffer sizes
    
    for i in range(10,-10,-1):
        log("Testing USB CDC payload size 0x{:x}".format(LOCAL_USB_COMMAND_BUFFER_SIZE-i), LOG_DEBUG)
        ramn.sendCommand("A"*(LOCAL_USB_COMMAND_BUFFER_SIZE-i) + "\r")
    
    for i in range(10,-10,-1):
        log("Testing USB CDC payload size 0x{:x}".format(USB_APP_RX_DATA_SIZE-i), LOG_DEBUG)
        ramn.sendCommand("A"*(USB_APP_RX_DATA_SIZE-i) + "\r")
         
    
    for i in range(10,-10,-1):
        log("Testing USB CDC payload size 0x{:x}".format(USB_COMMAND_BUFFER_SIZE-i), LOG_DEBUG)
        ramn.sendCommand("A"*(USB_COMMAND_BUFFER_SIZE-i) + "\r")
    
    
    for i in range(10,-10,-1):
        log("Testing USB CDC payload size 0x{:x}".format(USB_RX_BUFFER_SIZE-i), LOG_DEBUG)
        ramn.sendCommand("A"*(USB_RX_BUFFER_SIZE-i) + "\r")
    
    log("Testing USB CDC payload size 0x{:x}".format(LARGE_COMMAND_SIZE), LOG_DEBUG)
    ramn.sendCommand("A"*LARGE_COMMAND_SIZE + "\r")
    
    for i in range(NUMBER_OF_REPEATS):
        command = secrets.token_bytes(random.randint(1, FUZZ_COMMAND_MAX_SIZE))
        if command[0] == 'n' or b'\rn' in command or b'\nn' in command:
            log("Command may reset board, skip", LOG_DEBUG)
            continue
        if command[0] == '#' or b'\r#' in command or b'\n#' in command:
            log("Command may go into CLI mode, skip", LOG_DEBUG)
            continue

        log("testing: {}".format(command), LOG_DEBUG) 
        ramn.sendCommand(command)
          
      
    log("Empyting buffer...", LOG_OUTPUT)
    time.sleep(2)
    ramn.flush()
    
    ramn.sendCommand(b'#\r')
    ramn.sendCommand(b'#\r') #Twice in case of a problem with previous command
    
    log("Fuzzing USB CLI interface", LOG_OUTPUT)

    log("Sending large packets", LOG_OUTPUT)

    for i in range(10,-10,-1):
        ramn.sendCommand("A"*(LOCAL_USB_COMMAND_BUFFER_SIZE-i) + "\r")
    
    for i in range(10,-10,-1):
        ramn.sendCommand("A"*(USB_APP_RX_DATA_SIZE-i) + "\r")
    
    for i in range(10,-10,-1):
        ramn.sendCommand("A"*(USB_COMMAND_BUFFER_SIZE-i) + "\r")
    
    for i in range(10,-10,-1):
        ramn.sendCommand("A"*(USB_RX_BUFFER_SIZE-i) + "\r")
        
    ramn.sendCommand("A"*LARGE_COMMAND_SIZE + "\r")

    log("testing random commands of random lengths", LOG_DEBUG)
    for i in range(NUMBER_OF_REPEATS):
        command = secrets.token_bytes(random.randint(1, FUZZ_COMMAND_MAX_SIZE))
        log("testing: {}".format(command), LOG_DEBUG) 
        ramn.sendCommand(command)
        
    log("testing large commands", LOG_OUTPUT)
    
    for i in range(NUMBER_OF_REPEATS):
        command = secrets.token_bytes(FUZZ_COMMAND_MAX_SIZE)
        log("testing: {}".format(command), LOG_DEBUG) 
        ramn.sendCommand(command)    
    
    log("Empyting buffer...", LOG_OUTPUT)
    time.sleep(2)
    ramn.flush()
    
    log("Reverting to slcan", LOG_OUTPUT)
    
    ramn.sendCommand(b'b\r')  # Make sure we are not in CLI mode 
    ramn.sendCommand(b'b\r')  # Command above may fail, so try again to be sure
    ramn.sendCommand(b'S6\r') # Make sure we haven't changed the baud rate
    
    time.sleep(0.1)
    ramn.flush()
    
    log("Verify that ECU still answers with version number below, and verify that ECU A screen is still responsive", LOG_DATA)
    ramn.sendCommand(b'V\r') #Ask for version
    log(ramn.readline(), LOG_DATA)
    
    ramn.close(reset=False) 
    
    click.pause()
