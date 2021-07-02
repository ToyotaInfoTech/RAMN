#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This module holds various features used throughout all RAMN modules, such as DEBUG functions and hex/list conversions

import click
import platform
import intelhex
import time
import os
import serial.tools.list_ports
import binascii
from utils.RAMN_STM32L552_Utils import *  

#Verbose levels
DEFAULT_VERBOSE_LEVEL = 4
LOG_ERROR   = 0 
LOG_OUTPUT  = 1  
LOG_WARNING = 2  
LOG_DATA    = 3
LOG_DEBUG   = 4

def autoDetectRAMNPort():
    result = None
    for port in serial.tools.list_ports.comports():
        if (port.vid == 0x483) and (port.pid == 0x5740):
            result = port.device
    return result
    
#Class to read settings from a .ini file and make those settings easily available to other modules
class RAMN_Utils:
    VERBOSE_LEVEL=DEFAULT_VERBOSE_LEVEL
    RAMN_DEFAULT_PORT = None
    ECUA_FIRMWARE_PATH = None
    ECUB_FIRMWARE_PATH = None
    ECUC_FIRMWARE_PATH = None
    ECUD_FIRMWARE_PATH = None
    DEFAULT_VERBOSE = 4
    DEFAULT_TIMEOUT = 0
    CAN_NAME = None
    CAN_TYPE = None
    CAN_REFRESH_RATE = 200
    VCAND_HARDWARE_PORT = None
    SERIAL_FORWARD_TARGET = ''
    
    @staticmethod
    def setVerboseLevel(level):
        RAMN_Utils.VERBOSE_LEVEL = level
        
    @classmethod
    def readDefaultSettings(cls):
        if platform.system() == 'Windows':
            settingsPath = os.path.dirname(__file__) + '\\..\\settings\\windows.ini'
        else:
            settingsPath = os.path.dirname(__file__) + '/../settings/linux.ini' 
            
        try:
            with open(settingsPath,'r') as f:
                for line in f:
                    if len(line) > 1:
                        line = line.rstrip() #Remove trailing
                        if not line.startswith("#"):
                            line = line.split("#")[0] #Remove potential comments
                            unpacked = line.split("=")
                            if len(unpacked) == 2:
                                p,v = unpacked
                                p = p.strip(" ")
                                v = v.strip(" ")
                                if p == "PORT":
                                    if v == "AUTODETECT":
                                        RAMN_Utils.RAMN_DEFAULT_PORT = autoDetectRAMNPort()
                                        if RAMN_Utils.RAMN_DEFAULT_PORT == None:
                                            print("Could not find RAMN Serial Port")
                                    else:
                                        RAMN_Utils.RAMN_DEFAULT_PORT = v
                                elif p == "ECUA_FIRMWARE_PATH":
                                    RAMN_Utils.ECUA_FIRMWARE_PATH = v
                                elif p == "ECUB_FIRMWARE_PATH":
                                    RAMN_Utils.ECUB_FIRMWARE_PATH = v
                                elif p == "ECUC_FIRMWARE_PATH":
                                    RAMN_Utils.ECUC_FIRMWARE_PATH = v
                                elif p == "ECUD_FIRMWARE_PATH":
                                    RAMN_Utils.ECUD_FIRMWARE_PATH = v
                                elif p == "VERBOSE":
                                    RAMN_Utils.DEFAULT_VERBOSE = int(v,10)
                                elif p == "TIMEOUT":
                                    RAMN_Utils.DEFAULT_TIMEOUT = int(v,10)
                                elif p == "CAN_NAME":
                                    if v == "AUTODETECT":
                                        RAMN_Utils.CAN_NAME = autoDetectRAMNPort()
                                        if RAMN_Utils.CAN_NAME == None:
                                            print("Could not find RAMN Serial Port")
                                    else:
                                        RAMN_Utils.CAN_NAME = v
                                elif p == "CAN_TYPE":
                                    RAMN_Utils.CAN_TYPE = v
                                elif p == "CAN_REFRESH_RATE":
                                    RAMN_Utils.CAN_REFRESH_RATE = int(v,10)  
                                elif p == "VCAND_HARDWARE_PORT":
                                    if v == "AUTODETECT":
                                        RAMN_Utils.VCAND_HARDWARE_PORT = autoDetectRAMNPort()
                                        if RAMN_Utils.VCAND_HARDWARE_PORT == None:
                                            print("Could not find RAMN Serial Port")
                                    else:
                                        RAMN_Utils.VCAND_HARDWARE_PORT = v    
                                elif p == "SERIAL_FORWARD_TARGET":
                                    RAMN_Utils.SERIAL_FORWARD_TARGET = v    
                                     
                            else:
                                print("Invalid Settings File")
        except:
            #no settings file
            print("Settings File not found")
            
        return cls

#Displays a log message if verbose level matches conditions                            
def log(txt, typ=LOG_OUTPUT,end=None):
    if typ > RAMN_Utils.VERBOSE_LEVEL:
        return
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

def listToInt32(l):
    return (l[0] << 24) + (l[1] << 16) + (l[2] << 8) + l[3]
    
def listToInt16(l):
    return (l[0] << 8) + l[1]
    
def int32ToList(val):
    return [(val >> 24)&0xFF, (val >> 16)&0xFF, (val >> 8)&0xFF, (val&0xFF)]
    
def int16ToList(val):
    return [(val >> 8)&0xFF, (val&0xFF)]
    
def listToHex(l):
    result = ""
    for item in l:
        result += "{:02x}".format(item)
    return result
    
#Breaks a bytes area in chunks of the size requested by the target over UDS
def breakInUDSChunk(data,chunkSize):
    index = 0
    chunk = []
    result = []
    while True:
        chunk += [data[index]]
        index += 1
        if (index % chunkSize) == 0:
            yield chunk
            chunk = []
        if index >= len(data):
            yield chunk
            break
 
#Reads a hex file and returns bytes array to be written to the specified address with specified size
def getDownloadData(filename):
    ih = intelhex.IntelHex(filename)
    start = ih.segments()[0][0]
    end = ih.segments()[1][1]

    #Pad empty data inside hex file with FF
    for addr in range(ih.segments()[0][1],ih.segments()[1][0]):
        ih[addr] = 0xFF
    
    #make sure data is #8 bytes aligned    
    while ( (end)%8 != 0): 
        ih[end] = 0xFF
        end += 1
    
    #Check that address range is valid
    data = [ih[addr] for addr in range(start,end)]   
    if checkAddressValidity(start,end):
        return start, (end - start), data
    else:
        return None, None, None

#Class to make OBD-II commands human-readable. Mostly based on Wikipedia page: https://en.wikipedia.org/wiki/OBD-II_PIDs
class OBDIIAnalyzer(object):

    SAE_STANDARD_SERVICE_01 = {
    0x00:"PIDs supported [01 - 20]",
    0x01:"Monitor status since DTCs cleared",
    0x02:"Freeze DTC",
    0x03:"Fuel system status",
    0x04:"Calculated engine load",
    0x05:"Engine coolant temperature",
    0x06:"Short term fuel trim-Bank 1",
    0x07:"Long term fuel trim-Bank 1",
    0x08:"Short term fuel trim-Bank 2",
    0x09:"Long term fuel trim-Bank 2",
    0x0A:"Fuel pressure (gauge pressure)",
    0x0B:"Intake manifold absolute pressure",
    0x0C:"Engine RPM",
    0x0D:"Vehicle speed",
    0x0E:"Timing advance",
    0x0F:"Intake air temperature",
    0x10:"MAF air flow rate"    ,
    0x11:"Throttle position",
    0x12:"Commanded secondary air status",
    0x13:"Oxygen sensors present (in 2 banks)",
    0x14:"Oxygen Sensor 1 (Voltage/Short term fuel trim)",
    0x15:"Oxygen Sensor 2 (Voltage/Short term fuel trim)",
    0x16:"Oxygen Sensor 3 (Voltage/Short term fuel trim)",
    0x17:"Oxygen Sensor 4 (Voltage/Short term fuel trim)",
    0x18:"Oxygen Sensor 5 (Voltage/Short term fuel trim)",
    0x19:"Oxygen Sensor 6 (Voltage/Short term fuel trim)",
    0x1A:"Oxygen Sensor 7 (Voltage/Short term fuel trim)",
    0x1B:"Oxygen Sensor 8 (Voltage/Short term fuel trim)",
    0x1C:"OBD standards this vehicle conforms to",
    0x1D:"Oxygen sensors present (in 4 banks)",
    0x1E:"Auxiliary input status",
    0x1F:"Run time since engine start",
    0x20:"PIDs supported [21 - 40]",
    0x21:"Distance traveled with malfunction indicator lamp (MIL) on",
    0x22:"Fuel Rail Pressure (relative to manifold vacuum)",
    0x23:"Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)",
    0x24:"Oxygen Sensor 1 (Fuel-Air Equivalent Ratio/Voltage)",
    0x25:"Oxygen Sensor 2 (Fuel-Air Equivalent Ratio/Voltage)",
    0x26:"Oxygen Sensor 3 (Fuel-Air Equivalent Ratio/Voltage)",
    0x27:"Oxygen Sensor 4 (Fuel-Air Equivalent Ratio/Voltage)",
    0x28:"Oxygen Sensor 5 (Fuel-Air Equivalent Ratio/Voltage)",
    0x29:"Oxygen Sensor 6 (Fuel-Air Equivalent Ratio/Voltage)",
    0x2A:"Oxygen Sensor 7 (Fuel-Air Equivalent Ratio/Voltage)",
    0x2B:"Oxygen Sensor 8 (Fuel-Air Equivalent Ratio/Voltage)",
    0x2C:"Commanded EGR",
    0x2D:"EGR Error",
    0x2E:"Commanded evaporative purge",
    0x2F:"Fuel Tank Level Input",
    0x30:"Warm-ups since codes cleared",
    0x31:"Distance traveled since codes cleared",
    0x32:"Evap. System Vapor Pressure",
    0x33:"Absolute Barometric Pressure" ,
    0x34:"Oxygen Sensor 1 (Fuel-Air Equivalence Ratio/Current)",
    0x35:"Oxygen Sensor 2 (Fuel-Air Equivalence Ratio/Current)",
    0x36:"Oxygen Sensor 3 (Fuel-Air Equivalence Ratio/Current)",
    0x37:"Oxygen Sensor 4 (Fuel-Air Equivalence Ratio/Current)",
    0x38:"Oxygen Sensor 5 (Fuel-Air Equivalence Ratio/Current)",
    0x39:"Oxygen Sensor 6 (Fuel-Air Equivalence Ratio/Current)",
    0x3A:"Oxygen Sensor 7 (Fuel-Air Equivalence Ratio/Current)",
    0x3B:"Oxygen Sensor 8 (Fuel-Air Equivalence Ratio/Current)",
    0x3C:"Catalyst Temperature: Bank 1, Sensor 1",
    0x3D:"Catalyst Temperature: Bank 2, Sensor 1",
    0x3E:"Catalyst Temperature: Bank 1, Sensor 2",
    0x3F:"Catalyst Temperature: Bank 2, Sensor 2",
    0x40:"PIDs supported [41 - 60]",
    0x41:"Monitor status this drive cycle",
    0x42:"Control module voltage",
    0x43:"Absolute load value",
    0x44:"Fuel-Air commanded equivalence ratio",
    0x45:"Relative throttle position",
    0x46:"Ambient air temperature",
    0x47:"Absolute throttle position B",
    0x48:"Absolute throttle position C",
    0x49:"Accelerator pedal position D",
    0x4A:"Accelerator pedal position E",
    0x4B:"Accelerator pedal position F",
    0x4C:"Commanded throttle actuator",
    0x4D:"Time run with MIL on",
    0x4E:"Time since trouble codes cleared",
    0x4F:"Maximum value for Fuel-Air equivalence ratio, oxygen sensor voltage, oxygen sensor current, and intake manifold absolute pressure",
    0x50:"Maximum value for air flow rate from mass air flow sensor",
    0x51:"Fuel Type",
    0x52:"Ethanol fuel %",
    0x53:"Absolute Evap system Vapor Pressure",
    0x54:"Evap system vapor pressure",
    0x55:"Short term secondary oxygen sensor trim, A: bank 1, B: bank 3",
    0x56:"Long term secondary oxygen sensor trim, A: bank 1, B: bank 3",
    0x57:"Short term secondary oxygen sensor trim, A: bank 2, B: bank 4",
    0x58:"Long term secondary oxygen sensor trim, A: bank 2, B: bank 4",
    0x59:"Fuel rail absolute pressure",
    0x5A:"Relative accelerator pedal position",
    0x5B:"Hybrid battery pack remaining life",
    0x5C:"Engine oil temperature",
    0x5D:"Fuel injection timing",
    0x5E:"Engine fuel rate",
    0x5F:"Emission requirements to which vehicle is designed",
    0x60:"PIDs supported [61 - 80]",
    0x61:"Driver's demand engine - percent torque",
    0x62:"Actual engine - percent torque",
    0x63:"Engine reference torque",
    0x64:"Engine percent torque data",
    0x65:"Auxiliary input / output supported",
    0x66:"Mass air flow sensor",
    0x67:"Engine coolant temperature",
    0x68:"Intake air temperature sensor",
    0x69:"Commanded EGR and EGR Error",
    0x6A:"Commanded Diesel intake air flow control and relative intake air flow position",
    0x6B:"Exhaust gas recirculation temperature",
    0x6C:"Commanded throttle actuator control and relative throttle position",
    0x6D:"Fuel pressure control system",
    0x6E:"Injection pressure control system",
    0x6F:"Turbocharger compressor inlet pressure",
    0x70:"Boost pressure control",
    0x71:"Variable Geometry turbo (VGT) control",
    0x72:"Wastegate control",
    0x73:"Exhaust pressure",
    0x74:"Turbocharger RPM",
    0x75:"Turbocharger temperature",
    0x76:"Turbocharger temperature",
    0x77:"Charge air cooler temperature (CACT)",
    0x78:"Exhaust Gas temperature (EGT) Bank 1",
    0x79:"Exhaust Gas temperature (EGT) Bank 2",
    0x7A:"Diesel particulate filter (DPF) ",
    0x7B:"Diesel particulate filter (DPF) ",
    0x7C:"Diesel particulate filter (DPF) temperature",
    0x7D:"NOx NTE (Not-To-Exceed) control area status",
    0x7E:"PM NTE (Not-To-Exceed) control area status",
    0x7F:"Engine run time",
    0x80:"PIDs supported [81 - A0]",
    0x81:"Engine run time for Auxiliary Emissions Control Device(AECD)",
    0x82:"Engine run time for Auxiliary Emissions Control Device(AECD)",
    0x83:"NOx sensor",
    0x84:"Manifold surface temperature",
    0x85:"NOx reagent system",
    0x86:"Particulate matter (PM) sensor",
    0x87:"Intake manifold absolute pressure",
    0x88:"SCR Induce System",
    0x89:"Run Time for AECD #11-#15",
    0x8A:"Run Time for AECD #16-#20",
    0x8B:"Diesel Aftertreatment",
    0x8C:"O2 Sensor (Wide Range)",
    0x8D:"Throttle Position G",
    0x8E:"Engine Friction - Percent Torque",
    0x8F:"PM Sensor Bank 1 & 2",
    0x90:"WWH-OBD Vehicle OBD System Information",
    0x91:"WWH-OBD Vehicle OBD System Information",
    0x92:"Fuel System Control",
    0x93:"WWH-OBD Vehicle OBD Counters support",
    0x94:"NOx Warning And Inducement System",
    0x98:"Exhaust Gas Temperature Sensor",
    0x99:"Exhaust Gas Temperature Sensor",
    0x9A:"Hybrid/EV Vehicle System Data, Battery, Voltage",
    0x9B:"Diesel Exhaust Fluid Sensor Data",
    0x9C:"O2 Sensor Data",
    0x9D:"Engine Fuel Rate",
    0x9E:"Engine Exhaust Flow Rate",
    0x9F:"Fuel System Percentage Use",
    0xA0:"PIDs supported [A1 - C0]",
    0xA1:"NOx Sensor Corrected Data",
    0xA2:"Cylinder Fuel Rate",
    0xA3:"Evap System Vapor Pressure",
    0xA4:"Transmission Actual Gear",
    0xA5:"Diesel Exhaust Fluid Dosing",
    0xA6:"Odometer",
    0xC0:"PIDs supported [C1 - E0]",
    0xC3:"(cf Wikipedia)",
    0xC4:"(cf Wikipedia)"
    }

    SAE_STANDARD_SERVICE_05 = {
    0x0100:"OBD Monitor IDs supported ($01 - $20)",
    0x0101:"O2 Sensor Monitor Bank 1 Sensor 1",
    0x0102:"O2 Sensor Monitor Bank 1 Sensor 2",
    0x0103:"O2 Sensor Monitor Bank 1 Sensor 3",
    0x0104:"O2 Sensor Monitor Bank 1 Sensor 4",
    0x0105:"O2 Sensor Monitor Bank 2 Sensor 1",
    0x0106:"O2 Sensor Monitor Bank 2 Sensor 2",
    0x0107:"O2 Sensor Monitor Bank 2 Sensor 3",
    0x0108:"O2 Sensor Monitor Bank 2 Sensor 4",
    0x0109:"O2 Sensor Monitor Bank 3 Sensor 1",
    0x010A:"O2 Sensor Monitor Bank 3 Sensor 2",
    0x010B:"O2 Sensor Monitor Bank 3 Sensor 3",
    0x010C:"O2 Sensor Monitor Bank 3 Sensor 4",
    0x010D:"O2 Sensor Monitor Bank 4 Sensor 1",
    0x010E:"O2 Sensor Monitor Bank 4 Sensor 2",
    0x010F:"O2 Sensor Monitor Bank 4 Sensor 3",
    0x0110:"O2 Sensor Monitor Bank 4 Sensor 4",
    0x0201:"O2 Sensor Monitor Bank 1 Sensor 1",
    0x0202:"O2 Sensor Monitor Bank 1 Sensor 2",
    0x0203:"O2 Sensor Monitor Bank 1 Sensor 3",
    0x0204:"O2 Sensor Monitor Bank 1 Sensor 4",
    0x0205:"O2 Sensor Monitor Bank 2 Sensor 1",
    0x0206:"O2 Sensor Monitor Bank 2 Sensor 2",
    0x0207:"O2 Sensor Monitor Bank 2 Sensor 3",
    0x0208:"O2 Sensor Monitor Bank 2 Sensor 4",
    0x0209:"O2 Sensor Monitor Bank 3 Sensor 1",
    0x020A:"O2 Sensor Monitor Bank 3 Sensor 2",
    0x020B:"O2 Sensor Monitor Bank 3 Sensor 3",
    0x020C:"O2 Sensor Monitor Bank 3 Sensor 4",
    0x020D:"O2 Sensor Monitor Bank 4 Sensor 1",
    0x020E:"O2 Sensor Monitor Bank 4 Sensor 2",
    0x020F:"O2 Sensor Monitor Bank 4 Sensor 3",
    0x0210:"O2 Sensor Monitor Bank 4 Sensor 4"  
    }

    SAE_STANDARD_SERVICE_06 = {
    0x00:"ISO/SAE reserved",
    0x01:"Rich to lean sensor threshold voltage (constant)",
    0x02:"Lean to rich sensor threshold voltage (constant)",
    0x03:"Low sensor voltage for switch time calculation (constant)",
    0x04:"High sensor voltage for switch time calculation (constant)",
    0x05:"Rich to lean sensor switch time (calculated)",
    0x06:"Lean to rich sensor switch time (calculated)",
    0x07:"Minimum sensor voltage for test cycle (calculated)",
    0x08:"Maximum sensor voltage for test cycle (calculated)",
    0x09:"Time between sensor transitions (calculated)",
    0x0A:"Sensor period (calculated)",
    0x0B:"EWMA (Exponential Weighted Moving Average) misfire counts for last ten (10) driving cycles",
    }

    SAE_STANDARD_SERVICE_09 = {
    0x00:"Service 9 supported PIDs (01 to 20)",
    0x01:"VIN Message Count in PID 02",
    0x02:"Vehicle Identification Number (VIN)",
    0x03:"Calibration ID message count for PID 04",
    0x04:"Calibration ID",
    0x05:"Calibration verification numbers (CVN) message count for PID 06",
    0x06:"Calibration Verification Numbers (CVN) Several CVN can be outputed (4 bytes each)",
    0x07:"In-use performance tracking message count for PID 08 and 0B",
    0x08:"In-use performance tracking for spark ignition vehicles",
    0x09:"ECU name ,message count for PID 0A",
    0x0A:"ECU name",
    0x0B:"In-use performance tracking for compression ignition vehicles"
    }

#Class to make UDS commands human-readable.
class UDSAnalyzer(object):

    Request_FunctionsWithoutSubBytes = [0x03,0x04,0x06,0x07,0x08,0x22,0x23,0x24,0x2A,0x14,0x2F,0x34,0x35,0x36,0x37,0x84,0x86]
    Response_FunctionsWithoutSubBytes = [0x62,0x63,0x64,0x6A,0x54,0x6F,0x74,0x75,0x76,0x77]

    UDS_SID_STRINGS = {
        0x01:"OBD-II - Show current data",
        0x02:"OBD-II - Show freeze frame data",
        0x03:"OBD-II - Show stored Diagnostic Trouble Codes",
        0x04:"OBD-II - Clear Diagnostic Trouble Codes and stored values,",
        0x05:"OBD-II - Test results, oxygen sensor monitoring (non CANonly)",
        0x06:"OBD-II - Test results, other component/system monitoring",
        0x07:"OBD-II - Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)",
        0x08:"OBD-II - Control operation of on-board component/system",
        0x09:"OBD-II - Request vehicle information",
        0x0A:"OBD-II - Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)",
        0x10:"Diagnostic Session Control",
        0x11:"ECU Reset",
        0x14:"Clear Diagnostic Information",
        0x19:"Read DTC Information",
        0x22:"Read Data by Identifier",
        0x23:"Read Memory by Address",
        0x24:"Read Scaling Data by Identifier",
        0x27:"Security Access",
        0x28:"Communication Control",
        0x29:"Authentication",
        0x2A:"Read Data by Identifier Periodic",
        0x2C:"Dynamically Define Data Identifier",
        0x2E:"Write Data by Identifier",
        0x3D:"Write Memory by Address",
        0x3E:"Tester Present",
        0x2F:"Input Output Control by Identifier",
        0x31:"Routine Control   ",
        0x34:"Request Download",
        0x35:"Request Upload",
        0x36:"Transfer Data",
        0x37:"Request Transfer Exit",
        0x38:"Request File Transfer",
        0x3F:"Negative",
        0x83:"Access Timing Parameters",
        0x84:"Secured Data Transmission",
        0x85:"Control DTC Settings",
        0x86:"Response On Event",
        0x87:"Link Control",
    }

    ERROR_STRINGS= {
        0x10:"GR-General Reject",
        0x11:"SNS-Service Not Supported",
        0x12:"SFNS-Sub-Function NOT supported",
        0x13:"IMLOIF-Incorrect Message Length or Invalid Format",
        0x14:"RTL-Response too long",
        0x21:"BRR-Busy repeat request",
        0x22:"CNC-Conditions NOT correct",
        0x24:"RSE-Request Sequence Error",
        0x25:"NRFSC-No Response from sub-net component",
        0x26:"FPEORA-Failure Prevents Execution of Requested Action",
        0x31:"ROOR-Request out of range",
        0x33:"SAD-Security Access Denied",
        0x35:"IK-Invalid Key",
        0x36:"ENOA-Exceeded Number of Attempts",
        0x37:"RTDNE-Required Time Delay NOT Expired",
        0x38:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x39:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x40:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x41:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x42:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x43:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x44:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x45:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x46:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x47:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x48:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x49:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x4A:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x4B:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x4C:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x4D:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x4E:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x4F:"RBEDLSD-Reserved by Extended Data Link Security Document",
        0x70:"UDNA-Upload/Download NOT accepted",
        0x71:"TDS-Transfer Data Suspended",
        0x72:"GPF-General Programming Failure",
        0x73:"WBSC-Wrong Block Sequence Counter",
        0x78:"RCRRP-Request Correctly Received but Response is Pending",
        0x7E:"SFNSIAS-Sub-Function NOT Supported in Active Session",
        0x7F:"SNSIAS-Service NOT Supported in Active Session",
        0x81:"RPMTH-rpm too high",
        0x82:"RPMTL-rpm too low",
        0x83:"EIR-Engine is running",
        0x84:"EINR-Engine is NOT running",
        0x85:"ERTTL-Engine run-time too low",
        0x86:"TEMPTH-Temperature too high",
        0x87:"TEMPTL-Temperature too low",
        0x88:"VSTH-Vehicle speed too high",
        0x89:"VSTL-Vehicle speed too low",
        0x8A:"TPTH-Throttle/Pedal too high",
        0x8B:"TPTL-Throttle/Pedal too low",
        0x8C:"TRNIN-Transmission range NOT in neutral",
        0x8D:"TRNIG-Transmission range NOT in gear",
        0x8F:"?-Brake switch NOT closed",
        0x90:"SLNIP-Shifter lever NOT in park",
        0x91:"TCCL-Torque converter clutch locked",
        0x92:"VTH-Voltage too high",
        0x93:"VTL-Voltage too low"
    }

    DTC_SUB_FUNC = {
        0x01:"Report number of DTC by Status Mask",
        0x02:"Report DTC by Status Mask",
        0x03:"Report DTC Snapshot Identification",
        0x04:"Report DTC Snapshot Record by DTC number",
        0x05:"Report DTC Snapshot Record by Record number",
        0x06:"Report DTC Extended Data Record by DTC number",
        0x07:"Report number of DTC by Severity Mask Record",
        0x08:"Report DTC by Severity Mask Record",
        0x09:"Report Severity Information of DTC",
        0x0A:"Report Supported DTC",
        0x0B:"Report First Test Failed DTC",
        0x0C:"Report First Confirmed DTC",
        0x0D:"Report Most Recent Test Failed DTC",
        0x0E:"Report Most Recent Confirmed DTC",
        0x0F:"Report Mirror Memory DTC by Status Mask",
        0x10:"Report Mirror Memory DTC Extended Data Record by DTC number",
        0x11:"Report number of Mirror Memory DTC by Status Mask",
        0x12:"Report number of Emissions Related OBD DTC by Status Mask",
        0x13:"Report Emissions Related OBD DTC By status Mask",
        0x14:"Report DTC Fault Detection Counter",
        0x15:"Report DTC with Permanent Status",
    }

    def isValidSID(self, SID):
        return (SID in self.UDS_SID_STRINGS.keys()) or (SID-0x40 in self.UDS_SID_STRINGS.keys())

    def hasSubFunctionBytes(self, SID):
        return not (SID in self.Request_FunctionsWithoutSubBytes or SID in self.Response_FunctionsWithoutSubBytes)

    DTC_DOMAIN_CODE = {
    0x00:"P",
    0x01:"C",
    0x02:"B",
    0x03:"U"
    }

    def decodeDTC(self,data):
        result = ""
        if len(data) < 2:
            return "NOT A DTC"
        d = (data[0]>>6)&0x3 #first DTC character
        result += self.DTC_DOMAIN_CODE[d]
        d = (data[0]>>4)&0x3 #Second DTC character
        result += "{:01x}".format(d)
        d = (data[0])&0xf #Second DTC character
        result += "{:01x}".format(d)
        result += "{:02x}".format(data[1])
        return result

    def getSubByteDisplayString(self, payload):
        SID = payload[0]
        if SID in self.Request_FunctionsWithoutSubBytes or SID in self.Response_FunctionsWithoutSubBytes:
            return ""
        if len(payload) > 1:
            data = payload[1]
        else:
            data = []
        if SID == 0x7f and len(payload) > 2:
            if payload[2] in self.ERROR_STRINGS.keys(): return "Negative Response: " + self.ERROR_STRINGS[payload[2]]
            else: return "Negative Response: Invalid code"

        elif SID >= 0x40 and SID < 0x80 or SID >= 0xC3:
            SID -= 0x40        

        if SID == 0x01 or SID == 0x02:
            if data in OBDIIAnalyzer.SAE_STANDARD_SERVICE_01.keys():
                return OBDIIAnalyzer.SAE_STANDARD_SERVICE_01[data]
            else: return "Invalid"

        if SID == 0x05:
            data = (payload[1]<<8 + payload[2])
            if data in OBDIIAnalyzer.SAE_STANDARD_SERVICE_05.keys():
                return OBDIIAnalyzer.SAE_STANDARD_SERVICE_05[data]
            else: return "Invalid"

        if SID == 0x06:
            if data in OBDIIAnalyzer.SAE_STANDARD_SERVICE_06.keys():
                return OBDIIAnalyzer.SAE_STANDARD_SERVICE_06[data]
            else: return "Invalid"

        if SID == 0x09:
            if data in OBDIIAnalyzer.SAE_STANDARD_SERVICE_09.keys():
                return OBDIIAnalyzer.SAE_STANDARD_SERVICE_09[data]
            else: return "Invalid"

        if SID == 0x10:
            if data == 0x01: return "Default Session"
            elif data == 0x02: return "Programming Session"
            elif data == 0x03: return "Extended Diagnostic Session"
            elif data == 0x04: return "Safety System Diagnostic Session"
            elif data >= 0x40 and data <= 0x5F: return "Vehicle Manufaturer Specific"
            else: return "Invalid"

        elif SID == 0x11:
            if data == 0x01: return "Hard reset"
            else: return "Invalid"

        elif SID == 0x19:
            if data in self.DTC_SUB_FUNC.keys(): return "DTC Request: " + self.DTC_SUB_FUNC[data]
            else:  return "DTC Request: Invalid"
        elif SID  == 0x22 or SID == 0x2E:  return "address: " + hex(((payload[1])<<8) + payload[2])

        elif SID == 0x27:
            if data & 0x1 == 0x01: return "Seed Request level " + hex(data) + " : " + str(payload[2:])
            else: return "Key Send level " + hex(data-1) + " : " + str(payload[2:])

        elif SID == 0x28:
            if data == 0: return "EnableRxAndTx"
            elif data == 0x01: return "EnableRxAndDisableTx"
            elif data == 0x03: return "DisableRxAndTx"
            else: return "Invalid"

        elif SID == 0x31:
            if data == 0x01: return "Start Routine"
            elif data == 0x02: return "Stop Routine"
            elif data == 0x03: return "Request Routine Results"
            else: return "Invalid"

        elif SID == 0x34:  return "Data Format Identifier: " + hex(data)

        elif SID == 0x36:  return "Block Sequence Counter: " + hex(data)
        
        elif SID == 0x3E:
            if data == 0x00: return "with echo"
            elif data == 0x80: return "without echo"
            else: return "Invalid"

        else:
            return "Unknown or invalid Subbyte"

    def getErrorCodeDescription(self, code):
        if code in self.ERROR_STRINGS.keys():
            return self.ERROR_STRINGS[code]
        else:
            return "Invalid Error Code"
    
    def getSIDDescription(self,SID):
        description = ""
        if SID in self.UDS_SID_STRINGS.keys():
            description =  self.UDS_SID_STRINGS[SID]
        elif SID-0x40 in self.UDS_SID_STRINGS.keys():
            description =  self.UDS_SID_STRINGS[SID-0x40] + " Response"
        elif SID == 0x7F:
            description = "ERROR" 
        else: description =  "Unknown/Invalid SID"        
        
        return description

    def display(self, data):
        description = self.getSIDDescription(data[0])    
        result = "SID " + "${:02x}".format(data[0]) + " - "+ description
        if len(data) > 1 and self.hasSubFunctionBytes(data[0]):
                result += " - Subfunction " + "${:02x}".format(data[1]) + " - " + self.getSubByteDisplayString(data)
        return result
        

#Read default settings at import
RAMN_Utils.readDefaultSettings()
