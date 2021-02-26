EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Debugger breakout - RAMN V1"
Date "2021-02-22"
Rev "A"
Comp "Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED."
Comment1 ""
Comment2 ""
Comment3 "License: CC BY-SA 4.0"
Comment4 "https://github.com/toyotainfotech/ramn"
$EndDescr
Text Label 2850 1950 0    50   ~ 0
PA7
Text Label 2850 1850 0    50   ~ 0
PA5
Text Label 2850 1750 0    50   ~ 0
PA3
Text Label 2850 2050 0    50   ~ 0
PB1
Text Label 2850 2150 0    50   ~ 0
PB10
Text Label 2350 1550 2    50   ~ 0
3V3_ECU
Wire Wire Line
	2850 1650 3200 1650
$Comp
L power:GND #PWR0101
U 1 1 5D9A7138
P 3200 1650
AR Path="/5D9A7138" Ref="#PWR0101"  Part="1" 
AR Path="/5D8BFFCE/5D9A7138" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D9A7138" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D9A7138" Ref="#PWR?"  Part="1" 
F 0 "#PWR0101" H 3200 1400 50  0001 C CNN
F 1 "GND" H 3300 1650 50  0000 C CNN
F 2 "" H 3200 1650 50  0001 C CNN
F 3 "" H 3200 1650 50  0001 C CNN
	1    3200 1650
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5D9A713E
P 2850 1550
AR Path="/5D8BFFCE/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D9A713E" Ref="#PWR0102"  Part="1" 
F 0 "#PWR0102" H 2850 1400 50  0001 C CNN
F 1 "+5V" H 2865 1723 50  0000 C CNN
F 2 "" H 2850 1550 50  0001 C CNN
F 3 "" H 2850 1550 50  0001 C CNN
	1    2850 1550
	1    0    0    -1  
$EndComp
Text Label 2350 1650 2    50   ~ 0
PA1
Text Label 2350 2050 2    50   ~ 0
PB0
Text Label 2350 2150 2    50   ~ 0
PB2
Text Label 2350 1850 2    50   ~ 0
PA4
Text Label 2350 1950 2    50   ~ 0
PA6
Text Label 2850 2450 0    50   ~ 0
PA8
Text Label 2350 2450 2    50   ~ 0
PB15
Text Label 2350 2250 2    50   ~ 0
PB11
Text Label 2350 2350 2    50   ~ 0
PB13
Text Label 2850 2250 0    50   ~ 0
PB12
Text Label 2850 2350 0    50   ~ 0
PB14
Text Label 2350 1750 2    50   ~ 0
PA2
Text Label 2200 2550 2    50   ~ 0
nRST
Text Label 2200 2650 2    50   ~ 0
SYS_JTDI
Text Label 2200 2750 2    50   ~ 0
SYS_JTDO-SWO
Text Label 3050 2750 0    50   ~ 0
SYS_JTRST
Text Label 3050 2650 0    50   ~ 0
SYS_JTCK-SWCLK
Text Label 3050 2550 0    50   ~ 0
SYS_JTMS-SWDIO
$Comp
L Connector_Generic:Conn_02x13_Odd_Even J9
U 1 1 5D87B279
P 2550 2150
F 0 "J9" H 2600 2900 50  0000 C CNN
F 1 " " H 2600 2676 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x13_P2.54mm_Vertical" H 2550 2150 50  0001 C CNN
F 3 "~" H 2550 2150 50  0001 C CNN
	1    2550 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2350 2550 2200 2550
Wire Wire Line
	2350 2650 2200 2650
Wire Wire Line
	2350 2750 2200 2750
Wire Wire Line
	2850 2750 3050 2750
Wire Wire Line
	2850 2650 3050 2650
Wire Wire Line
	2850 2550 3050 2550
$Comp
L Connector:Conn_ARM_JTAG_SWD_20 J10
U 1 1 5DEDB2F4
P 2500 3950
F 0 "J10" H 1971 3996 50  0000 R CNN
F 1 "Conn_ARM_JTAG_SWD_20" H 1971 3905 50  0000 R CNN
F 2 "Connector_IDC:IDC-Header_2x10_P2.54mm_Horizontal" H 2950 2900 50  0001 L TNN
F 3 "http://infocenter.arm.com/help/topic/com.arm.doc.dui0499b/DUI0499B_system_design_reference.pdf" V 2150 2700 50  0001 C CNN
	1    2500 3950
	1    0    0    -1  
$EndComp
Text Label 2400 3050 2    50   ~ 0
3V3_ECU
Wire Wire Line
	2400 3050 2400 3100
Wire Wire Line
	2500 3150 2500 3100
Wire Wire Line
	2500 3100 2400 3100
Connection ~ 2400 3100
Wire Wire Line
	2400 3100 2400 3150
Text Label 3300 3450 0    50   ~ 0
SYS_JTRST
Text Label 3300 3550 0    50   ~ 0
nRST
Text Label 3300 3850 0    50   ~ 0
SYS_JTCK-SWCLK
Text Label 3300 3950 0    50   ~ 0
SYS_JTMS-SWDIO
Text Label 3300 4150 0    50   ~ 0
SYS_JTDI
Text Label 3300 4050 0    50   ~ 0
SYS_JTDO-SWO
NoConn ~ 3100 4350
NoConn ~ 3100 4450
$Comp
L power:GND #PWR0105
U 1 1 5DEFA108
P 2400 4800
AR Path="/5DEFA108" Ref="#PWR0105"  Part="1" 
AR Path="/5D8BFFCE/5DEFA108" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DEFA108" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DEFA108" Ref="#PWR?"  Part="1" 
F 0 "#PWR0105" H 2400 4550 50  0001 C CNN
F 1 "GND" H 2500 4800 50  0000 C CNN
F 2 "" H 2400 4800 50  0001 C CNN
F 3 "" H 2400 4800 50  0001 C CNN
	1    2400 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 4750 2400 4800
NoConn ~ 3100 3750
Wire Wire Line
	3300 3850 3100 3850
Wire Wire Line
	3300 3950 3100 3950
Wire Wire Line
	3300 4050 3100 4050
Wire Wire Line
	3300 4150 3100 4150
Wire Wire Line
	3300 3550 3100 3550
Wire Wire Line
	3100 3450 3300 3450
Text Label 5300 3900 2    50   ~ 0
3V3_ECU
Text Label 5300 4000 2    50   ~ 0
PA1
Text Label 5300 4400 2    50   ~ 0
PB0
Text Label 5300 4500 2    50   ~ 0
PB2
Text Label 5300 4200 2    50   ~ 0
PA4
Text Label 5300 4300 2    50   ~ 0
PA6
Text Label 5300 4800 2    50   ~ 0
PB15
Text Label 5300 4600 2    50   ~ 0
PB11
Text Label 5300 4700 2    50   ~ 0
PB13
Text Label 5300 4100 2    50   ~ 0
PA2
Text Label 5150 4900 2    50   ~ 0
nRST
Text Label 5150 5000 2    50   ~ 0
SYS_JTDI
Text Label 5150 5100 2    50   ~ 0
SYS_JTDO-SWO
Text Label 9050 4300 0    50   ~ 0
PA7
Text Label 9050 4200 0    50   ~ 0
PA5
Text Label 9050 4100 0    50   ~ 0
PA3
Text Label 9050 4400 0    50   ~ 0
PB1
Text Label 9050 4500 0    50   ~ 0
PB10
$Comp
L power:+5V #PWR?
U 1 1 5DECB5FB
P 9050 3900
AR Path="/5D8BFFCE/5DECB5FB" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DECB5FB" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DECB5FB" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5DECB5FB" Ref="#PWR?"  Part="1" 
AR Path="/5DECB5FB" Ref="#PWR0107"  Part="1" 
F 0 "#PWR0107" H 9050 3750 50  0001 C CNN
F 1 "+5V" H 9065 4073 50  0000 C CNN
F 2 "" H 9050 3900 50  0001 C CNN
F 3 "" H 9050 3900 50  0001 C CNN
	1    9050 3900
	1    0    0    -1  
$EndComp
Text Label 9050 4800 0    50   ~ 0
PA8
Text Label 9050 4600 0    50   ~ 0
PB12
Text Label 9050 4700 0    50   ~ 0
PB14
Text Label 9250 5100 0    50   ~ 0
SYS_JTRST
Text Label 9250 5000 0    50   ~ 0
SYS_JTCK-SWCLK
Text Label 9250 4900 0    50   ~ 0
SYS_JTMS-SWDIO
$Comp
L Connector:TestPoint 3V3
U 1 1 5DECBCF4
P 5400 3900
F 0 "3V3" H 5458 4018 50  0000 L CNN
F 1 "TestPoint" H 5458 3927 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 5600 3900 50  0001 C CNN
F 3 "~" H 5600 3900 50  0001 C CNN
	1    5400 3900
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint 5V1
U 1 1 5DEDB8D4
P 8850 3900
F 0 "5V1" H 8650 4050 50  0000 L CNN
F 1 "TestPoint" H 8908 3927 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 9050 3900 50  0001 C CNN
F 3 "~" H 9050 3900 50  0001 C CNN
	1    8850 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 3900 8850 3900
$Comp
L Connector:TestPoint PA1
U 1 1 5DEDC6DE
P 5500 4000
F 0 "PA1" H 5558 4118 50  0000 L CNN
F 1 "TestPoint" H 5558 4027 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 5700 4000 50  0001 C CNN
F 3 "~" H 5700 4000 50  0001 C CNN
	1    5500 4000
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PA2
U 1 1 5DEDCA7A
P 5650 4100
F 0 "PA2" H 5708 4218 50  0000 L CNN
F 1 "TestPoint" H 5708 4127 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 5850 4100 50  0001 C CNN
F 3 "~" H 5850 4100 50  0001 C CNN
	1    5650 4100
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PA4
U 1 1 5DEDCC99
P 5750 4200
F 0 "PA4" H 5808 4318 50  0000 L CNN
F 1 "TestPoint" H 5808 4227 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 5950 4200 50  0001 C CNN
F 3 "~" H 5950 4200 50  0001 C CNN
	1    5750 4200
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PA6
U 1 1 5DEDCF05
P 5850 4300
F 0 "PA6" H 5908 4418 50  0000 L CNN
F 1 "TestPoint" H 5908 4327 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6050 4300 50  0001 C CNN
F 3 "~" H 6050 4300 50  0001 C CNN
	1    5850 4300
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PB0
U 1 1 5DEDD0B9
P 5950 4400
F 0 "PB0" H 6008 4518 50  0000 L CNN
F 1 "TestPoint" H 6008 4427 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6150 4400 50  0001 C CNN
F 3 "~" H 6150 4400 50  0001 C CNN
	1    5950 4400
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PB2
U 1 1 5DEDD2B2
P 6050 4500
F 0 "PB2" H 6108 4618 50  0000 L CNN
F 1 "TestPoint" H 6108 4527 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6250 4500 50  0001 C CNN
F 3 "~" H 6250 4500 50  0001 C CNN
	1    6050 4500
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PB11
U 1 1 5DEDD56E
P 6150 4600
F 0 "PB11" H 6208 4718 50  0000 L CNN
F 1 "TestPoint" H 6208 4627 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6350 4600 50  0001 C CNN
F 3 "~" H 6350 4600 50  0001 C CNN
	1    6150 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 3900 5400 3900
Wire Wire Line
	5500 4000 5300 4000
Wire Wire Line
	5300 4100 5650 4100
Wire Wire Line
	5300 4200 5750 4200
Wire Wire Line
	5850 4300 5300 4300
Wire Wire Line
	5950 4400 5300 4400
Wire Wire Line
	5300 4500 6050 4500
Wire Wire Line
	6150 4600 5300 4600
$Comp
L Connector:TestPoint PB13
U 1 1 5DEE3A62
P 6250 4700
F 0 "PB13" H 6308 4818 50  0000 L CNN
F 1 "TestPoint" H 6308 4727 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6450 4700 50  0001 C CNN
F 3 "~" H 6450 4700 50  0001 C CNN
	1    6250 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 4700 5300 4700
$Comp
L Connector:TestPoint PB15
U 1 1 5DEE4A4C
P 6350 4800
F 0 "PB15" H 6408 4918 50  0000 L CNN
F 1 "TestPoint" H 6408 4827 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6550 4800 50  0001 C CNN
F 3 "~" H 6550 4800 50  0001 C CNN
	1    6350 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 4800 5300 4800
$Comp
L Connector:TestPoint GND1
U 1 1 5DEE7DDD
P 8700 4000
F 0 "GND1" H 8450 4150 50  0000 L CNN
F 1 "TestPoint" H 8758 4027 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8900 4000 50  0001 C CNN
F 3 "~" H 8900 4000 50  0001 C CNN
	1    8700 4000
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PA3
U 1 1 5DEE95A3
P 8600 4100
F 0 "PA3" H 8400 4250 50  0000 L CNN
F 1 "TestPoint" H 8658 4127 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8800 4100 50  0001 C CNN
F 3 "~" H 8800 4100 50  0001 C CNN
	1    8600 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4100 8600 4100
$Comp
L Connector:TestPoint PA5
U 1 1 5DEEB1FE
P 8500 4200
F 0 "PA5" H 8300 4350 50  0000 L CNN
F 1 "TestPoint" H 8558 4227 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8700 4200 50  0001 C CNN
F 3 "~" H 8700 4200 50  0001 C CNN
	1    8500 4200
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PA7
U 1 1 5DEEB4E8
P 8400 4300
F 0 "PA7" H 8200 4450 50  0000 L CNN
F 1 "TestPoint" H 8458 4327 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8600 4300 50  0001 C CNN
F 3 "~" H 8600 4300 50  0001 C CNN
	1    8400 4300
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint PB1
U 1 1 5DEEB8CB
P 8300 4400
F 0 "PB1" H 8100 4550 50  0000 L CNN
F 1 "TestPoint" H 8358 4427 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8500 4400 50  0001 C CNN
F 3 "~" H 8500 4400 50  0001 C CNN
	1    8300 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8500 4200 9050 4200
Wire Wire Line
	9050 4300 8400 4300
Wire Wire Line
	9050 4400 8300 4400
$Comp
L Connector:TestPoint PB10
U 1 1 5DEEEAF7
P 8200 4500
F 0 "PB10" H 7950 4650 50  0000 L CNN
F 1 "TestPoint" H 8258 4527 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8400 4500 50  0001 C CNN
F 3 "~" H 8400 4500 50  0001 C CNN
	1    8200 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4500 8200 4500
$Comp
L Connector:TestPoint PB12
U 1 1 5DEF02F5
P 8100 4600
F 0 "PB12" H 7850 4750 50  0000 L CNN
F 1 "TestPoint" H 8158 4627 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8300 4600 50  0001 C CNN
F 3 "~" H 8300 4600 50  0001 C CNN
	1    8100 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4600 8100 4600
Wire Wire Line
	9050 4700 7950 4700
$Comp
L Connector:TestPoint PB14
U 1 1 5DEF3022
P 7950 4700
F 0 "PB14" H 7700 4850 50  0000 L CNN
F 1 "TestPoint" H 8008 4727 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8150 4700 50  0001 C CNN
F 3 "~" H 8150 4700 50  0001 C CNN
	1    7950 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4800 7800 4800
$Comp
L Connector:TestPoint PA8
U 1 1 5DEF46D2
P 7800 4800
F 0 "PA8" H 7600 4950 50  0000 L CNN
F 1 "TestPoint" H 7858 4827 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 8000 4800 50  0001 C CNN
F 3 "~" H 8000 4800 50  0001 C CNN
	1    7800 4800
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint JTMS1
U 1 1 5DEF5F67
P 7700 4900
F 0 "JTMS1" H 7400 5050 50  0000 L CNN
F 1 "TestPoint" H 7758 4927 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 7900 4900 50  0001 C CNN
F 3 "~" H 7900 4900 50  0001 C CNN
	1    7700 4900
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint nRST1
U 1 1 5DEF6F64
P 6450 4900
F 0 "nRST1" H 6508 5018 50  0000 L CNN
F 1 "TestPoint" H 6508 4927 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6650 4900 50  0001 C CNN
F 3 "~" H 6650 4900 50  0001 C CNN
	1    6450 4900
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint JTDI1
U 1 1 5DEF70C0
P 6550 5000
F 0 "JTDI1" H 6608 5118 50  0000 L CNN
F 1 "TestPoint" H 6608 5027 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6750 5000 50  0001 C CNN
F 3 "~" H 6750 5000 50  0001 C CNN
	1    6550 5000
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint JTDO1
U 1 1 5DEF729C
P 6700 5100
F 0 "JTDO1" H 6758 5218 50  0000 L CNN
F 1 "TestPoint" H 6758 5127 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6900 5100 50  0001 C CNN
F 3 "~" H 6900 5100 50  0001 C CNN
	1    6700 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 4900 6450 4900
Wire Wire Line
	5150 5000 6550 5000
Wire Wire Line
	5150 5100 6700 5100
Wire Wire Line
	7700 4900 9250 4900
Wire Wire Line
	7650 5000 9250 5000
$Comp
L Connector:TestPoint JTCK1
U 1 1 5DF03EBE
P 7650 5000
F 0 "JTCK1" H 7350 5150 50  0000 L CNN
F 1 "TestPoint" H 7708 5027 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 7850 5000 50  0001 C CNN
F 3 "~" H 7850 5000 50  0001 C CNN
	1    7650 5000
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint JTRST1
U 1 1 5DF04051
P 7550 5100
F 0 "JTRST1" H 7200 5200 50  0000 L CNN
F 1 "TestPoint" H 7608 5127 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 7750 5100 50  0001 C CNN
F 3 "~" H 7750 5100 50  0001 C CNN
	1    7550 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 5100 9250 5100
$Comp
L Connector:TestPoint GND2
U 1 1 5DF49BD6
P 9650 3900
F 0 "GND2" H 9708 4018 50  0000 L CNN
F 1 "TestPoint" H 9708 3927 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 9850 3900 50  0001 C CNN
F 3 "~" H 9850 3900 50  0001 C CNN
	1    9650 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 4000 9350 4000
$Comp
L power:GND #PWR0106
U 1 1 5DECB5F5
P 9400 4000
AR Path="/5DECB5F5" Ref="#PWR0106"  Part="1" 
AR Path="/5D8BFFCE/5DECB5F5" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DECB5F5" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DECB5F5" Ref="#PWR?"  Part="1" 
F 0 "#PWR0106" H 9400 3750 50  0001 C CNN
F 1 "GND" H 9500 4000 50  0000 C CNN
F 2 "" H 9400 4000 50  0001 C CNN
F 3 "" H 9400 4000 50  0001 C CNN
	1    9400 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	9650 3900 9350 3900
Wire Wire Line
	9350 3900 9350 4000
Connection ~ 9350 4000
Wire Wire Line
	9350 4000 9400 4000
$EndSCHEMATC
