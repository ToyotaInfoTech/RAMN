EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Chassis - RAMN: Resistant Automotive Miniature Network V1"
Date "2021-02-22"
Rev "A"
Comp "Copyright (c) 2021 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED."
Comment1 "https://github.com/AkiyukiOkayasu/Kicad_Akiyuki_Symbol"
Comment2 "NKK NR01 symbol from Akiyuki Okayasu"
Comment3 "License: CC BY-SA 4.0"
Comment4 "https://github.com/toyotainfotech/ramn"
$EndDescr
$Comp
L Connector_Generic:Conn_02x13_Odd_Even J7
U 1 1 5D87B279
P 3700 2700
F 0 "J7" H 3750 3400 50  0000 C CNN
F 1 " " H 3750 3226 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x13_P2.54mm_Vertical" H 3700 2700 50  0001 C CNN
F 3 "~" H 3700 2700 50  0001 C CNN
	1    3700 2700
	1    0    0    -1  
$EndComp
Text Label 3500 2100 2    50   ~ 0
3V3_ECU
Wire Wire Line
	4000 2200 4350 2200
$Comp
L power:GND #PWR02
U 1 1 5FC10AC8
P 4350 2200
AR Path="/5FC10AC8" Ref="#PWR02"  Part="1" 
AR Path="/5D8BFFCE/5FC10AC8" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5FC10AC8" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5FC10AC8" Ref="#PWR?"  Part="1" 
F 0 "#PWR02" H 4350 1950 50  0001 C CNN
F 1 "GND" H 4450 2200 50  0000 C CNN
F 2 "" H 4350 2200 50  0001 C CNN
F 3 "" H 4350 2200 50  0001 C CNN
	1    4350 2200
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 60225AF7
P 4000 2100
AR Path="/5D8BFFCE/60225AF7" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/60225AF7" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/60225AF7" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/60225AF7" Ref="#PWR?"  Part="1" 
AR Path="/60225AF7" Ref="#PWR01"  Part="1" 
F 0 "#PWR01" H 4000 1950 50  0001 C CNN
F 1 "+5V" H 4015 2273 50  0000 C CNN
F 2 "" H 4000 2100 50  0001 C CNN
F 3 "" H 4000 2100 50  0001 C CNN
	1    4000 2100
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H3
U 1 1 60225AF5
P 1050 1550
F 0 "H3" H 1150 1596 50  0000 L CNN
F 1 "MountingHole" H 1150 1505 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_ISO7380" H 1050 1550 50  0001 C CNN
F 3 "~" H 1050 1550 50  0001 C CNN
	1    1050 1550
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H4
U 1 1 5FC10AC6
P 1050 1750
F 0 "H4" H 1150 1796 50  0000 L CNN
F 1 "MountingHole" H 1150 1705 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_ISO7380" H 1050 1750 50  0001 C CNN
F 3 "~" H 1050 1750 50  0001 C CNN
	1    1050 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 1850 6700 2050
Text Label 6700 1850 2    50   ~ 0
3V3_ECU
Wire Wire Line
	6700 2350 6700 2500
$Comp
L power:GND #PWR03
U 1 1 5D829CA5
P 6700 2500
AR Path="/5D829CA5" Ref="#PWR03"  Part="1" 
AR Path="/5D8BFFCE/5D829CA5" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D829CA5" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D829CA5" Ref="#PWR?"  Part="1" 
F 0 "#PWR03" H 6700 2250 50  0001 C CNN
F 1 "GND" H 6800 2500 50  0000 C CNN
F 2 "" H 6700 2500 50  0001 C CNN
F 3 "" H 6700 2500 50  0001 C CNN
	1    6700 2500
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT_TRIM_US RV3
U 1 1 5D829CAB
P 6700 2200
F 0 "RV3" H 6632 2246 50  0000 R CNN
F 1 "R_POT_TRIM_US" H 6632 2155 50  0000 R CNN
F 2 "custom:RK09K1110A2S" H 6700 2200 50  0001 C CNN
F 3 "~" H 6700 2200 50  0001 C CNN
	1    6700 2200
	-1   0    0    -1  
$EndComp
NoConn ~ 4000 3100
NoConn ~ 4000 3200
NoConn ~ 4000 3300
NoConn ~ 3500 3300
NoConn ~ 3500 3200
NoConn ~ 3500 3100
Text GLabel 5300 2200 0    50   Input ~ 0
Wheel
Wire Wire Line
	5300 2200 5400 2200
Text GLabel 2900 2200 0    50   Input ~ 0
Wheel
Text GLabel 4300 2400 2    50   Input ~ 0
Lamp
Wire Wire Line
	4300 2400 4000 2400
Wire Wire Line
	3500 2400 3200 2400
Wire Wire Line
	2900 2200 3500 2200
NoConn ~ 4000 3000
NoConn ~ 4000 2900
NoConn ~ 4000 2800
NoConn ~ 4000 2600
NoConn ~ 4000 2500
NoConn ~ 4000 2300
NoConn ~ 3500 2300
NoConn ~ 3500 2500
NoConn ~ 3500 2600
NoConn ~ 3500 2900
NoConn ~ 3500 3000
$Comp
L Akiyuki_UI:NKK_NR01 SW3
U 1 1 5FC0F272
P 4450 4500
F 0 "SW3" H 4450 4875 50  0000 C CNN
F 1 "NKK_NR01" H 4450 4784 50  0000 C CNN
F 2 "Akiyuki_UI:NR01" H 4425 4525 50  0001 C CNN
F 3 "" H 4425 4525 50  0001 C CNN
	1    4450 4500
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R?
U 1 1 5FC11ACA
P 3800 4150
AR Path="/5FC10863/5FC11ACA" Ref="R?"  Part="1" 
AR Path="/5FC11ACA" Ref="R7"  Part="1" 
F 0 "R7" H 3868 4196 50  0000 L CNN
F 1 "10k" H 3868 4105 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 3800 4150 50  0001 C CNN
F 3 "~" H 3800 4150 50  0001 C CNN
	1    3800 4150
	1    0    0    -1  
$EndComp
Text GLabel 3200 2400 0    50   Input ~ 0
SW1
NoConn ~ 3500 2800
$Comp
L power:GND #PWR05
U 1 1 5FC17AC1
P 3800 5450
AR Path="/5FC17AC1" Ref="#PWR05"  Part="1" 
AR Path="/5D8BFFCE/5FC17AC1" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5FC17AC1" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5FC17AC1" Ref="#PWR?"  Part="1" 
F 0 "#PWR05" H 3800 5200 50  0001 C CNN
F 1 "GND" H 3900 5450 50  0000 C CNN
F 2 "" H 3800 5450 50  0001 C CNN
F 3 "" H 3800 5450 50  0001 C CNN
	1    3800 5450
	1    0    0    -1  
$EndComp
Text GLabel 5600 4500 2    50   Input ~ 0
Lamp
Text Label 3800 3800 0    50   ~ 0
3V3_ECU
Wire Wire Line
	4800 4500 5150 4500
Wire Wire Line
	4100 4350 3800 4350
$Comp
L Device:R_Small_US R?
U 1 1 5FC1B743
P 3800 4500
AR Path="/5FC10863/5FC1B743" Ref="R?"  Part="1" 
AR Path="/5FC1B743" Ref="R8"  Part="1" 
F 0 "R8" H 3868 4546 50  0000 L CNN
F 1 "10k" H 3868 4455 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 3800 4500 50  0001 C CNN
F 3 "~" H 3800 4500 50  0001 C CNN
	1    3800 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 4400 3800 4350
Wire Wire Line
	4100 4500 4050 4500
Wire Wire Line
	4050 4500 4050 4600
Wire Wire Line
	4050 4600 3800 4600
$Comp
L Device:R_Small_US R?
U 1 1 5FC1C8F6
P 3800 4750
AR Path="/5FC10863/5FC1C8F6" Ref="R?"  Part="1" 
AR Path="/5FC1C8F6" Ref="R9"  Part="1" 
F 0 "R9" H 3868 4796 50  0000 L CNN
F 1 "10k" H 3868 4705 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 3800 4750 50  0001 C CNN
F 3 "~" H 3800 4750 50  0001 C CNN
	1    3800 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 4650 3800 4600
Connection ~ 3800 4600
Wire Wire Line
	3800 4850 4100 4850
Wire Wire Line
	4100 4850 4100 4650
$Comp
L Device:R_Small_US R?
U 1 1 5FC1E6F1
P 3800 5050
AR Path="/5FC10863/5FC1E6F1" Ref="R?"  Part="1" 
AR Path="/5FC1E6F1" Ref="R10"  Part="1" 
F 0 "R10" H 3868 5096 50  0000 L CNN
F 1 "10k" H 3868 5005 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 3800 5050 50  0001 C CNN
F 3 "~" H 3800 5050 50  0001 C CNN
	1    3800 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 4950 3800 4850
Connection ~ 3800 4850
Wire Wire Line
	4800 4650 4800 5300
Wire Wire Line
	4800 5300 3800 5300
Wire Wire Line
	3800 5300 3800 5450
Wire Wire Line
	4800 4350 4800 3900
Wire Wire Line
	4800 3900 3800 3900
Connection ~ 3800 3900
Wire Wire Line
	3800 3900 3800 3800
Wire Wire Line
	3800 4350 3800 4250
Connection ~ 3800 4350
Wire Wire Line
	3800 3900 3800 4050
Wire Wire Line
	3800 5150 3800 5300
Connection ~ 3800 5300
NoConn ~ 4000 2700
Wire Wire Line
	4700 6000 4700 5900
Text Label 4700 5900 2    50   ~ 0
3V3_ECU
Wire Wire Line
	4700 6300 4850 6300
Connection ~ 4700 6300
Wire Wire Line
	4700 6200 4700 6300
Wire Wire Line
	4450 6300 4550 6300
$Comp
L Device:R_Small_US R11
U 1 1 5FC76E07
P 4700 6100
F 0 "R11" H 4900 6050 50  0000 R CNN
F 1 "10k" H 4900 6150 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 4700 6100 50  0001 C CNN
F 3 "~" H 4700 6100 50  0001 C CNN
	1    4700 6100
	-1   0    0    1   
$EndComp
Text GLabel 4450 6300 0    50   Input ~ 0
SW1
NoConn ~ 5250 6200
Wire Wire Line
	5350 6400 5250 6400
$Comp
L power:GND #PWR06
U 1 1 5D821982
P 5350 6400
AR Path="/5D821982" Ref="#PWR06"  Part="1" 
AR Path="/5D8BFFCE/5D821982" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D821982" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D821982" Ref="#PWR?"  Part="1" 
F 0 "#PWR06" H 5350 6150 50  0001 C CNN
F 1 "GND" H 5450 6400 50  0000 C CNN
F 2 "" H 5350 6400 50  0001 C CNN
F 3 "" H 5350 6400 50  0001 C CNN
	1    5350 6400
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW2
U 1 1 5D823D3F
P 5050 6300
F 0 "SW2" H 5050 6585 50  0000 C CNN
F 1 " " H 5050 6494 50  0000 C CNN
F 2 "Button_Switch_THT:SW_CuK_OS102011MA1QN1_SPDT_Angled" H 5050 6500 50  0001 C CNN
F 3 "~" H 5050 6500 50  0001 C CNN
	1    5050 6300
	1    0    0    -1  
$EndComp
NoConn ~ 3500 2700
$Comp
L Device:C_Small C4
U 1 1 602262C9
P 5400 2350
F 0 "C4" H 5492 2396 50  0000 L CNN
F 1 "100nF" H 5492 2305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5400 2350 50  0001 C CNN
F 3 "~" H 5400 2350 50  0001 C CNN
	1    5400 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 2250 5400 2200
$Comp
L power:GND #PWR0101
U 1 1 60227372
P 5400 2500
AR Path="/60227372" Ref="#PWR0101"  Part="1" 
AR Path="/5D8BFFCE/60227372" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/60227372" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/60227372" Ref="#PWR?"  Part="1" 
F 0 "#PWR0101" H 5400 2250 50  0001 C CNN
F 1 "GND" H 5500 2500 50  0000 C CNN
F 2 "" H 5400 2500 50  0001 C CNN
F 3 "" H 5400 2500 50  0001 C CNN
	1    5400 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 2500 5400 2450
Text Label 3500 2200 2    50   ~ 0
PA1
Text Label 3450 2400 2    50   ~ 0
PA4
Text Label 4050 2400 0    50   ~ 0
PA5
$Comp
L Device:D_TVS TVS4
U 1 1 602E2082
P 5950 2350
F 0 "TVS4" V 5904 2430 50  0000 L CNN
F 1 "D_TVS" V 5995 2430 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-123F" H 5950 2350 50  0001 C CNN
F 3 "~" H 5950 2350 50  0001 C CNN
	1    5950 2350
	0    1    1    0   
$EndComp
Connection ~ 5400 2200
$Comp
L power:GND #PWR0102
U 1 1 602E4F84
P 5950 2500
AR Path="/602E4F84" Ref="#PWR0102"  Part="1" 
AR Path="/5D8BFFCE/602E4F84" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/602E4F84" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/602E4F84" Ref="#PWR?"  Part="1" 
F 0 "#PWR0102" H 5950 2250 50  0001 C CNN
F 1 "GND" H 6050 2500 50  0000 C CNN
F 2 "" H 5950 2500 50  0001 C CNN
F 3 "" H 5950 2500 50  0001 C CNN
	1    5950 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 2200 5950 2200
Connection ~ 5950 2200
Wire Wire Line
	5950 2200 6550 2200
$Comp
L Device:D_TVS TVS5
U 1 1 602E62CB
P 5150 4700
F 0 "TVS5" V 5104 4780 50  0000 L CNN
F 1 "D_TVS" V 5195 4780 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-123F" H 5150 4700 50  0001 C CNN
F 3 "~" H 5150 4700 50  0001 C CNN
	1    5150 4700
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 602E62D1
P 5150 4850
AR Path="/602E62D1" Ref="#PWR0103"  Part="1" 
AR Path="/5D8BFFCE/602E62D1" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/602E62D1" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/602E62D1" Ref="#PWR?"  Part="1" 
F 0 "#PWR0103" H 5150 4600 50  0001 C CNN
F 1 "GND" H 5250 4850 50  0000 C CNN
F 2 "" H 5150 4850 50  0001 C CNN
F 3 "" H 5150 4850 50  0001 C CNN
	1    5150 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 4550 5150 4500
Connection ~ 5150 4500
Wire Wire Line
	5150 4500 5600 4500
$Comp
L Device:D_TVS TVS6
U 1 1 602E7F61
P 4550 6500
F 0 "TVS6" V 4504 6580 50  0000 L CNN
F 1 "D_TVS" V 4595 6580 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-123F" H 4550 6500 50  0001 C CNN
F 3 "~" H 4550 6500 50  0001 C CNN
	1    4550 6500
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 602E7F67
P 4550 6650
AR Path="/602E7F67" Ref="#PWR0104"  Part="1" 
AR Path="/5D8BFFCE/602E7F67" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/602E7F67" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/602E7F67" Ref="#PWR?"  Part="1" 
F 0 "#PWR0104" H 4550 6400 50  0001 C CNN
F 1 "GND" H 4650 6650 50  0000 C CNN
F 2 "" H 4550 6650 50  0001 C CNN
F 3 "" H 4550 6650 50  0001 C CNN
	1    4550 6650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 6350 4550 6300
Connection ~ 4550 6300
Wire Wire Line
	4550 6300 4700 6300
$Comp
L Mechanical:Fiducial FID1
U 1 1 603362AF
P 1000 6950
F 0 "FID1" H 1085 6996 50  0000 L CNN
F 1 "Fiducial" H 1085 6905 50  0000 L CNN
F 2 "Fiducial:Fiducial_1mm_Mask2mm" H 1000 6950 50  0001 C CNN
F 3 "~" H 1000 6950 50  0001 C CNN
	1    1000 6950
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:Fiducial FID2
U 1 1 60337903
P 1650 6950
F 0 "FID2" H 1735 6996 50  0000 L CNN
F 1 "Fiducial" H 1735 6905 50  0000 L CNN
F 2 "Fiducial:Fiducial_1mm_Mask2mm" H 1650 6950 50  0001 C CNN
F 3 "~" H 1650 6950 50  0001 C CNN
	1    1650 6950
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:Fiducial FID3
U 1 1 60337A1C
P 2300 6950
F 0 "FID3" H 2385 6996 50  0000 L CNN
F 1 "Fiducial" H 2385 6905 50  0000 L CNN
F 2 "Fiducial:Fiducial_1mm_Mask2mm" H 2300 6950 50  0001 C CNN
F 3 "~" H 2300 6950 50  0001 C CNN
	1    2300 6950
	1    0    0    -1  
$EndComp
$EndSCHEMATC
