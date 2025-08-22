#!/usr/bin/env python

# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#this script allows reprogramming of STM32L552 microcontrollers over FDCAN (CAN-FD) based on AN5405

import serial
import time
import click
from intelhex import IntelHex
from pathlib import Path
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
import utils.RAMN_Utils
    
#Memory address ranges of STM32L552 (STM32L552CE) are as below (manual p179)
#CODE NON-SECURE 
#0x0000 0000 ~ 0x0800 0000 -> Code (External Memories)
#0x0800 0000 ~ 0x0808 0000 -> Flash memory (ECU Application)
#0x0A00 0000 ~ 0x0A03 0000 -> SRAM1
#0x0A03 0000 ~ 0x0A04 0000 -> SRAM2
#0x0BF9 0000 ~ 0x0BF9 7FFF -> System Memory
#0x0BFA 0000 ~ 0x0BFB 0000 -> OTP Area (/!\ One Time Programmable Memory - NOT REVERSIBLE /!\)
#0x4002 2040 ~ 0x400 22140    #Option Bytes (/!\ careful when overwriting - you may lose access to bootloader from USB /!\)
                                                       
#CODE NON-SECURE CALLABLE
#0x0C00 0000 ~ 0x0C08 0000 -> FLASH
#0x0E00 0000 ~ 0x0E03 0000 -> SRAM1
#0x0E03 0000 ~ 0x0E04 0000 -> SRAM2
#0x0FF8 0000 ~ 0x0FF8 27FF -> RSS

#PERIPHERALS NON-SECURE
#0x4000 0000 ~ 0x4000 E000 -> APB1
#0x4001 0000 ~ 0x4001 6800 -> APB2
#0x4002 0000 ~ 0x4003 3400 -> AHB1
#0x4202 0000 ~ 0x420C 8400 -> AHB2
#0x4402 0000 ~ 0x4402 2000 -> AHB3

#PERIPHERALS NON-SECURE CALLABLE
#0x5000 0000 ~ 0x5000 E000 -> APB1
#0x5001 0000 ~ 0x5001 6800 -> APB2
#0x5002 0000 ~ 0x5003 3000 -> AHB1
#0x5202 0000 ~ 0x520C 8000 -> AHB2
#0x5402 0000 ~ 0x5402 2000 -> AHB3

#0x2000 0000 ~ 0x2003 0000 -> SRAM1 NON-SECURE
#0x2003 0000 ~ 0x3000 0000 -> SRAM2 NON-SECURE
#0x3000 0000 ~ 0x3003 0000 -> SRAM1 NON-SECURE CALLABLE
#0x3003 0000 ~ ?           -> SRAM2 NON-SECURE CALLABLE

#0x6000 0000 ~ 0x7000 0000 -> FMC bank 1 Non-secure
#0x8000 0000 ~ 0x9000 0000 -> FMC bank 3 Non-Secure
#0x9000 0000 ~ 0xA000 0000 -> OCTOSPI1 bank Non-secure
#0xE000 0000 ~ 0xFFFF FFFF -> Cortex M33 Non-secure

#Unique device ID register: 0x0BFA 0590, 96Bits

#memory areas readable by the CAN bootloader are as below

MEMORY_SECTORS = [i for i in range(0,256)]

readableRange = {
    "FLASH":       (0x08000000,0x08080000), #(0x08000000,0x08040000) for STM32L552CC
    "SYSTEM":      (0x0BF90000,0x0BF98000),
    "OTP":         (0x0BFA0000,0x0BFA0200),
    "OPTIONBYTES": (0x40022040,0x40022140),
    #"RSS":         (0x0FF90000, 0x0FF91FFF),
    #"RSS LIBRARY": (0x0FF92000, 0x0FF927FF)
}

UNIQUE_DEVICE_ID_ADDRESS = 0x0BFA0590
FLASH_SIZE_DATA_REGISTER = 0x0BFA05E0
PACKAGE_DATA_REGISTER    = 0x0BFA0500

DEFAULT_JUMP_ADDRESS = 0x08000000

COMMAND_GET = 0x00
COMMAND_GETVER = 0x01
COMMAND_GETID = 0x02
COMMAND_READMEM = 0x11
COMMAND_GO = 0x21
COMMAND_WRITEMEM= 0x31
COMMAND_ERASE = 0x44
COMMAND_WRITEPROTECT = 0x63
COMMAND_WRITEUNPROTECT = 0x73
COMMAND_READOUTPROTECT = 0x82
COMMAND_READOUTUNPROTECT = 0x92
COMMAND_READOUTUNPROTECT_ANSWER = 0x111 #for some reason this is where we get the answer ...

#Command supported by STM32L5 FDCAN Bootloader 
command_name = {
    COMMAND_GET:"Get",
    COMMAND_GETVER:"Get Version & Read Protection Status",
    COMMAND_GETID:"Get ID",
    COMMAND_READMEM:"Read Memory",
    COMMAND_GO:"Go",
    COMMAND_WRITEMEM:"Write Memory",
    COMMAND_ERASE:"Erase",
    COMMAND_WRITEPROTECT:"Write Protect",
    COMMAND_WRITEUNPROTECT:"Write Unprotect",
    COMMAND_READOUTPROTECT:"Readout Protect",
    COMMAND_READOUTUNPROTECT:"Readout Unprotect"
}

LOG_OUTPUT  = 0
LOG_DEBUG   = 1
LOG_WARNING = 2
LOG_ERROR   = 3      
LOG_DATA    = 4
def log(txt, typ=LOG_OUTPUT,end=None):
    if(typ == LOG_DEBUG):
        txt = click.style(txt, fg='blue')
    if(typ == LOG_WARNING):
        txt = click.style(txt, fg='yellow')
    if(typ == LOG_ERROR):
        txt = click.style(txt, fg='red')
    if(typ == LOG_DATA):
        txt = click.style(txt, fg='cyan')
    if end==None:
        click.echo(txt) 
    else:
        click.echo(txt,nl=False)

#Note that this is a simplified address check
def checkAddressValidity(start,end):
    startOK = False
    endOK = False
    for ran in readableRange.keys():
        if start >= readableRange[ran][0] and start < readableRange[ran][1]:
            startOK = True
        if end > readableRange[ran][0] and end <= readableRange[ran][1]:
            endOK = True
    return startOK and endOK
    
def getTotalHexSize(ranges):
    res = 0
    for item in ranges:
        res += item[1] - item[0]
    return res

def isACK(resp,command=None):
    if command != None:
        ack = "1t{:03x}".format(command) + "179"
        if resp == ack :
            #log("Found ACK:" + resp,LOG_DEBUG)
            return True
        #log(ack + " != " + resp,LOG_DEBUG)
    else:
        #if command is unknown, no other choice than to look at data only
        if resp.endswith("179"):
            return True
    return False
        
def isNACK(resp,command=None):
    if command != None:
        if resp == "1t{:03x}".format(command) + "11f":
            return True
    else:
        #if command is unknown, no other choice than to look at data only
        if resp.endswith("11f"):
            return True
    return False

def checkDumpValid(start,stop,hexStr):
    if hexStr == None:
            log("Memory dump failed for " + hex(start) + "-" + hex(stop),LOG_ERROR)
            return False
    if len(hexStr)//2 != (stop-start):
            log("Incoherent size for memory read: expected " + str(stop-start) + " bytes but only got " + str(len(hexStr)//2),LOG_ERROR)
            return False
    return True 

def getFDCANPayloadSize(length):
    if length <= 8:
        return length
    elif length <= 12:
        return 9
    elif length <= 16:
        return 0xA
    elif length <= 20:
        return 0xB
    elif length <= 24:
        return 0xC
    elif length <= 32:
        return 0xD
    elif length <= 48:
        return 0xE    
    elif length <= 64:
        return 0xF
        
def getFDCANPadding(length):
    result = ""
    if length <= 8:
        result = ""
    elif length <= 12:
        result += (12-length)*"00"
    elif length <= 16:
        result += (16-length)*"00"
    elif length <= 20:
        result += (20-length)*"00"
    elif length <= 24:
        result += (24-length)*"00"
    elif length <= 32:
        result += (32-length)*"00"
    elif length <= 48:
        result += (48-length)*"00"   
    elif length <= 64:
        result += (64-length)*"00"
        
    return result
        

def breakIn64byteChunk(data):
    div = len(data) // 64
    mod = len(data) % 64
    for i in range(0,div):
        res = ""
        for j in range(0,64):
            res += "{:02x}".format(data[i*64 + j])
        yield res
    if mod != 0:
        res = ""
        for i in range(0,mod):    
            res += "{:02x}".format(data[div*64 + i])
        yield res
        
def breakIn8byteChunk(data):
    div = len(data) // 8
    mod = len(data) % 8
    for i in range(0,div):
        res = ""
        for j in range(0,8):
            res += "{:02x}".format(data[i*8 + j])
        yield res
    if mod != 0:
        res = ""
        for i in range(0,mod):    
            res += "{:02x}".format(data[div*8 + i])
        yield res        
        
class slRAMN():
    def __init__(self,port,ecuName):
        self.ser = serial.Serial(port,timeout=5)
        self.ecuName = ecuName
    def write(self,txt):
        self.ser.write(txt.encode())
        #time.sleep(0.1)
    def readline(self):
        while True:
            line = self.ser.read_until(b'\r').decode()
            #log("Read: " + line,LOG_DEBUG)
            if len(line) == 0:
                return None #timeout
            else:
                if not line.startswith("d"):
                    return line[:-1] # remove the trailing \r
                else:
                    log("Received error message: " + line,LOG_ERROR)
    def close(self):
        self.ser.close()
        
    #use retry to discard other messages until an ACK is found (e.g. if you are expecting late messages)
    def waitForACK(self,command,retry=False):
        while True:
            line = self.readline()
            if line == None:
                log("Timeout",LOG_ERROR)
                return False
            if isACK(line,command):
                return True
            elif isNACK(line,command):
                log("Received NACK from target",LOG_WARNING)
                return False
            elif len(line) > 0:
                if line[0] == '\a':
                    #return line only
                    log("ECU A rejected an slcan command", LOG_ERROR)
                if not retry:
                    log("Expected ACK but found something else: " + line, LOG_ERROR)
                    return False
                         
    def startBootloader(self):
        if (len(self.ecuName) != 1) or (self.ecuName not in "BCD"):
            log("Received a request for inexisting ECU",LOG_ERROR)
            return False
        self.write('p' + self.ecuName + '\r')
        #note that we might receive some old receive frames before receiving the bootloader ACK
        #hence we don't wait for the CR of the command
        if self.readline() == None:
            return False
        log("Successfully entered Bootloader of ECU " + self.ecuName,LOG_OUTPUT)
        return True
        
    def waitForCR(self):
        return True
        #TODO: remove below
        line = self.ser.read(1).decode()
        if line == None:
           log("Timeout",LOG_ERROR)
           return False
        if len(line) == 0:
           return False
        if line[0] == '\r':
           #log("received CR",LOG_DEBUG)
           return True
        return False
        
    #function to send a command and wait for an ack     
    def sendCommand(self,command,params="",dontWait=False):
        result = []
        if (len(params) % 2) != 0:
            log("Invalid HEX parameter: " + params,LOG_ERROR)
           
        payloadSize = len(params)//2
        comm = "1t{:03x}".format(command) + "{:01x}".format(getFDCANPayloadSize(payloadSize)) + params + getFDCANPadding(payloadSize) +  "\r"
        #log("Sending Command: " + comm,LOG_DEBUG)
        self.write(comm)  
        if not self.waitForCR():
            log("Command was not ACKed with CR", LOG_WARNING)
        
        if command == COMMAND_READOUTUNPROTECT:
            command = COMMAND_READOUTUNPROTECT_ANSWER #TODO: this is a quickfix, it seems the readout unprotect responds on 0x111
        
        if not dontWait:
            if self.waitForACK(command):
                #log("Command " + hex(command) + " was ACKED",LOG_DEBUG)
                return True    
            return False 
        return True
        
    #function to read data after sending a command that was ACKed
    def readUntilACK(self,command):
        result = []
        while True:
            line = self.readline()
            if line == None:
                return None     
                
            if not isACK(line,command):
               #process data here
               result.append(line[6:]) #read only data (the xx part in tiiidxx\r)
            else:
                break
        return result  
              
    def readMemory(self,address,size):
        if (size < 1 or size > 256):
            log("Invalid size (must be 1~256): " + str(size),LOG_ERROR)
    
        if self.sendCommand(COMMAND_READMEM,"{:08x}".format(address) + "{:02x}".format(size-1)):
            result = ""
            data = self.readUntilACK(COMMAND_READMEM)
            for m in data:
                result += m
            
            return result[0:size*2] #remove padding
        else:
            log("Data Read failed... Wrong adddress range or protection ON?", LOG_ERROR)
            return None
            
    def readAddressRange(self,start,end,progressBar=None):
        result = ""
        while True:
            diff = end - start
            chunk = min(diff,256)
            if chunk == 0:
                break
            mem = self.readMemory(start,chunk) 
            if mem != None:
                result += mem
                if progressBar != None:
                    progressBar.update(len(mem)//2)
            else:
                return None
            start += chunk
        return result
        
    #Use Flash page number of instead of 32bit address
    def readPage(self,page):
        start = 0x08000000 + 0x800*page
        stop = start + 0x800
        return self.readAddressRange(start,stop)       
            
    def printTargetInfo(self):
        if self.sendCommand(COMMAND_GET):
            data = self.readUntilACK(COMMAND_GET)
            if data != None:
                if len(data) < 2:
                    log("Expected at least 2 messages but only got " + str(len(data)),LOG_ERROR)
                log("Bootloader version: " + str(data[1]),LOG_DATA)
                log("Reported number of supported commands: " + str(int(data[0],16)-1),LOG_DATA)
                for i in range(2,len(data)):
                    log("command 0x" + data[i],LOG_DATA,end="")
                    val = int(data[i],16)
                    if val in command_name.keys():
                        log(" (" + command_name[val] + ")",LOG_DATA, end="")
                    log("")
        else:
            log("Failed sending command COMMAND_GET",LOG_ERROR)
        if self.sendCommand(COMMAND_GETID):
            data = self.readUntilACK(COMMAND_GETID)
            if data != None:
                log("Chip ID: " + data[0],LOG_DATA) 
        else:
            log("Failed sending command COMMAND_GETID",LOG_ERROR)      

        mem = self.readMemory(UNIQUE_DEVICE_ID_ADDRESS,4)    
        if mem != None:
            log("Unique Device ID: " + mem,LOG_DATA)
        else:
            log("Could not read Unique Device ID", LOG_ERROR)
         
        mem = self.readMemory(FLASH_SIZE_DATA_REGISTER,2)    
        if mem != None:
            log("Flash size data register: " + mem,LOG_DATA)
        else:
            log("Could not read Flash size data register", LOG_ERROR)
            
        mem = self.readMemory(PACKAGE_DATA_REGISTER,2)    
        if mem != None:
            log("Package data register: " + mem,LOG_DATA)
        else:
            log("Could not read Package data register", LOG_ERROR)

    def dumpHexFile(self,filename,areas=readableRange.values()):
        with click.progressbar(length=getTotalHexSize(areas),label="Reading program",show_pos=True) as bar:
            ih = IntelHex()
            for area in areas:
                start = area[0]
                end = area[1]
                hexStr = self.readAddressRange(start,end,progressBar = bar)
                if checkDumpValid(start,end,hexStr):
                    for offset in range(len(hexStr)//2):
                        val = int(hexStr[(offset*2):((offset*2)+2)],16)
                        ih[start+offset] = val
                ih.tofile(filename, format='hex')
              
    #note that we work directly with start and end address to avoid confusion with intel hex format   
    def binaryDump(self,area,filename):
        start = area[0]
        end = area[1]
        with click.progressbar(length=getTotalHexSize([area]),label="Dumping {:08x}-{:08x} in ".format(start,end) + filename,show_pos=True) as bar:
            hexStr = self.readAddressRange(start,end,progressBar = bar)
            if checkDumpValid(start,end,hexStr):
                with open(filename,"wb") as f:
                        for offset in range(len(hexStr)//2):
                            val = int(hexStr[(offset*2):((offset*2)+2)],16)
                            f.write(val.to_bytes(1,byteorder='big'))
                            
    def jumpTo(self,address):
        res = self.sendCommand(COMMAND_GO,params="{:08x}".format(address))
        if not res:
            log("Target refused to JUMP to address " + hex(address),LOG_WARNING)
        return res
        
    def eraseMemory(self):
        if self.sendCommand(COMMAND_ERASE,params="FFFF"):
            if self.waitForACK(COMMAND_ERASE):
                return True
                #TODO: double ACK needed ?
                if self.waitForACK(COMMAND_ERASE):
                    return True
                else:
                    log("Memory erase failed (1 ACK)",LOG_ERROR)
            else:
                log("Memory erase failed (0 ACK)",LOG_ERROR)
        log("Target refused memory erase (Write Protection might be enabled ?)", LOG_ERROR)
        return False
        
    def writeProtect(self,sectors=MEMORY_SECTORS): 
        if len(sectors) == 0:
            return True
        if self.sendCommand(COMMAND_WRITEPROTECT,params="{:02x}".format(len(sectors))):
        #if self.sendCommand(COMMAND_WRITEPROTECT,params=""):
            #Add the number of data at the beginning
            sectors = [len(sectors)] + sectors
            for data in breakIn8byteChunk(sectors):
                if self.sendCommand(COMMAND_WRITEPROTECT,params=data,dontWait=True):
                    continue
                log("memory write protect failed",LOG_ERROR)
                return False
            if self.waitForACK(COMMAND_WRITEPROTECT):
                time.sleep(1)
                if self.startBootloader():
                    return True
                log("Something when wrong after reset",LOG_ERROR)
        log("Memory write protect failed - maybe readout protection is active ?",LOG_ERROR)
        return False
        
    def writeUnprotect(self):
        if self.sendCommand(COMMAND_WRITEUNPROTECT,params="00"):
            if self.waitForACK(COMMAND_WRITEUNPROTECT):
                #write unprotect succeeded, expect reset
                time.sleep(1)
                if self.startBootloader():
                    return True
        log("Memory write unprotect failed - maybe it was already not protected ? ",LOG_ERROR)
        return False
            
    def readoutProtect(self):
        if self.sendCommand(COMMAND_READOUTPROTECT,params="00"): 
            if self.waitForACK(COMMAND_READOUTPROTECT):
                #readout protect succeeded, expect reset
                time.sleep(1)
                if self.startBootloader():
                    return True
                return True
        log("Write readout protect Failed - maybe it is already protected ?",LOG_ERROR)
        return False
                
    def readoutUnprotect(self):
        if self.sendCommand(COMMAND_READOUTUNPROTECT,params="00"): 
            if self.waitForACK(COMMAND_READOUTUNPROTECT_ANSWER):
                #readout unprotect succeeded, expect reset
                time.sleep(1)
                if self.startBootloader():
                    return True
                return True
        log("Memory readout unprotect failed - maybe it was already not protected ?",LOG_ERROR)
        return False  
        
    def isMemoryErased(self):
        mem = self.readMemory(readableRange["FLASH"][0],256) 
        if mem == None:
            log("Could not read memory, don't know if memory is erased or not",LOG_ERROR)
        elif mem == "FF"*256:
            return True
        return False
        
    #Note that microcontroller writes data 8 bytes at a time. If you send less than 8 bytes, remaining bytes will have unknown value.
    def writeMemory(self,address,data):
        if len(data) < 1 or len(data) > 256:
            log("Invalid Size for memory write. Should be between 1 and 256")
        if self.sendCommand(COMMAND_WRITEMEM,params="{:08x}".format(address) + "{:02x}".format(len(data)-1)):
            for chunk in breakIn64byteChunk(data):
                if not self.sendCommand(COMMAND_WRITEMEM,params=chunk,dontWait=True):
                    log("memory read protect failed at address: " + hex(address),LOG_ERROR)
                    log("address might be out of range or read/write protected",LOG_ERROR)
                    return False
            if self.waitForACK(COMMAND_WRITEMEM):
                return True
        return False
            
    def downloadHex(self,filename,otp=False):
        if not self.isMemoryErased():
            log("Beginning of Flash is not FF...FF, did you make sure flash memory is erased ?",LOG_WARNING)
        ih = IntelHex(filename)
        with click.progressbar(length=getTotalHexSize(ih.segments()),label="Writing program",show_pos=True) as bar:
            for seg in ih.segments():
                start = seg[0]
                offset = start
                end = seg[1]
                if not checkAddressValidity(start,end):
                    log("Addresses not in expected ranges: {:08x}-{:08x}".format(start,end),LOG_WARNING)
                    log("Skipping",LOG_WARNING)
                    continue
                if not otp:
                    otp_start = readableRange['OTP'][0]
                    otp_end = readableRange['OTP'][1]
                    if (start >= otp_start and start < otp_end) or (end >= otp_start and end < otp_end):
                        log("Addresses are overlapping with OTP range, set OTP flag to enable overwrite", LOG_ERROR)
                        return False
                    
                while offset < end:
                    #log("Writing from {:08x} to {:08x}".format(start,min(end,start+256)),LOG_DEBUG,end="\r")
                    if not self.writeMemory(offset,[ih[i] for i in range(offset,min(end,offset+256))]):
                        log("Writing failed at {:08x}".format(offset),LOG_ERROR)
                        return False
                    bar.update(min(end-offset,256))
                    offset += min(end-offset,256)
        return True
        
    def verifyDownload(self,filename):
        ih = IntelHex(filename)
        with click.progressbar(length=getTotalHexSize(ih.segments()),label="Verifying program",show_pos=True) as bar:
            for seg in ih.segments():
                start = seg[0]
                end = seg[1]
                if not checkAddressValidity(start,end):
                    log("Addresses not in expected ranges: {:08x}-{:08x}".format(start,end),LOG_WARNING)
                    log("Skipping",LOG_WARNING)
                    continue

                val = self.readAddressRange(start,end,bar)
                if val == None:
                    log("Could not read from target", LOG_ERROR)
                    return False
                    
                for i in range(0,len(val)//2):
                    fileread = ih[start+i]
                    uc = int(val[(2*i):(2*i)+2],16)
                    if fileread != uc:
                        log("\r\nFound mismatching byte at address {:08x} : {:02x} in uC, {:02x} in file. Aborting".format(start+i, uc, fileread), LOG_ERROR)
                        return False                
        return True

             
@click.command()
@click.argument('serial_port')
@click.argument('ecu_name')
@click.option('--readout-unprotect', '-ru', help='Disable flash readout protect',is_flag=True)
@click.option('--readout-protect', '-rp', help='Enable flash readout protect',is_flag=True)
@click.option('--write-unprotect', '-wu', help='Disable flash write protect',is_flag=True)
@click.option('--write-protect', '-wp', help='Enable flash write protect',is_flag=True)
@click.option('--verify', '-v', help='Verify target is programmed with input file',is_flag=True)
@click.option('--input', '-i', help='Input file',type=click.Path(exists=True))
@click.option('--output', '-o', help='Output file',type=click.Path())
@click.option('--erase-all', '-e', help='Erase all flash',is_flag=True)
@click.option('--program', '-p', help='Program target with input file',is_flag=True)
@click.option('--dump', '-d', help='Dump target memory',is_flag=True)
@click.option('--binary', '-b', help='Use binary format for dump instead of hex',is_flag=True)
@click.option('--range', '-r', type=click.Choice(['FLASH','SYSTEM','OPTIONBYTES','OTP']), multiple=True,default=['OTP','OPTIONBYTES','SYSTEM','FLASH'], help="Specify (multiple) address ranges")
@click.option('--jump', '-j', help="Jump to address at the end", type=str)
@click.option('--info', '-I', help="Print target information", is_flag=True)
@click.option('--OTP', help="Set this flag to be able to write OTP", is_flag=True)
@click.option('--reset', help="reset ECUs after programming", is_flag=True)

def canboot(serial_port,ecu_name,readout_unprotect,readout_protect,write_unprotect,write_protect,verify,input,output,erase_all, program,dump,binary,range,jump,info,otp,reset):

    if program and input == None:
        raise click.BadParameter('Need input file to program') 
          
    if dump and output != None:
        if os.path.exists(output):
            if binary and not os.path.isdir(output):
                raise click.BadParameter('Output must be a directory when dumping in binary mode') 
            if not binary and not os.path.isfile(output):
                raise click.BadParameter('Output must be a file when dumping in hex mode') 
        else:
            if binary:
                Path(output).mkdir(parents=True, exist_ok=True)
     
    if serial_port == "AUTO":
        serial_port = utils.RAMN_Utils.autoDetectRAMNPort()
     
    s = slRAMN(serial_port,ecu_name)
    errHappened = False
    
    # read version to make sure this is actually RAMN we are talking too.
    s.write("V\r")
    answer = s.readline()
    if answer is not None:
        log("ECU A reported version: {}".format(answer), LOG_OUTPUT)
    
    if not s.startBootloader():
        log("Error - make sure that external CAN adapters are disconnected, they prevent a required baudrate change",LOG_ERROR)
        errHappened = True
    
    else:
   
        try:
            option_byte = s.readMemory(0x40022042,1) 
            option_byte_integer = int(option_byte, 16)

            if (option_byte_integer >> 4)&0x1 != 0:
                log("Bank swap active! You may want to check the option bytes.", LOG_WARNING)
        except:
            log("Could not read option bytes", LOG_ERROR)
        
        if info:
            log("Asking target uC for info",LOG_OUTPUT)
            s.printTargetInfo()
        
        if readout_unprotect:
            log("Removing memory readout protect",LOG_OUTPUT)
            if s.readoutUnprotect():
                log("Memory readout protection successfully removed",LOG_OUTPUT)

        if write_unprotect:
            log("Removing memory write protection",LOG_OUTPUT)
            if s.writeUnprotect():
                log("Memory write protection successfully removed",LOG_OUTPUT)
                    
        if dump:
            log("Dumping memory",LOG_OUTPUT)
            if output == None:
                for area in range:
                    log("Dumping area : " + area)
                    data = None
                    with click.progressbar(length=getTotalHexSize([readableRange[area]]),label="Dumping in progress",show_pos=True) as bar:
                        data = s.readAddressRange(readableRange[area][0],readableRange[area][1],progressBar=bar)
                    if data != None:
                        log("*************************************")
                        if binary:
                            log((bytearray.fromhex(data).decode('ascii','replace')),LOG_DATA)
                        else:  
                            log(data,LOG_DATA)
                        log("*************************************")
                    else:
                        log("Dump failed", LOG_ERROR)
            else:
                if binary: 
                    for area in range:
                        if s.binaryDump(readableRange[area], output + "/" + area + ".bin"):
                            log("Binary memory dump successful",LOG_OUTPUT)
                else:
                    if s.dumpHexFile(output,areas=[readableRange[r] for r in range]):
                        log("Hex memory dump successful",LOG_OUTPUT)

        if erase_all:
            log("Erasing memory",LOG_OUTPUT)
            if s.eraseMemory():
                log("Memory erase successful",LOG_OUTPUT)      
            else:
                log("Fail",LOG_ERROR)
                errHappened = True

        if program:
            log("Starting writing memory",LOG_OUTPUT)
            if s.downloadHex(input,otp=otp):
                log("Memory writing successful")
            else:
                log("Fail",LOG_ERROR)
                errHappened = True
        # log("Memory write successful",LOG_OUTPUT)
     
        if write_protect:        
            log("Protecting memory write",LOG_OUTPUT)
            if s.writeProtect():
                log("Memory write protection successful",LOG_OUTPUT)    

        if verify:
            if s.verifyDownload(input):
                log("Target is successfully programmed")    

        if readout_protect:
            log("Protecting memory readout",LOG_OUTPUT)
            if s.readoutProtect():
                log("Memory readout protection successful",LOG_OUTPUT)  
            else:
                log("Fail",LOG_ERROR)
                errHappened = True
        
        if jump != None:
            log("Jumping to Application")
            addr = DEFAULT_JUMP_ADDRESS
            try:
                addr = int(jump,0)
                if addr < 0:
                    addr = DEFAULT_JUMP_ADDRESS
                    raise 
            except:
                log('Invalid jump address: ' + str(jump) + '. Jumping to 0x{:08x} instead'.format(addr),LOG_WARNING) 

            if s.jumpTo(addr):
                log("Target accepted to jump to address 0x{:08x}".format(addr)) 
                
        if reset:     
            log("Resetting ECUs")
            if readout_protect or write_protect or readout_unprotect or write_unprotect:
                time.sleep(3) # Wait a bit, as these commands may take time to execute
            s.write('n' + '\r')
            if s.readline() == None:
                log("ECU did not answer reset command")
        
    s.close()
    if errHappened:
        log("Errors happened",LOG_ERROR)
        click.pause()
    log("Done", LOG_OUTPUT)
    
if __name__ == '__main__':
    canboot()
