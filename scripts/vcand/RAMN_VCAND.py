#!/usr/bin/env python

# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#Script to emulate a virtual can-fd bus with socketCAN and RAMN. Can forward serial commands between other serial ports.

import serial
import threading
import queue
import can

import sys
sys.path.append("..")
from utils.RAMN_Utils import *

# -------- SETTINGS --------------------------------------------------

ramn_port      = RAMN_Utils.VCAND_HARDWARE_PORT     #Name of RAMN Port to fork. Can be detected automatically, or set manually, e.g. '/dev/ttyACM0'
pts_name_ch1   = RAMN_Utils.SERIAL_FORWARD_TARGET   #Name of serial port on which to forward commands. Note that this is not the port on which is other application is reading - but the socat counterpart. For example, if socat generated /dev/pts/1 and /dev/pts/2, you can write "/dev/pts/1" here, and your application should use "/dev/pts/2" 
pts_name_ch2   = ''                                 #(Leave empty if unused) Name of serial port on which to forward commands. See above.
can_name       = RAMN_Utils.CAN_NAME                #Name of virtual CAN Interface on which to forward CAN messages.

# -------- CODE --------------------------------------------------

CHANNEL_HARDWARE_RAMN       = 0
CHANNEL_VIRTUAL_SERIAL_CH1  = 1 
CHANNEL_VIRTUAL_SERIAL_CH2  = 2 
CHANNEL_VIRTUAL_CAN         = 3

TXQ_MAX_ITEM                = 1000

rxq                    = queue.Queue()
txq_hardware_ramn      = queue.Queue()
txq_virtual_serial_ch1 = queue.Queue()
txq_virtual_serial_ch2 = queue.Queue()
txq_virtual_CAN        = queue.Queue()


#Returns the size of a payload depending on the DLC field of a CAN message
def getCANFDPayload(size):
    if size <= 8:
        return size
    if size == 12:
        return 9
    elif size == 16:
        return 0xA
    elif size == 20:
        return 0xB
    elif size == 24:
        return 0xC
    elif size == 32:
        return 0xD
    elif size == 48:
        return 0xE
    elif size == 0x64:
        return 0xF
    else:
        print("Got Invalid CAN-FD Payload Size")
        
#Convert a CAN Message to a serial message        
def getSerialFromMsg(msg):
    cmd = ''
    if msg.is_fd:
        if msg.bitrate_switch:
            cmd += '1'
        else:
            cmd += '0'
    if msg.is_extended_id:
        if msg.is_remote_frame:
            cmd += 'R'
        else:
            cmd += 'T'
        cmd += "{:08x}".format(msg.arbitration_id)
    else:
        if msg.is_remote_frame:
            cmd += 'r'
        else:
            cmd += 't'
        cmd += "{:03x}".format(msg.arbitration_id)
     
    cmd += "{:1x}".format(getCANFDPayload(len(msg.data)))
    if not msg.is_remote_frame:
        for d in msg.data:
            cmd += "{:02x}".format(d)
    if msg.error_state_indicator:
        cmd += "i"
    return cmd.encode() + b'\r'
    
#Convert a serial message to a CAN message
def getMessageFromSerial(line):
    rmsg = None
    bitrate_switch=False
    is_extended_id = False
    error_state_indicator=False
    is_fd = False
    data=b''
    if len(line) > 0:
        if line[0] == '0':
            is_fd = True
            bitrate_switch = False
            line = line[1:]
        elif line[0] == '1':
            is_fd = True
            bitrate_switch = True
            line = line[1:]
            
    if len(line) > 0:    
        if line[0] == 't':
            if len(line) >= 5:
                canid = int(line[1:4],16)
                if len(line) >= 7:
                    if line[-1] == 'i':
                        error_state_indicator=True
                        line = line[:-1]
                    data = bytes.fromhex(line[5:])
                rmsg = can.Message(arbitration_id=canid,data=data,is_extended_id=is_extended_id, is_fd=is_fd, bitrate_switch=bitrate_switch, is_remote_frame=False, error_state_indicator=error_state_indicator)
        elif  line[0] == 'T':
            if len(line) >= 10:
                canid = int(line[1:9],16)
                if len(line) >= 12:
                    if line[-1] == 'i':
                        error_state_indicator=True
                        line = line[:-1]
                    data = bytes.fromhex(line[10:])
                is_extended_id = True
                rmsg = can.Message(arbitration_id=canid,data=data,is_extended_id=is_extended_id, is_fd=is_fd, bitrate_switch=bitrate_switch, is_remote_frame=False, error_state_indicator=error_state_indicator)
        if line[0] == 'r':
            if len(line) >= 4:
                canid = int(line[1:4],16)
                is_remote_frame=True
                if line[-1] == 'i':
                    error_state_indicator=True
                rmsg = can.Message(arbitration_id=canid,data=data,is_extended_id=is_extended_id, is_fd=is_fd, bitrate_switch=bitrate_switch, is_remote_frame=True, error_state_indicator=error_state_indicator)
        elif  line[0] == 'R':
            if len(line) >= 9:
                canid = int(line[1:9],16)
                is_remote_frame=True
                is_extended_id = True
                if line[-1] == 'i':
                        error_state_indicator=True
                rmsg = can.Message(arbitration_id=canid,data=data,is_extended_id=is_extended_id, is_fd=is_fd, bitrate_switch=bitrate_switch, is_remote_frame=True, error_state_indicator=error_state_indicator)  
    return rmsg

#Call back for CAN message received
CHANNEL_VIRTUAL_CAN_BYTES = bytes([CHANNEL_VIRTUAL_CAN])
def onNewCANMessage(msg):
    cmd = getSerialFromMsg(msg)
    rxq.put(CHANNEL_VIRTUAL_CAN_BYTES + cmd)
    
#Thread that reads for a serial port, and queue full commands.
def receiveSerialThread(ser,r,channel):    
    while True:
        if ser.in_waiting > 0:
            buffer = ser.read_until(b'\r') 
            if len(buffer) > 0:
                r.put(channel + buffer)

#Thread that empty queues to write their content to a specified serial port   
def sendSerialThread(ser,t):
    while True:
        item = t.get()
        ser.write(item)
        t.task_done()

#Thread that empty queues to write their content to a specified can bus  
def sendCANThread(bus,t):
    while True:
        try:
            item = t.get()
            bus.send(item)
            t.task_done()
        except can.CanError:
            print("Failed to forward RAMN CAN Message to virtual CAN: " + str(line))
  
#Write to queue, with size check to avoid queues getting stuck because counterpart is not reading.
def addItemToQueue(item,t):
    if t.qsize() <= TXQ_MAX_ITEM:
        t.put(item)
    else:
        #print("queue full (size:{}): {} ".format(t.qsize(),str(t)))
        pass
  
try:
    can_bus = can.interface.Bus(bustype='socketcan', channel=can_name, fd=True)
    notifier = can.Notifier(can_bus,[onNewCANMessage])
except Exception as e:
    print("Could Not open CAN bus :" + str(e))
    can_bus = None
     
try:
    if len(pts_name_ch1) > 0:
        virt_ch1_ser = serial.Serial(pts_name_ch1, rtscts=True,dsrdtr=True)
    else:
        virt_ch1_ser = None
except Exception as e:
    print("Could Not open Serial port :" + str(e))
    virt_ch1_ser = None
    
try:
    if len(pts_name_ch2) > 0:
        virt_ch2_ser = serial.Serial(pts_name_ch2, rtscts=True,dsrdtr=True)
    else:
        virt_ch2_ser = None
except Exception as e:
    print("Could Not open Serial port :" + str(e))
    virt_ch2_ser = None
    
try:
    #open serial port
    ramn_ser = serial.Serial(ramn_port)
    #Open slcan
    ramn_ser.write(b'O\r')
    #Start sending/receing threads
    p_hardware_rx = threading.Thread(target=receiveSerialThread,      args=(ramn_ser,rxq,bytes([CHANNEL_HARDWARE_RAMN])))
    p_hardware_rx.start()
    p_hardware_tx = threading.Thread(target=sendSerialThread,         args=(ramn_ser,txq_hardware_ramn))
    p_hardware_tx.start()
except:
    print("Could not open RAMN serial port at {}. Permission Issue ?".format(ramn_port))
    sys.exit(1)

if virt_ch1_ser != None:
    #Start sending/receing threads
    p_virtual_ch1_rx  = threading.Thread(target=receiveSerialThread,  args=(virt_ch1_ser,rxq,bytes([CHANNEL_VIRTUAL_SERIAL_CH1])))
    p_virtual_ch1_rx.start()
    p_virtual_ch1_tx  = threading.Thread(target=sendSerialThread,     args=(virt_ch1_ser,txq_virtual_serial_ch1))
    p_virtual_ch1_tx.start()
    
if virt_ch2_ser != None:
    #Start sending/receing threads
    p_virtual_ch2_rx  = threading.Thread(target=receiveSerialThread,  args=(virt_ch2_ser,rxq,bytes([CHANNEL_VIRTUAL_SERIAL_CH2])))
    p_virtual_ch2_rx.start()
    p_virtual_ch2_tx  = threading.Thread(target=sendSerialThread,     args=(virt_ch2_ser,txq_virtual_serial_ch2))
    p_virtual_ch2_tx.start()
    
if can_bus != None:
    #Only TX, RX already handled by notifier
    p_virtual_can_tx  = threading.Thread(target=sendCANThread,        args=(can_bus,txq_virtual_CAN))
    p_virtual_can_tx.start()
    
print("All Threads started")

while True:
    item = rxq.get()
    if len(item) > 0:
        if (item[0] == CHANNEL_HARDWARE_RAMN):
            #print("RAMN:" + item[1:].decode())
            if virt_ch1_ser != None: addItemToQueue(item[1:],txq_virtual_serial_ch1)
            if virt_ch2_ser != None: addItemToQueue(item[1:],txq_virtual_serial_ch2)
            if can_bus      != None: 
                msg = getMessageFromSerial(item[1:-1].decode())
                if msg != None:
                    addItemToQueue(msg,txq_virtual_CAN)
        elif (item[0] == CHANNEL_VIRTUAL_CAN):
            #print("CAN :" + item[1:].decode())
            if ramn_ser     != None: addItemToQueue(item[1:],txq_hardware_ramn)
            if virt_ch1_ser != None: addItemToQueue(item[1:],txq_virtual_serial_ch1)
            if virt_ch2_ser != None: addItemToQueue(item[1:],txq_virtual_serial_ch2)
        elif (item[0] == CHANNEL_VIRTUAL_SERIAL_CH1):
            #print("PTS1:" + item[1:].decode())
            if ramn_ser     != None: addItemToQueue(item[1:],txq_hardware_ramn)
            if virt_ch2_ser != None: addItemToQueue(item[1:],txq_virtual_serial_ch2)
            if can_bus      != None: 
                msg = getMessageFromSerial(item[1:-1].decode())
                if msg != None:
                    addItemToQueue(msg,txq_virtual_CAN)
        elif (item[0] == CHANNEL_VIRTUAL_SERIAL_CH2):
            #print("PTS2:" + item[1:].decode())
            if ramn_ser     != None: addItemToQueue(item[1:],txq_hardware_ramn)
            if virt_ch1_ser != None: addItemToQueue(item[1:],txq_virtual_serial_ch1)
            if can_bus      != None: 
                msg = getMessageFromSerial(item[1:-1].decode())
                if msg != None:
                    addItemToQueue(msg,txq_virtual_CAN)
        else: 
            print("ERROR: Received Command from unknown Channel")
    else:
        print("ERROR: Received Empty data")
    rxq.task_done()
