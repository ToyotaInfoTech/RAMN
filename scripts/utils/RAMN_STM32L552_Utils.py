#!/usr/bin/env python
# Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

#This modules holds functions to make it easier to access STM32L552/STM32L562 microcontroller's memory area

#Main areas readable by Application. There are more readable areas (e.g. peripheral registers), refer to STM32L5 manual
ECUreadableRange = {
    "FLASH_CURRENT":       (0x08000000,0x08040000), #(0x08000000,0x08040000) for STM32L552CC 
    "FLASH_BACKUP":        (0x08040000,0x08080000), #(0x08000000,0x08040000) for STM32L552CC 
    #"OTP":                (0x0BFA0000,0x0BFA0200), #Commented out as precaution, is actually readable
    "SRAM1":               (0x20000000,0x20030000),
    "SRAM2":               (0x20030000,0x20040000),
}

#Simplified address change for memory areas
def checkAddressValidity(start,end):
    startOK = False
    endOK = False
    for ran in ECUreadableRange.keys():
        if start >= ECUreadableRange[ran][0] and start < ECUreadableRange[ran][1]:
            startOK = True
        if end > ECUreadableRange[ran][0] and end <= ECUreadableRange[ran][1]:
            endOK = True
    return startOK and endOK
    

        

