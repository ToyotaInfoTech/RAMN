#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This module contains general functions to access diagnostic features

from utils.RAMN_UDS_Handler import *
from utils.RAMN_KWP_Handler import *
from utils.RAMN_XCP_Handler import *

def J1939_ID(prio, edp, dp, pf, ps_da, sa):
    return ((prio & 0x7) << 26) | ((edp & 0x1) << 25) | ((dp & 0x1) << 24) | ((pf & 0xFF) << 16) | ((ps_da & 0xFF) << 8) | (sa & 0xFF)

def get_diag_id(da, sa):
    return J1939_ID(6, 0, 0, 0xDA, da, sa)

#returns handlers to interact with ECUs over ISO-TP
def getRAMNHandlers(port, tester_sa=0xF9, filterID=b'M7e0', filterMask=b'm7F0',filterIDExtended=None, filterMaskExtended=None):
    if filterIDExtended is None:
        # Accept extended IDs destined to tester_sa
        mask = 0x1FFFFF00
        val = J1939_ID(6, 0, 0, 0xDA, tester_sa, 0)
        filterIDExtended = f"M{val:08x}".encode()
        filterMaskExtended = f"m{mask:08x}".encode()
        
    ramn = RAMN_USB_Handler(port)    
    ramn.open(autoOpen=False)
    
    #Set CAN filter to only receive Diagnostic messages
    ramn.sendCommand(b'W2')   #filter value/mask type

    ramn.sendCommand(filterID)#Filter ID
    ramn.sendCommand(filterMask)#Filter mask  
    ramn.sendCommand(filterIDExtended)#Filter ID
    ramn.sendCommand(filterMaskExtended)#Filter mask  
    
    #Open port and empty current buffer
    ramn.sendCommand(b'O') 
    time.sleep(0.1)
    ramn.flush()
    
    #Create ISO-TP communication handler
    # We map what the tester receives to what it transmits to match the library logic
    tp_map = {
        get_diag_id(tester_sa, 0xE1): get_diag_id(0xE1, tester_sa),
        get_diag_id(tester_sa, 0xE2): get_diag_id(0xE2, tester_sa),
        get_diag_id(tester_sa, 0xE3): get_diag_id(0xE3, tester_sa),
        get_diag_id(tester_sa, 0xE5): get_diag_id(0xE5, tester_sa),
        get_diag_id(tester_sa, 0xE6): get_diag_id(0xE6, tester_sa),
        get_diag_id(tester_sa, 0xE7): get_diag_id(0xE7, tester_sa),
    }
    tp = RAMN_ISOTP_Handler(ramn, tp_map, is_extended=True)

    return ramn,tp
    
#returns handlers to access UDS diagnostics for all ECUs of RAMN
def getECUHandlersUDS(ramn,tp, tester_sa=0xF9):
    ECUA = RAMN_UDS_Handler(ramn,tp,txid=get_diag_id(0xE0, tester_sa),rxid=get_diag_id(tester_sa, 0xE0),name="A",isUSB=True,is_extended=True)
    ECUB = RAMN_UDS_Handler(ramn,tp,txid=get_diag_id(0xE1, tester_sa),rxid=get_diag_id(tester_sa, 0xE1),name="B",is_extended=True)
    ECUC = RAMN_UDS_Handler(ramn,tp,txid=get_diag_id(0xE2, tester_sa),rxid=get_diag_id(tester_sa, 0xE2),name="C",is_extended=True)
    ECUD = RAMN_UDS_Handler(ramn,tp,txid=get_diag_id(0xE3, tester_sa),rxid=get_diag_id(tester_sa, 0xE3),name="D",is_extended=True)
    return ECUA, ECUB, ECUC, ECUD
    
#returns handlers to access KWP diagnostics for all ECUs of RAMN
def getECUHandlersKWP(ramn,tp, tester_sa=0xF9):
    ECUB = RAMN_KWP_Handler(ramn,tp,txid=get_diag_id(0xE5, tester_sa),rxid=get_diag_id(tester_sa, 0xE5),name="B",is_extended=True)
    ECUC = RAMN_KWP_Handler(ramn,tp,txid=get_diag_id(0xE6, tester_sa),rxid=get_diag_id(tester_sa, 0xE6),name="C",is_extended=True)
    ECUD = RAMN_KWP_Handler(ramn,tp,txid=get_diag_id(0xE7, tester_sa),rxid=get_diag_id(tester_sa, 0xE7),name="D",is_extended=True)
    return None, ECUB, ECUC, ECUD    
    
#returns handlers to access XCP diagnostics for all ECUs of RAMN
def getECUHandlersXCP(ramn, tester_sa=0xF9):
    ECUB = RAMN_XCP_Handler(ramn,txid=get_diag_id(0x52, tester_sa),rxid=get_diag_id(tester_sa, 0x52),name="B",is_extended=True)
    ECUC = RAMN_XCP_Handler(ramn,txid=get_diag_id(0x54, tester_sa),rxid=get_diag_id(tester_sa, 0x54),name="C",is_extended=True)
    ECUD = RAMN_XCP_Handler(ramn,txid=get_diag_id(0x56, tester_sa),rxid=get_diag_id(tester_sa, 0x56),name="D",is_extended=True)
    return None, ECUB, ECUC, ECUD
    
