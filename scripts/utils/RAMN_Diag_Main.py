#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This module contains general functions to access diagnostic features

from utils.RAMN_UDS_Handler import *
from utils.RAMN_KWP_Handler import *
from utils.RAMN_XCP_Handler import *

#UDS IDs for ECU A
UDS_ECUA_RX = 0x7e0
UDS_ECUA_TX = 0x7e8

#UDS IDs for ECU B
UDS_ECUB_RX = 0x7e1
UDS_ECUB_TX = 0x7e9

#UDS IDs for ECU C
UDS_ECUC_RX = 0x7e2
UDS_ECUC_TX = 0x7eA

#UDS IDs for ECU D
UDS_ECUD_RX = 0x7e3
UDS_ECUD_TX = 0x7eB  


#KWP IDs for ECU B
KWP_ECUB_RX = 0x7e5
KWP_ECUB_TX = 0x7eD

#KWP IDs for ECU C
KWP_ECUC_RX = 0x7e6
KWP_ECUC_TX = 0x7eE

#KWP IDs for ECU D
KWP_ECUD_RX = 0x7e7
KWP_ECUD_TX = 0x7eF  

#XCP IDs for ECU B
XCP_ECUB_RX = 0x552
XCP_ECUB_TX = 0x553

#XCP IDs for ECU C
XCP_ECUC_RX = 0x554
XCP_ECUC_TX = 0x555

#XCP IDs for ECU D
XCP_ECUD_RX = 0x556
XCP_ECUD_TX = 0x557 

#returns handlers to interact with ECUs over ISO-TP
def getRAMNHandlers(port,filterID=b'M7e0', filterMask=b'm7F0',filterIDextended=b'M00000000', filterMaskExtended=b'm7FFFFFFF'):
    ramn = RAMN_USB_Handler(port)    
    ramn.open(autoOpen=False)
    
    #Set CAN filter to only receive Diagnostic messages
    ramn.sendCommand(b'W2')   #filter value/mask type

    ramn.sendCommand(filterID)#Filter ID
    ramn.sendCommand(filterMask)#Filter mask  
    ramn.sendCommand(filterIDextended)#Filter ID
    ramn.sendCommand(filterMaskExtended)#Filter mask  
    
    #Open port and empty current buffer
    ramn.sendCommand(b'O') 
    time.sleep(0.1)
    ramn.flush()
    
    #Create ISO-TP communication handler
    tp = RAMN_ISOTP_Handler(ramn,{UDS_ECUB_TX:UDS_ECUB_RX, UDS_ECUC_TX:UDS_ECUC_RX, UDS_ECUD_TX:UDS_ECUD_RX,KWP_ECUB_TX:KWP_ECUB_RX, KWP_ECUC_TX:KWP_ECUC_RX, KWP_ECUD_TX:KWP_ECUD_RX})  

    return ramn,tp
    
#returns handlers to access UDS diagnostics for all ECUs of RAMN
def getECUHandlersUDS(ramn,tp):
    ECUA = RAMN_UDS_Handler(ramn,tp,txid=UDS_ECUA_RX,rxid=UDS_ECUA_TX,name="A",isUSB=True)
    ECUB = RAMN_UDS_Handler(ramn,tp,txid=UDS_ECUB_RX,rxid=UDS_ECUB_TX,name="B")
    ECUC = RAMN_UDS_Handler(ramn,tp,txid=UDS_ECUC_RX,rxid=UDS_ECUC_TX,name="C")
    ECUD = RAMN_UDS_Handler(ramn,tp,txid=UDS_ECUD_RX,rxid=UDS_ECUD_TX,name="D")
    return ECUA, ECUB, ECUC, ECUD
    
#returns handlers to access KWP diagnostics for all ECUs of RAMN
def getECUHandlersKWP(ramn,tp):
    ECUB = RAMN_KWP_Handler(ramn,tp,txid=KWP_ECUB_RX,rxid=KWP_ECUB_TX,name="B")
    ECUC = RAMN_KWP_Handler(ramn,tp,txid=KWP_ECUC_RX,rxid=KWP_ECUC_TX,name="C")
    ECUD = RAMN_KWP_Handler(ramn,tp,txid=KWP_ECUD_RX,rxid=KWP_ECUD_TX,name="D")
    return None, ECUB, ECUC, ECUD    
    
#returns handlers to access KWP diagnostics for all ECUs of RAMN
def getECUHandlersXCP(ramn):
    ECUB = RAMN_XCP_Handler(ramn,txid=XCP_ECUB_RX,rxid=XCP_ECUB_TX,name="B")
    ECUC = RAMN_XCP_Handler(ramn,txid=XCP_ECUC_RX,rxid=XCP_ECUC_TX,name="C")
    ECUD = RAMN_XCP_Handler(ramn,txid=XCP_ECUD_RX,rxid=XCP_ECUD_TX,name="D")
    return None, ECUB, ECUC, ECUD    
    