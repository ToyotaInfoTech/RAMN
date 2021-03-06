EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Powertrain - RAMN: Resistant Automotive Miniature Network V1"
Date "2020-12-02"
Rev "A"
Comp "Copyright (c) 2020 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED."
Comment1 ""
Comment2 ""
Comment3 "License: CC BY-SA 4.0"
Comment4 "https://github.com/toyotainfotech/ramn"
$EndDescr
$Comp
L Connector_Generic:Conn_02x13_Odd_Even J6
U 1 1 5D87B279
P 3700 2700
F 0 "J6" H 3750 3400 50  0000 C CNN
F 1 " " H 3750 3226 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x13_P2.54mm_Vertical" H 3700 2700 50  0001 C CNN
F 3 "~" H 3700 2700 50  0001 C CNN
	1    3700 2700
	1    0    0    -1  
$EndComp
Text Label 4000 2400 0    50   ~ 0
PA5
Wire Wire Line
	4000 2200 4350 2200
$Comp
L power:GND #PWR0101
U 1 1 5D9A7138
P 4350 2200
AR Path="/5D9A7138" Ref="#PWR0101"  Part="1" 
AR Path="/5D8BFFCE/5D9A7138" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D9A7138" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D9A7138" Ref="#PWR?"  Part="1" 
F 0 "#PWR0101" H 4350 1950 50  0001 C CNN
F 1 "GND" H 4450 2200 50  0000 C CNN
F 2 "" H 4350 2200 50  0001 C CNN
F 3 "" H 4350 2200 50  0001 C CNN
	1    4350 2200
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5D9A713E
P 4000 2100
AR Path="/5D8BFFCE/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D9A713E" Ref="#PWR0102"  Part="1" 
F 0 "#PWR0102" H 4000 1950 50  0001 C CNN
F 1 "+5V" H 4015 2273 50  0000 C CNN
F 2 "" H 4000 2100 50  0001 C CNN
F 3 "" H 4000 2100 50  0001 C CNN
	1    4000 2100
	1    0    0    -1  
$EndComp
Text Label 3500 2200 2    50   ~ 0
PA1
Text Label 3500 2400 2    50   ~ 0
PA4
$Comp
L Mechanical:MountingHole H1
U 1 1 5D80EA60
P 1050 1550
F 0 "H1" H 1150 1596 50  0000 L CNN
F 1 "MountingHole" H 1150 1505 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_ISO7380" H 1050 1550 50  0001 C CNN
F 3 "~" H 1050 1550 50  0001 C CNN
	1    1050 1550
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 5D80EE26
P 1050 1750
F 0 "H2" H 1150 1796 50  0000 L CNN
F 1 "MountingHole" H 1150 1705 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_ISO7380" H 1050 1750 50  0001 C CNN
F 3 "~" H 1050 1750 50  0001 C CNN
	1    1050 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT_TRIM_US RV1
U 1 1 5D80FCC3
P 5600 1900
F 0 "RV1" H 5532 1946 50  0000 R CNN
F 1 "R_POT_TRIM_US" H 5532 1855 50  0000 R CNN
F 2 "Potentiometer_THT:Potentiometer_Bourns_PTA1543_Single_Slide" H 5600 1900 50  0001 C CNN
F 3 "~" H 5600 1900 50  0001 C CNN
	1    5600 1900
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5D810409
P 5600 2200
AR Path="/5D810409" Ref="#PWR0103"  Part="1" 
AR Path="/5D8BFFCE/5D810409" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D810409" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D810409" Ref="#PWR?"  Part="1" 
F 0 "#PWR0103" H 5600 1950 50  0001 C CNN
F 1 "GND" H 5700 2200 50  0000 C CNN
F 2 "" H 5600 2200 50  0001 C CNN
F 3 "" H 5600 2200 50  0001 C CNN
	1    5600 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 2050 5600 2200
Text Label 5600 1550 2    50   ~ 0
3V3_ECU
Wire Wire Line
	5600 1550 5600 1750
$Comp
L Device:R_POT_TRIM_US RV2
U 1 1 5D8112ED
P 7050 1900
F 0 "RV2" H 6982 1946 50  0000 R CNN
F 1 "R_POT_TRIM_US" H 6982 1855 50  0000 R CNN
F 2 "Potentiometer_THT:Potentiometer_Bourns_PTA1543_Single_Slide" H 7050 1900 50  0001 C CNN
F 3 "~" H 7050 1900 50  0001 C CNN
	1    7050 1900
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5D8112F3
P 7050 2200
AR Path="/5D8112F3" Ref="#PWR0104"  Part="1" 
AR Path="/5D8BFFCE/5D8112F3" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D8112F3" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D8112F3" Ref="#PWR?"  Part="1" 
F 0 "#PWR0104" H 7050 1950 50  0001 C CNN
F 1 "GND" H 7150 2200 50  0000 C CNN
F 2 "" H 7050 2200 50  0001 C CNN
F 3 "" H 7050 2200 50  0001 C CNN
	1    7050 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7050 2050 7050 2200
Text Label 7050 1550 2    50   ~ 0
3V3_ECU
Wire Wire Line
	7050 1550 7050 1750
Text GLabel 6650 1900 0    50   Input ~ 0
Brake
Text GLabel 5300 1900 0    50   Input ~ 0
Accel
Wire Wire Line
	5300 1900 5350 1900
Connection ~ 6100 4000
Text GLabel 3300 2200 0    50   Input ~ 0
Brake
Wire Wire Line
	3500 2200 3300 2200
Text GLabel 3300 2400 0    50   Input ~ 0
Accel
Wire Wire Line
	3300 2400 3500 2400
Text GLabel 4400 2400 2    50   Input ~ 0
JOYSTICK
Wire Wire Line
	4000 2400 4400 2400
NoConn ~ 3500 2300
NoConn ~ 3500 2500
NoConn ~ 3500 2600
NoConn ~ 3500 2700
NoConn ~ 3500 2800
NoConn ~ 3500 2900
NoConn ~ 3500 3000
NoConn ~ 3500 3100
NoConn ~ 3500 3200
NoConn ~ 3500 3300
NoConn ~ 4000 3300
NoConn ~ 4000 3200
NoConn ~ 4000 3100
NoConn ~ 4000 3000
NoConn ~ 4000 2900
NoConn ~ 4000 2800
NoConn ~ 4000 2700
NoConn ~ 4000 2600
NoConn ~ 4000 2500
NoConn ~ 4000 2300
$Comp
L power:GND #PWR0110
U 1 1 5DF8242C
P 4800 3800
AR Path="/5DF8242C" Ref="#PWR0110"  Part="1" 
AR Path="/5D8BFFCE/5DF8242C" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF8242C" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF8242C" Ref="#PWR?"  Part="1" 
F 0 "#PWR0110" H 4800 3550 50  0001 C CNN
F 1 "GND" H 4900 3800 50  0000 C CNN
F 2 "" H 4800 3800 50  0001 C CNN
F 3 "" H 4800 3800 50  0001 C CNN
	1    4800 3800
	1    0    0    -1  
$EndComp
Text Label 3950 3650 2    50   ~ 0
3V3_ECU
Wire Wire Line
	3950 3650 4200 3650
Wire Wire Line
	4800 3650 4800 3800
Wire Wire Line
	5350 1800 5350 1900
Connection ~ 5350 1900
Wire Wire Line
	5350 1900 5450 1900
Wire Wire Line
	6650 1900 6750 1900
Wire Wire Line
	6750 1800 6750 1900
Connection ~ 6750 1900
Wire Wire Line
	6750 1900 6900 1900
Text Label 3500 2100 2    50   ~ 0
3V3_ECU
$Comp
L power:GND #PWR0105
U 1 1 5FC08D43
P 6100 5600
AR Path="/5FC08D43" Ref="#PWR0105"  Part="1" 
AR Path="/5D8BFFCE/5FC08D43" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5FC08D43" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5FC08D43" Ref="#PWR?"  Part="1" 
F 0 "#PWR0105" H 6100 5350 50  0001 C CNN
F 1 "GND" H 6200 5600 50  0000 C CNN
F 2 "" H 6100 5600 50  0001 C CNN
F 3 "" H 6100 5600 50  0001 C CNN
	1    6100 5600
	1    0    0    -1  
$EndComp
Text Notes 8450 4050 0    50   ~ 0
UP
Text Notes 8400 4250 0    50   ~ 0
LEFT
Text Notes 8350 4450 0    50   ~ 0
DOWN
Text Notes 8350 4850 0    50   ~ 0
RIGHT
Text Notes 8350 5000 0    50   ~ 0
PUSH
Wire Wire Line
	6100 4000 6100 4100
Wire Wire Line
	6100 4300 6100 4350
Wire Wire Line
	6100 4350 6500 4350
Wire Wire Line
	6500 4350 6500 4200
Wire Wire Line
	6500 4200 7900 4200
Connection ~ 6100 4350
Wire Wire Line
	6100 4350 6100 4400
Wire Wire Line
	6650 4400 7900 4400
Wire Wire Line
	6100 4700 6100 4650
Wire Wire Line
	6100 4650 6650 4650
Wire Wire Line
	6650 4650 6650 4400
Connection ~ 6100 4650
Wire Wire Line
	6100 4650 6100 4600
Wire Wire Line
	7900 4800 6800 4800
Wire Wire Line
	6800 4800 6800 4950
Wire Wire Line
	6800 4950 6100 4950
Wire Wire Line
	6100 4950 6100 4900
Wire Wire Line
	6100 5000 6100 4950
Connection ~ 6100 4950
Wire Wire Line
	6100 5300 6100 5250
Wire Wire Line
	6100 5250 7000 5250
Wire Wire Line
	7000 5250 7000 5000
Wire Wire Line
	7000 5000 7900 5000
Connection ~ 6100 5250
Wire Wire Line
	6100 5250 6100 5200
Wire Wire Line
	6100 5600 6100 5500
$Comp
L Connector:TestPoint BRAKE
U 1 1 5DF85F29
P 6750 1800
F 0 "BRAKE" H 6808 1918 50  0000 L CNN
F 1 "TestPoint" H 6808 1827 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6950 1800 50  0001 C CNN
F 3 "~" H 6950 1800 50  0001 C CNN
	1    6750 1800
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint ACCEL
U 1 1 5DF83B8A
P 5350 1800
F 0 "ACCEL" H 5408 1918 50  0000 L CNN
F 1 "TestPoint" H 5408 1827 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 5550 1800 50  0001 C CNN
F 3 "~" H 5550 1800 50  0001 C CNN
	1    5350 1800
	1    0    0    -1  
$EndComp
$Comp
L Connector:TestPoint 3V3
U 1 1 5DF81D44
P 4200 3650
F 0 "3V3" H 4258 3768 50  0000 L CNN
F 1 "TestPoint" H 4258 3677 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 4400 3650 50  0001 C CNN
F 3 "~" H 4400 3650 50  0001 C CNN
	1    4200 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R6
U 1 1 5FC08602
P 6100 5400
F 0 "R6" H 6168 5446 50  0000 L CNN
F 1 "10k" H 6168 5355 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 6100 5400 50  0001 C CNN
F 3 "~" H 6100 5400 50  0001 C CNN
	1    6100 5400
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R5
U 1 1 5FC051D6
P 6100 5100
F 0 "R5" H 6168 5146 50  0000 L CNN
F 1 "10k" H 6168 5055 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 6100 5100 50  0001 C CNN
F 3 "~" H 6100 5100 50  0001 C CNN
	1    6100 5100
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R4
U 1 1 5FC01FBF
P 6100 4800
F 0 "R4" H 6168 4846 50  0000 L CNN
F 1 "10k" H 6168 4755 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 6100 4800 50  0001 C CNN
F 3 "~" H 6100 4800 50  0001 C CNN
	1    6100 4800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R3
U 1 1 5FC01C49
P 6100 4500
F 0 "R3" H 6168 4546 50  0000 L CNN
F 1 "10k" H 6168 4455 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 6100 4500 50  0001 C CNN
F 3 "~" H 6100 4500 50  0001 C CNN
	1    6100 4500
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R2
U 1 1 5FC01876
P 6100 4200
F 0 "R2" H 6168 4246 50  0000 L CNN
F 1 "10k" H 6168 4155 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 6100 4200 50  0001 C CNN
F 3 "~" H 6100 4200 50  0001 C CNN
	1    6100 4200
	1    0    0    -1  
$EndComp
$Comp
L SKQUAAA010:SKQUAAA010 SW1
U 1 1 5FBFA995
P 8700 4200
F 0 "SW1" H 8100 4550 50  0000 L BNN
F 1 "SKQUAAA010" H 8700 4200 50  0001 L BNN
F 2 "SKQUAAA010" H 8700 4200 50  0001 L BNN
F 3 "" H 8700 4200 50  0001 L BNN
	1    8700 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 4000 6100 4000
Wire Wire Line
	6100 3200 6100 3350
Text Label 6100 3200 2    50   ~ 0
3V3_ECU
Wire Wire Line
	6250 4000 7900 4000
Wire Wire Line
	6250 4000 6100 4000
Connection ~ 6250 4000
Wire Wire Line
	6250 3800 6250 4000
$Comp
L Connector:TestPoint SHIFT
U 1 1 5DF7EE51
P 6250 3800
F 0 "SHIFT" H 6308 3918 50  0000 L CNN
F 1 "TestPoint" H 6308 3827 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 6450 3800 50  0001 C CNN
F 3 "~" H 6450 3800 50  0001 C CNN
	1    6250 3800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R1
U 1 1 5DF64316
P 6100 3450
F 0 "R1" H 6168 3496 50  0000 L CNN
F 1 "10k" H 6168 3405 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 6100 3450 50  0001 C CNN
F 3 "~" H 6100 3450 50  0001 C CNN
	1    6100 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	7650 4600 7900 4600
$Comp
L power:GND #PWR0106
U 1 1 5FC2DAB8
P 7650 4600
AR Path="/5FC2DAB8" Ref="#PWR0106"  Part="1" 
AR Path="/5D8BFFCE/5FC2DAB8" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5FC2DAB8" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5FC2DAB8" Ref="#PWR?"  Part="1" 
F 0 "#PWR0106" H 7650 4350 50  0001 C CNN
F 1 "GND" H 7750 4600 50  0000 C CNN
F 2 "" H 7650 4600 50  0001 C CNN
F 3 "" H 7650 4600 50  0001 C CNN
	1    7650 4600
	0    1    1    0   
$EndComp
Text GLabel 5850 4000 0    50   Input ~ 0
JOYSTICK
Wire Wire Line
	6100 4000 6100 3550
$Comp
L Connector:TestPoint GND
U 1 1 5DF8168E
P 4800 3650
F 0 "GND" H 4858 3768 50  0000 L CNN
F 1 "TestPoint" H 4858 3677 50  0001 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 5000 3650 50  0001 C CNN
F 3 "~" H 5000 3650 50  0001 C CNN
	1    4800 3650
	1    0    0    -1  
$EndComp
$EndSCHEMATC
