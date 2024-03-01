#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Collection of functions to make it more intuitive to access UDS features of RAMNs ECU over python.

from utils.RAMN_ISOTP_Handler   import *

#Class that handles UDS communication for one ECU
class RAMN_UDS_Handler:
    def __init__(self,ramn,tp,txid,rxid,name="Unknown",isUSB=False):
        self.uds = UDSAnalyzer()
        self.ramn = ramn
        self.txid=txid
        self.rxid=rxid
        self.tp = tp 
        self.ramn = ramn
        self.name = name
        self.isUSB = isUSB
    
    #Send a Raw payload over UDS
    def sendRawData(self,toSend,timeout=RAMN_Utils.DEFAULT_TIMEOUT,recvAnswer=True):
        if isinstance(toSend,list):
            toSend = bytes(toSend)
        if len(toSend) > 0xFFF:
            log("Requested a payload which size is too big: {}".format(len(toSend)),LOG_ERROR)
            return None
                
        log("SEND:" + self.uds.display(list(toSend)),LOG_DEBUG)
        if self.isUSB:
            payload = self.ramn.sendUDSCommandUSB(toSend,timeout=timeout,recvAnswer=recvAnswer)
            if recvAnswer:
                if payload != None:
                    log("RECV:" + self.uds.display(list(payload)),LOG_DEBUG)
                    return payload
        else:
            self.tp.sendFrame(toSend, self.txid, self.rxid)    
            tstart = time.time()
            while timeout == 0 or (time.time() - tstart < timeout):
                payload = self.tp.update(recvAnswer,timeout)
                if recvAnswer:
                    if payload != None:
                        log("RECV:" + self.uds.display(list(payload)),LOG_DEBUG)
                        return payload
                else:
                    if self.tp.isTxOver():
                        return []
            return None
     
    #Send a UDS command with specified parameters
    def sendCommand(self, commandID, params=[],timeout=RAMN_Utils.DEFAULT_TIMEOUT, recvAnswer=True):
        r = self.sendRawData([commandID] + params,timeout, recvAnswer)
        if recvAnswer:
            if r == None:
                log("Timeout",LOG_ERROR)
                return 'b'
            elif len(r) == 0:
                log("Received Empty Data",LOG_ERROR)
                return b''
            elif r[0] == commandID + 0x40:
                return list(r[1:])
            elif r[0] == 0x7F: #negative command
                log("Received Negative RESPONSE: " + hex(r[2]) + " (" + UDSAnalyzer.ERROR_STRINGS[r[2]] + ") for COMMAND " + hex(r[1]) + " (" + 
UDSAnalyzer.UDS_SID_STRINGS[r[1]] + ")",LOG_ERROR)
            else: log("Unexpected UDS RESPONSE: " + hex(r[0]),LOG_ERROR)
            return None
        return []
        
    #Sends a tester present command and wait for answer
    def testerPresent(self):
        if self.sendCommand(0x3E,[0x00]) != None : log("ECU responsed to Tester Present (TX:0x{:03x} RX:0x{:03x})".format(self.txid,self.rxid),LOG_DATA)
        else: log("ECU did not respond to Tester Present",LOG_ERROR)
    
    #Goes to specified diagnostic session
    def diagnosticSessionControl(self, session):
        return self.sendCommand(0x10,[session])
        
    #perform "security access" (currently implemented as a dummy, non-secure Seed XOR with static "key")
    def performSecurityAccess(self,level):
        l = self.sendCommand(0x27,[level])
        if l == None:
            return None
        l[0] += 1
        l[1] ^= 0x12
        l[2] ^= 0x34
        l[3] ^= 0x56
        l[4] ^= 0x78
        return self.sendCommand(0x27,l)     
    
    def readMemoryByAddress(self,address,size):
        if checkAddressValidity(address,address+size):
            return self.sendCommand(0x23, [0x24] + int32ToList(address) + int16ToList(size))
        else:
            log("Tried to read a memory outside of readable range", LOG_ERROR)
        
    def routineControl(self,action,subroutine, parameters=[], recvAnswer=True):
        return self.sendCommand(0x31, [action] + int16ToList(subroutine) + parameters,recvAnswer=recvAnswer)[3:]
    
    #Request that ECU stops sending periodic messages
    def disablePeriodicSending(self):
        return self.routineControl(0x01,0x0200) 
        
    #Request that ECU resumes sending periodic messages    
    def enablePeriodicSending(self):
        return self.routineControl(0x02,0x0200) 
    
    def requestDownload(self,address,size):
        return self.sendCommand(0x34, [0x00, 0x44] + int32ToList(address) + int32ToList(size))
        
    def requestUpload(self,address,size):
        blockSize =  self.sendCommand(0x35, [0x00, 0x44] + int32ToList(address) + int32ToList(size))   
        if blockSize != None:
            return listToInt16(blockSize[1:])
        return None
    
    def transferDataSingle(self, seq, data=[]):
        return self.sendCommand(0x36, [seq] + data)
        
    def requestTransferExit(self):
        return self.sendCommand(0x37)
        
    def resetECU(self):
        return self.sendCommand(0x11, [0x01],recvAnswer=False)
    
    #perform as many "Transfer Data" commands as necessary, for UPLOAD (ECU->Python Script)
    def transferDataUpload(self,expectedSize):
        seq = 1
        result = []
        while len(result) != expectedSize:
            r = self.transferDataSingle(seq)
            if r == None:
                return None
            if seq != r[0]:
                log("Transfer Data Sequence Error, expected {:02x}, got {:02x}".format(seq,r[0]),LOG_ERROR)
                return None
            else:
                result += r[1:]
            seq =  (seq + 1)& 0xFF
        return result
    
    #perform as many "Transfer Data" commands as necessary, for DOWNLOAD (Python Script->ECU)    
    def transferDataDownload(self,data,chunkSize):
        seq = 1
        for item in breakInUDSChunk(data,chunkSize):
            res = self.transferDataSingle(seq, item)
            seq =  (seq + 1)& 0xFF        
            if res == None:
                return None
        return []
        
    def readDataByIdentifier(self,identifier):
        r =  self.sendCommand(0x22, int16ToList(identifier&0xFFFF))
        if r != None:
            return r[2:]
        return None
   
    #NO CHECK PERFORMED - MAY CRASH THE ECU
    def writeMemoryByAddressUINT8(self,address,val):
        return self.sendCommand(0x3D, [0x42] + int32ToList(address) + [0x00, 0x01] + [val&0xFF])     
    
    #NO CHECK PERFORMED - MAY CRASH THE ECU
    def writeMemoryByAddressUINT16(self,address,val):
        return self.sendCommand(0x3D, [0x42] + int32ToList(address) + [0x00, 0x02] + int16ToList(val)) 
    
    #NO CHECK PERFORMED - MAY CRASH THE ECU    
    def writeMemoryByAddressUINT32(self,address,val):
        return self.sendCommand(0x3D, [0x42] + int32ToList(address) + [0x00, 0x04] + int32ToList(val)) 
        
    #NO CHECK PERFORMED - MAY CRASH THE ECU    
    def writeMemoryByAddressArray(self,address,arr):
        if len(arr) > 0xFF7:
            log("Cannot write more than 0xFF7 bytes in memory at once", LOG_ERROR)
        else:
            return self.sendCommand(0x3D, [0x42] + int32ToList(address) + int16ToList(len(arr)))     
    
    def writeDataByIdentifier(self,identifier,val):
        return self.sendCommand(0x2E, int16ToList(identifier&0xFFFF) + val) 
        
    def writeDataByIdentifier32(self,identifier,val):
        return self.sendCommand(0x2E,int16ToList(identifier&0xFFFF) + int32ToList(val)) 
     
    def readVIN(self):
        v =  self.readDataByIdentifier(0xF190)
        if v != None:
            return (''.join(chr(i) for i in v)).strip('\x00')
        return None
        
    def writeVIN(self,vin):
        return self.writeDataByIdentifier(0xF190, [ord(i) for i in vin.ljust(20, '\x00')]) 
        
    def readCompileTime(self):
        return ''.join(chr(i) for i in self.readDataByIdentifier(0xF184))
        
    def readECUSerial(self):
        r = self.readDataByIdentifier(0xF18C)
        serial = ''
        if r != None:
             serial += 'WAFER XY ' + ''.join("{:02x}".format(i) for i in r[0:4])
             serial += ' WAFER NUMBER ' +  ''.join("{:02x}".format(r[4]))
             serial += ' LOT ' + ''.join(chr(i) for i in r[5:])
        return serial
    
    #Perform basic operations required before most UDS feaetures
    def setUpForProgramming(self):
        self.testerPresent()                #Optional
        self.diagnosticSessionControl(0x02) #Request programming Session
        self.performSecurityAccess(0x01)    #Perform Security Access
        self.disablePeriodicSending()       #Optional
        
    def getCRC32(self,address,size):
        crc = self.routineControl(0x01,0x0206,parameters=int32ToList(address)+int32ToList(size))
        if crc != None:
            if len(crc) >= 4:
                return listToInt32(crc[:4])
        return None
    
    #prints info that can be obtained over US in a human readable manner
    def printInfo(self):  
        log("Reading ECU {} Info".format(self.name),LOG_OUTPUT)
        compileTime = self.readCompileTime()
        if compileTime != None:
            log("ECU Compile Time: " + str(compileTime),LOG_DATA)
        serial = self.readECUSerial()
        if serial != None:
            log("ECU Serial Number: " + str(serial),LOG_DATA)
            
        vin = self.readVIN()
        if vin != None:
            log("VIN: " + str(vin),LOG_DATA)
        else:
            log("No VIN recorded")
        
        crc = self.getCRC32(0x08000000, 0x3E000)
        if crc != None:
            log("Current Bank CRC (Without EEPROM) : {:08x}".format(crc),LOG_DATA)
        else:
            log("Could not retrieve CRC", LOG_WARNING)
        
        crc = self.getCRC32(0x08000000, 0x40000)
        if crc != None:
            log("Current Bank CRC (With EEPROM)    : {:08x}".format(crc),LOG_DATA)
        else:
            log("Could not retrieve CRC", LOG_WARNING)        

    
    #Reprograms ECU with specified firmware
    def reprogram(self,filename, erase=True, copyEEPROM=True, swap=True):
        # Routine Control, Erase Memory (FF00)
        if erase:
            log("Requesting Erase of alternative firmware...",LOG_DATA)
            self.eraseAlternativeBank()
       
        addr, size, data = getDownloadData(filename)
        if (size > 0x3E000):
            log("Firmware too big to fit in a single bank - use hardware bootloader or programmer",LOG_ERROR)
            return None
        
        # Request Download
        log("Requesting Download at address {:08x} with size {:08x} ({:d} bytes)".format(addr,size,size),LOG_DATA)
        if self.requestDownload(addr,size) != None:
        
            # Transfer Data
            log("Sending firmware...".format(addr,size,size),LOG_DATA)
            self.transferDataDownload(data,chunkSize=0xFF8)
        
            # Request Transfer Exit
            if self.requestTransferExit() != None:
            
                if copyEEPROM:
                    log("Requesting Copy of EEPROM", LOG_DATA)
                    self.copyEEPROMtoAlternative() #request copy of EEPROM
                    
                #Routine Control, validate application (FF01)
                if swap:
                    log("Requesting Swap of Memory Banks", LOG_DATA)
                    if self.routineControl(0x01,0xFF01, recvAnswer=False) != None: #Request SWAP of banks
                        time.sleep(1)
                        return True
                else:
                    return True
        return False
   
    #function to test ISO-TP Link. Return True if data was successfully echoed        
    # Direction 0 for echo of provided data             (DOWNLINK-UPLINK test)
    # Direction 1 for received ACK only                 (DOWNLINK test)
    # Direction 2 for sending full content of TX buffer (UPLINK test)
    def echo(self, data=[1,2,3,4], direction=0):
        log("Sending Echo request of size 0x{:x} ({} bytes) type {}".format(len(data),len(data),direction),LOG_DATA)
        if direction == 0:
            res = self.routineControl(0x01,0x0203,parameters=data)
        elif direction == 1:
            res = self.routineControl(0x01,0x0204,parameters=data)
        elif direction == 2:
            res = self.routineControl(0x01,0x0205,parameters=data)
        if res != None:
            #Verify Data content only for bidirectional tests
            if direction == 0:
                if (len(res) > 0) and (len(data) == len(res)):
                    return all(res[i] == data[i] for i in range(len(data)))
            elif direction == 1:
                return True #Getting a response is enough for downlink
            elif direction == 2:
                if (len(res) > 0) and (len(res) == listToInt16(data)):
                    return all(res[i] == (i&0xff) for i in range(len(data)))
            else:
                return True
        return False
    
    #Dumps the firmware of ECU using requestDownload
    def dumpFirmware(self,addr, size):
        dump = None
        # Request Download
        blocksize = self.requestUpload(addr,size)
        if blocksize != None: 
            log("Requested Upload, got a maximum blocksize of {:04x}".format(blocksize),LOG_DATA)

            # Transfer Data
            log("Receiving firmware...".format(addr,size,size),LOG_DATA)
            dump = self.transferDataUpload(size)
            
            if dump != None:
                # Request Transfer Exit
                if self.requestTransferExit() != None:
                    if len(dump) != size:
                        log("Got Unexpected Size from dump. Expected {:x}, got {:x}".format(size,len(data)),LOG_ERROR)
                        dump = None
        return dump
    
    #Dumps a specified area of memory using ReadMemoryByAddress
    def dumpArea(self,start,end,blockSize=0xFF0):
        dump = []
        addr = start 
        size = end - start
        index = 0
        while (index < size):
            reqSize = min(blockSize,(size-index))
            r = self.readMemoryByAddress(addr+index,reqSize)
            if r != None:
                dump += r
                index += reqSize
            else:
                log("Read Memory By Address Failed, Could not read Data", LOG_ERROR)
                break
        if len(dump) != size:
            log("Read Memory By Address Failed, Did not receive enough bytes", LOG_ERROR)
        return dump
        
    #Dump the area of memory in charge of EEPROM emulation
    def dumpEEPROM(self,blockSize=0xFF0):
        return self.dumpArea(0x0803E000,0x08040000)
    
    #Erase the current EEPROM emulation layer
    def eraseCurrentEEPROM(self):
        return self.routineControl(0x01,0x0201) 
        
    #Copy active EEPROM to inactive EEPROM
    def copyEEPROMtoAlternative(self):
        return self.routineControl(0x01,0x0202) 
     
    #Erase the alternative firmware. Required before reprogramming.
    def eraseAlternativeBank(self):
        return self.routineControl(0x01,0xFF00)
       
    #Load a Chip8 game over UDS
    def loadChip8(self, game_path):
        bc = os.path.getsize(game_path)
        with open(game_path, 'rb') as f:
            data = [bc >> 8, bc&0xFF]
            while 1:
                b = f.read(1)
                if not b:
                    break
                data += [ord(b)]

        self.sendCommand(0x42,data)
        
    #Displays an image (236 x 195 pixels)
    def displayImage(self, image_path):
        from PIL import Image
        image = Image.open(image_path).convert("RGB")
        #size should be 236 195 to fit on RAMN canvas.
        STARTX = 2
        STARTY = 2
        WIDTH = image.size[0]
        HEIGHT = image.size[1]
        STEP = 8 #8 max
        log("Sending image with size {} x {}".format(WIDTH, HEIGHT), LOG_DEBUG)

        payloads = []
        for i in range(0, HEIGHT + 1, STEP):
            Y = i
            YLEN = (min(HEIGHT-i,STEP))
            payload = [STARTX, STARTY+Y, WIDTH, YLEN]

            for y in range(YLEN):
                for x in range(WIDTH):
                    r,g,b = image.getpixel((x,Y+y))

                    R = r >> 3
                    G = g >> 2
                    B = b >> 3

                    rgb565 = (R << 11) | (G << 5) | B
             
                    payload += [(rgb565)&0xFF]
                    payload += [(rgb565 >> 8)&0xFF]

            payloads.append(payload)
        for payload in payloads:
            self.sendCommand(0x41,payload)
    
    #Verify that firmware is correctly flashed
    def verify(self,filename):
        addr, size, data = getDownloadData(filename)
        log("Verifying area at address {:08x} with size {:08x}".format(addr,size),LOG_DEBUG)
        dump = self.dumpFirmware(addr,size)
        if dump != None:
            result = True
            for i in range(len(data)):
                if data[i] != dump[i]:
                    result = False
            if result:
                return True
        else:
            log("Verify Failed: No Data", LOG_ERROR)
        return False
    
    #Puts the ECU back in operational mode
    def close(self,reset=False):
        if not self.isUSB:
            self.enablePeriodicSending()
        if(reset):
            self.resetECU()
        else:
            self.diagnosticSessionControl(0x01) #Go back to standard session