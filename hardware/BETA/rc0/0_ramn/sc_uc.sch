EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 8 9
Title "RAMN: Resistant Automotive Miniature Network V1.0"
Date "2020-12-02"
Rev "A"
Comp "Copyright (c) 2020 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED."
Comment1 ""
Comment2 ""
Comment3 "License: CC BY-SA 4.0"
Comment4 "https://github.com/toyotainfotech/ramn"
$EndDescr
$Comp
L power:GND #PWR?
U 1 1 5D7F2751
P 2500 2350
AR Path="/5D7F2751" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2751" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F2751" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F2751" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F2751" Ref="#PWR066"  Part="1" 
F 0 "#PWR066" H 2500 2100 50  0001 C CNN
F 1 "GND" H 2505 2177 50  0000 C CNN
F 2 "" H 2500 2350 50  0001 C CNN
F 3 "" H 2500 2350 50  0001 C CNN
	1    2500 2350
	1    0    0    -1  
$EndComp
Text Notes 2700 2450 0    50   ~ 0
internal pull-ups\nSTBY high -> transceiver OFF
Wire Wire Line
	2000 2150 1850 2150
Wire Wire Line
	950  2050 2000 2050
Text Label 1900 1750 2    50   ~ 0
CAN1_TX
Text Label 1900 1850 2    50   ~ 0
CAN1_RX
Text Label 1750 2150 2    50   ~ 0
CAN_STB
Wire Wire Line
	3550 2050 3950 2050
Connection ~ 3550 2050
Wire Wire Line
	3550 2150 3550 2050
Wire Wire Line
	3000 2050 3550 2050
Wire Wire Line
	3550 2150 3950 2150
Text HLabel 3950 2150 2    50   Output ~ 0
CANL_OUT
Text HLabel 3950 2050 2    50   Input ~ 0
CANL_IN
Wire Wire Line
	3550 1850 3950 1850
Wire Wire Line
	3550 1750 3950 1750
Text HLabel 3950 1850 2    50   Output ~ 0
CANH_OUT
Text HLabel 3950 1750 2    50   Input ~ 0
CANH_IN
Connection ~ 3550 1850
Wire Wire Line
	3550 1850 3550 1750
Connection ~ 950  2050
Wire Wire Line
	950  2000 950  2050
$Comp
L power:+5V #PWR?
U 1 1 5D7F276F
P 2500 1050
AR Path="/5D8BFFCE/5D7F276F" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F276F" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F276F" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F276F" Ref="#PWR065"  Part="1" 
F 0 "#PWR065" H 2500 900 50  0001 C CNN
F 1 "+5V" H 2515 1223 50  0000 C CNN
F 2 "" H 2500 1050 50  0001 C CNN
F 3 "" H 2500 1050 50  0001 C CNN
	1    2500 1050
	1    0    0    -1  
$EndComp
Connection ~ 2500 1150
Wire Wire Line
	2500 1050 2500 1150
Wire Wire Line
	2500 1150 2500 1550
Wire Wire Line
	2750 1150 2500 1150
Wire Wire Line
	2750 1350 2750 1400
$Comp
L power:GND #PWR?
U 1 1 5D7F277A
P 2750 1400
AR Path="/5D7F277A" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F277A" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F277A" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F277A" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F277A" Ref="#PWR067"  Part="1" 
F 0 "#PWR067" H 2750 1150 50  0001 C CNN
F 1 "GND" H 2755 1227 50  0000 C CNN
F 2 "" H 2750 1400 50  0001 C CNN
F 3 "" H 2750 1400 50  0001 C CNN
	1    2750 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 5D7F2781
P 2750 1250
AR Path="/5D7F2781" Ref="C?"  Part="1" 
AR Path="/5D80E812/5D7F2781" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2781" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F2781" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F2781" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F2781" Ref="C34"  Part="1" 
F 0 "C34" H 2842 1296 50  0000 L CNN
F 1 "100nF" H 2842 1205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2750 1250 50  0001 C CNN
F 3 "~" H 2750 1250 50  0001 C CNN
F 4 "" H 2750 1250 50  0001 C CNN "not mounted"
	1    2750 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	950  2250 950  2300
$Comp
L power:GND #PWR?
U 1 1 5D7F2788
P 950 2300
AR Path="/5D7F2788" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2788" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F2788" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F2788" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F2788" Ref="#PWR062"  Part="1" 
F 0 "#PWR062" H 950 2050 50  0001 C CNN
F 1 "GND" H 955 2127 50  0000 C CNN
F 2 "" H 950 2300 50  0001 C CNN
F 3 "" H 950 2300 50  0001 C CNN
	1    950  2300
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 5D7F278F
P 950 2150
AR Path="/5D7F278F" Ref="C?"  Part="1" 
AR Path="/5D80E812/5D7F278F" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F278F" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F278F" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F278F" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F278F" Ref="C33"  Part="1" 
F 0 "C33" H 1042 2196 50  0000 L CNN
F 1 "100nF" H 1042 2105 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 950 2150 50  0001 C CNN
F 3 "~" H 950 2150 50  0001 C CNN
F 4 "" H 950 2150 50  0001 C CNN "not mounted"
	1    950  2150
	1    0    0    -1  
$EndComp
$Comp
L Interface_CAN_LIN:MCP2562-E-SN U?
U 1 1 5D7F2795
P 2500 1950
AR Path="/5D8BFFCE/5D7F2795" Ref="U?"  Part="1" 
AR Path="/5D815E09/5D7F2795" Ref="U?"  Part="1" 
AR Path="/5D8EDE42/5D7F2795" Ref="U?"  Part="1" 
AR Path="/5D7DEA89/5D7F2795" Ref="U10"  Part="1" 
F 0 "U10" H 2000 2450 50  0000 C CNN
F 1 "ATA6561-GAQW-N" H 2050 2350 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 2500 1450 50  0001 C CIN
F 3 "" H 2500 1950 50  0001 C CNN
	1    2500 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8300 4250 8450 4250
Wire Wire Line
	8300 4150 8450 4150
Wire Wire Line
	8300 4050 8450 4050
Wire Wire Line
	8250 5550 8450 5550
Wire Wire Line
	8250 5450 8450 5450
Wire Wire Line
	8250 5350 8450 5350
Wire Wire Line
	8250 5250 8450 5250
Wire Wire Line
	8250 5150 8450 5150
Wire Wire Line
	8250 5050 8450 5050
Wire Wire Line
	9650 4850 9800 4850
Wire Wire Line
	9650 4750 9800 4750
Wire Wire Line
	9650 4650 9800 4650
Wire Wire Line
	9650 4550 9800 4550
Wire Wire Line
	9650 4450 9800 4450
Wire Wire Line
	9650 4350 9800 4350
Wire Wire Line
	9650 4250 9800 4250
Text Label 9800 4850 0    50   ~ 0
PA8
Text Label 8250 5550 2    50   ~ 0
PB15
Text Label 8250 5450 2    50   ~ 0
PB14
Text Label 8250 5350 2    50   ~ 0
PB13
Text Label 8250 5250 2    50   ~ 0
PB12
Text Label 8250 5150 2    50   ~ 0
PB11
Text Label 8250 5050 2    50   ~ 0
PB10
Text Label 8300 4250 2    50   ~ 0
PB2
Text Label 8300 4150 2    50   ~ 0
PB1
Text Label 8300 4050 2    50   ~ 0
PB0
Text Label 9800 4750 0    50   ~ 0
PA7
Text Label 9800 4650 0    50   ~ 0
PA6
Text Label 9800 4550 0    50   ~ 0
PA5
Text Label 9800 4450 0    50   ~ 0
PA4
Text Label 9800 4350 0    50   ~ 0
PA3
Text Label 9800 4250 0    50   ~ 0
PA2
Text Label 9800 4150 0    50   ~ 0
PA1
Wire Wire Line
	9650 4150 9800 4150
Wire Wire Line
	8150 3450 8450 3450
Wire Wire Line
	9800 5150 9650 5150
Wire Wire Line
	9650 5250 9800 5250
Text Label 8400 3450 2    50   ~ 0
BOOT0
Wire Wire Line
	8450 3650 8400 3650
Text Label 9800 4950 0    50   ~ 0
CAN_STB
Wire Wire Line
	8250 4850 8450 4850
Text Label 8250 4850 2    50   ~ 0
CAN1_RX
Wire Wire Line
	8250 4950 8450 4950
Text Label 8250 4950 2    50   ~ 0
CAN1_TX
Wire Wire Line
	6400 3350 8450 3350
Wire Wire Line
	5750 3250 8450 3250
Wire Wire Line
	6200 3350 6400 3350
Wire Wire Line
	5750 3350 6000 3350
Connection ~ 5750 3350
$Comp
L power:GND #PWR?
U 1 1 5D7F27D8
P 5750 3650
AR Path="/5D7F27D8" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F27D8" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F27D8" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F27D8" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F27D8" Ref="#PWR069"  Part="1" 
F 0 "#PWR069" H 5750 3400 50  0001 C CNN
F 1 "GND" H 5755 3477 50  0000 C CNN
F 2 "" H 5750 3650 50  0001 C CNN
F 3 "" H 5750 3650 50  0001 C CNN
	1    5750 3650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5D7F27DE
P 6400 3650
AR Path="/5D7F27DE" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F27DE" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F27DE" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F27DE" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F27DE" Ref="#PWR072"  Part="1" 
F 0 "#PWR072" H 6400 3400 50  0001 C CNN
F 1 "GND" H 6405 3477 50  0000 C CNN
F 2 "" H 6400 3650 50  0001 C CNN
F 3 "" H 6400 3650 50  0001 C CNN
	1    6400 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 3350 5750 3450
$Comp
L Device:C_Small C?
U 1 1 5D7F27E6
P 6400 3550
AR Path="/5D7F27E6" Ref="C?"  Part="1" 
AR Path="/5D80E812/5D7F27E6" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F27E6" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F27E6" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F27E6" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F27E6" Ref="C36"  Part="1" 
F 0 "C36" H 6500 3600 50  0000 L CNN
F 1 "10pF" H 6500 3500 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 6400 3550 50  0001 C CNN
F 3 "~" H 6400 3550 50  0001 C CNN
F 4 "" H 6400 3550 50  0001 C CNN "not mounted"
	1    6400 3550
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 5D7F27ED
P 5750 3550
AR Path="/5D7F27ED" Ref="C?"  Part="1" 
AR Path="/5D80E812/5D7F27ED" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F27ED" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F27ED" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F27ED" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F27ED" Ref="C35"  Part="1" 
F 0 "C35" H 5850 3600 50  0000 L CNN
F 1 "10pF" H 5850 3500 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5750 3550 50  0001 C CNN
F 3 "~" H 5750 3550 50  0001 C CNN
F 4 "" H 5750 3550 50  0001 C CNN "not mounted"
	1    5750 3550
	1    0    0    -1  
$EndComp
$Comp
L Device:Crystal_Small Y?
U 1 1 5D7F27F3
P 6100 3350
AR Path="/5D7F27F3" Ref="Y?"  Part="1" 
AR Path="/5D8BFFCE/5D7F27F3" Ref="Y?"  Part="1" 
AR Path="/5D815E09/5D7F27F3" Ref="Y?"  Part="1" 
AR Path="/5D8EDE42/5D7F27F3" Ref="Y?"  Part="1" 
AR Path="/5D7DEA89/5D7F27F3" Ref="Y4"  Part="1" 
F 0 "Y4" H 6150 3200 50  0000 C CNN
F 1 "NX3225GD-8MHZ-STD-CRA-3" H 6450 3500 50  0001 C CNN
F 2 "digikey-footprints:SMD-2_3.2x2.5mm" H 6100 3350 50  0001 C CNN
F 3 "~" H 6100 3350 50  0001 C CNN
	1    6100 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	9800 5550 9650 5550
Wire Wire Line
	9800 5450 9650 5450
Connection ~ 9050 5800
Wire Wire Line
	9150 5800 9050 5800
Wire Wire Line
	9150 5750 9150 5800
Connection ~ 8950 5800
Wire Wire Line
	9050 5800 8950 5800
Wire Wire Line
	9050 5750 9050 5800
Wire Wire Line
	8850 5800 8850 5850
Connection ~ 8850 5800
Wire Wire Line
	8950 5800 8850 5800
Wire Wire Line
	8950 5750 8950 5800
Wire Wire Line
	8850 5750 8850 5800
$Comp
L power:GND #PWR?
U 1 1 5D7F2806
P 8850 5850
AR Path="/5D7F2806" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2806" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F2806" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F2806" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F2806" Ref="#PWR077"  Part="1" 
F 0 "#PWR077" H 8850 5600 50  0001 C CNN
F 1 "GND" H 8855 5677 50  0000 C CNN
F 2 "" H 8850 5850 50  0001 C CNN
F 3 "" H 8850 5850 50  0001 C CNN
	1    8850 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8450 4350 8250 4350
Wire Wire Line
	8450 4450 8250 4450
Text Label 8250 4450 2    50   ~ 0
SYS_JTRST
Text Label 8250 4350 2    50   ~ 0
SYS_JTDO-SWO
Text Label 9800 5550 0    50   ~ 0
SYS_JTDI
Text Label 9800 5450 0    50   ~ 0
SYS_JTCK-SWCLK
Text Label 9800 5350 0    50   ~ 0
SYS_JTMS-SWDIO
Wire Wire Line
	9650 5350 9800 5350
$Comp
L power:GND #PWR?
U 1 1 5D7F2814
P 7500 2850
AR Path="/5D7F2814" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2814" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F2814" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F2814" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F2814" Ref="#PWR074"  Part="1" 
F 0 "#PWR074" H 7500 2600 50  0001 C CNN
F 1 "GND" H 7505 2677 50  0000 C CNN
F 2 "" H 7500 2850 50  0001 C CNN
F 3 "" H 7500 2850 50  0001 C CNN
	1    7500 2850
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 5D7F281B
P 7500 2750
AR Path="/5D7F281B" Ref="C?"  Part="1" 
AR Path="/5D80E812/5D7F281B" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F281B" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F281B" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F281B" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F281B" Ref="C38"  Part="1" 
F 0 "C38" H 7250 2750 50  0000 L CNN
F 1 "100nF" H 7200 2650 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7500 2750 50  0001 C CNN
F 3 "~" H 7500 2750 50  0001 C CNN
F 4 "" H 7500 2750 50  0001 C CNN "not mounted"
	1    7500 2750
	1    0    0    -1  
$EndComp
Connection ~ 7500 1850
Wire Wire Line
	7300 1850 7300 1800
Wire Wire Line
	7500 1850 7300 1850
$Comp
L Device:CP1_Small C?
U 1 1 5D7F2824
P 7500 1950
AR Path="/5D7F2824" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2824" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F2824" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F2824" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F2824" Ref="C37"  Part="1" 
F 0 "C37" H 7591 1996 50  0000 L CNN
F 1 "10uF" H 7591 1905 50  0000 L CNN
F 2 "Capacitor_Tantalum_SMD:CP_EIA-3528-15_AVX-H" H 7500 1950 50  0001 C CNN
F 3 "THJB106K016RJN " H 7500 1950 50  0001 C CNN
	1    7500 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 2050 7500 2150
$Comp
L power:GND #PWR?
U 1 1 5D7F282B
P 7500 2150
AR Path="/5D7F282B" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F282B" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F282B" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F282B" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F282B" Ref="#PWR073"  Part="1" 
F 0 "#PWR073" H 7500 1900 50  0001 C CNN
F 1 "GND" H 7505 1977 50  0000 C CNN
F 2 "" H 7500 2150 50  0001 C CNN
F 3 "" H 7500 2150 50  0001 C CNN
	1    7500 2150
	1    0    0    -1  
$EndComp
Connection ~ 8400 1850
Wire Wire Line
	7500 1850 8400 1850
Connection ~ 9150 2750
Wire Wire Line
	9150 2750 9250 2750
Wire Wire Line
	9050 2750 9050 1850
Connection ~ 9050 2750
Wire Wire Line
	9150 2750 9050 2750
Wire Wire Line
	9150 2850 9150 2750
Wire Wire Line
	8850 2450 8850 2850
Wire Wire Line
	9050 2850 9050 2750
$Comp
L power:GND #PWR?
U 1 1 5D7F283D
P 8400 2150
AR Path="/5D7F283D" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F283D" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F283D" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F283D" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F283D" Ref="#PWR075"  Part="1" 
F 0 "#PWR075" H 8400 1900 50  0001 C CNN
F 1 "GND" H 8405 1977 50  0000 C CNN
F 2 "" H 8400 2150 50  0001 C CNN
F 3 "" H 8400 2150 50  0001 C CNN
	1    8400 2150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5D7F2843
P 8550 2150
AR Path="/5D7F2843" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2843" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F2843" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F2843" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F2843" Ref="#PWR076"  Part="1" 
F 0 "#PWR076" H 8550 1900 50  0001 C CNN
F 1 "GND" H 8555 1977 50  0000 C CNN
F 2 "" H 8550 2150 50  0001 C CNN
F 3 "" H 8550 2150 50  0001 C CNN
	1    8550 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 2100 8400 2150
Wire Wire Line
	8550 2100 8550 2150
Connection ~ 8550 1850
Wire Wire Line
	8550 1850 8550 1900
Wire Wire Line
	8400 1850 8400 1900
Wire Wire Line
	8400 1850 8550 1850
$Comp
L Device:C_Small C?
U 1 1 5D7F2853
P 8550 2000
AR Path="/5D7F2853" Ref="C?"  Part="1" 
AR Path="/5D80E812/5D7F2853" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2853" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F2853" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F2853" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F2853" Ref="C40"  Part="1" 
F 0 "C40" H 8650 2000 50  0000 L CNN
F 1 "100nF" H 8650 1900 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8550 2000 50  0001 C CNN
F 3 "~" H 8550 2000 50  0001 C CNN
F 4 "" H 8550 2000 50  0001 C CNN "not mounted"
	1    8550 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 5D7F285A
P 8400 2000
AR Path="/5D7F285A" Ref="C?"  Part="1" 
AR Path="/5D80E812/5D7F285A" Ref="C?"  Part="1" 
AR Path="/5D8BFFCE/5D7F285A" Ref="C?"  Part="1" 
AR Path="/5D815E09/5D7F285A" Ref="C?"  Part="1" 
AR Path="/5D8EDE42/5D7F285A" Ref="C?"  Part="1" 
AR Path="/5D7DEA89/5D7F285A" Ref="C39"  Part="1" 
F 0 "C39" H 8150 2000 50  0000 L CNN
F 1 "100nF" H 8100 1900 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8400 2000 50  0001 C CNN
F 3 "~" H 8400 2000 50  0001 C CNN
F 4 "" H 8400 2000 50  0001 C CNN "not mounted"
	1    8400 2000
	1    0    0    -1  
$EndComp
$Comp
L MCU_ST_STM32L4:STM32L443CCTx U?
U 1 1 5D7F2860
P 9050 4250
AR Path="/5D7F2860" Ref="U?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2860" Ref="U?"  Part="1" 
AR Path="/5D815E09/5D7F2860" Ref="U?"  Part="1" 
AR Path="/5D8EDE42/5D7F2860" Ref="U?"  Part="1" 
AR Path="/5D7DEA89/5D7F2860" Ref="U11"  Part="1" 
F 0 "U11" H 9400 2750 50  0000 C CNN
F 1 "STM32L443CCTx" H 9500 2650 50  0000 C CNN
F 2 "Package_QFP:LQFP-48_7x7mm_P0.5mm" H 8550 2850 50  0001 R CNN
F 3 "http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00254865.pdf" H 9050 4250 50  0001 C CNN
	1    9050 4250
	1    0    0    -1  
$EndComp
Text Notes 7050 3150 0    50   ~ 0
internal pull-up. LOW = RESET
Wire Wire Line
	7500 2650 7500 2600
Wire Wire Line
	7500 2600 8050 2600
Wire Wire Line
	8050 2600 8050 3050
Wire Wire Line
	8050 3050 8450 3050
Wire Wire Line
	5750 3250 5750 3350
Connection ~ 6400 3350
Wire Wire Line
	6400 3350 6400 3450
NoConn ~ 8450 3750
NoConn ~ 8450 3850
$Sheet
S 1950 3000 800  300 
U 5D7F286F
F0 "sheet5D7F273B" 50
F1 "LDO.sch" 50
F2 "3.3V" O R 2750 3200 50 
F3 "5V" I L 1950 3200 50 
F4 "ENABLE" I L 1950 3100 50 
$EndSheet
$Comp
L power:+5V #PWR?
U 1 1 5D7F2875
P 1150 3050
AR Path="/5D8BFFCE/5D7F2875" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F2875" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F2875" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F2875" Ref="#PWR063"  Part="1" 
F 0 "#PWR063" H 1150 2900 50  0001 C CNN
F 1 "+5V" H 1165 3223 50  0000 C CNN
F 2 "" H 1150 3050 50  0001 C CNN
F 3 "" H 1150 3050 50  0001 C CNN
	1    1150 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	1150 3050 1150 3200
Wire Wire Line
	2750 3200 2900 3200
Text Label 3700 3200 0    50   ~ 0
3V3_ECU
Text Label 7300 1800 0    50   ~ 0
3V3_ECU
Text Label 950  2000 0    50   ~ 0
3V3_ECU
Text Label 8400 2450 2    50   ~ 0
3V3_ECU
Wire Wire Line
	8400 2450 8850 2450
$Comp
L Connector:TestPoint TP?
U 1 1 5D7F2883
P 5750 3250
AR Path="/5D7F2883" Ref="TP?"  Part="1" 
AR Path="/5D80E812/5D7F2883" Ref="TP?"  Part="1" 
AR Path="/5D8BFFCE/5D7F2883" Ref="TP?"  Part="1" 
AR Path="/5D815E09/5D7F2883" Ref="TP?"  Part="1" 
AR Path="/5D8EDE42/5D7F2883" Ref="TP?"  Part="1" 
AR Path="/5D7DEA89/5D7F2883" Ref="TP10"  Part="1" 
F 0 "TP10" H 5808 3368 50  0000 L CNN
F 1 " " H 5808 3277 50  0000 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 5950 3250 50  0001 C CNN
F 3 "~" H 5950 3250 50  0001 C CNN
	1    5750 3250
	1    0    0    -1  
$EndComp
Text HLabel 1850 3100 0    50   Input ~ 0
ECU_ENABLE
NoConn ~ 8450 4550
NoConn ~ 8450 4650
NoConn ~ 8450 4750
Wire Wire Line
	1150 3200 1950 3200
Wire Wire Line
	1850 3100 1950 3100
NoConn ~ 9800 5250
NoConn ~ 9800 5150
$Comp
L Device:R_Small_US R?
U 1 1 5D7F2891
P 1850 2400
AR Path="/5D7F2891" Ref="R?"  Part="1" 
AR Path="/5D80E812/5D7F2891" Ref="R?"  Part="1" 
AR Path="/5D815E09/5D7F2891" Ref="R?"  Part="1" 
AR Path="/5D8EDE42/5D7F2891" Ref="R?"  Part="1" 
AR Path="/5D7DEA89/5D7F2891" Ref="R14"  Part="1" 
F 0 "R14" V 1750 2450 50  0000 C CNN
F 1 "10k" V 1950 2450 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 1850 2400 50  0001 C CNN
F 3 "~" H 1850 2400 50  0001 C CNN
	1    1850 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 2150 1850 2300
Connection ~ 1850 2150
Wire Wire Line
	1850 2150 1750 2150
Wire Wire Line
	1850 2500 1850 2550
$Comp
L power:GND #PWR?
U 1 1 5D7F289B
P 1850 2550
AR Path="/5D7F289B" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D7F289B" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D7F289B" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D7F289B" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D7F289B" Ref="#PWR064"  Part="1" 
F 0 "#PWR064" H 1850 2300 50  0001 C CNN
F 1 "GND" H 1855 2377 50  0000 C CNN
F 2 "" H 1850 2550 50  0001 C CNN
F 3 "" H 1850 2550 50  0001 C CNN
	1    1850 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R?
U 1 1 5D80FD8B
P 3250 3200
AR Path="/5D80FD8B" Ref="R?"  Part="1" 
AR Path="/5D80E812/5D80FD8B" Ref="R?"  Part="1" 
AR Path="/5D8BFFCE/5D7D0A92/5D80FD8B" Ref="R?"  Part="1" 
AR Path="/5D815E09/5D82B40A/5D80FD8B" Ref="R?"  Part="1" 
AR Path="/5D8EDE42/5D82B40A/5D80FD8B" Ref="R?"  Part="1" 
AR Path="/5D7DEA89/5D7F286F/5D80FD8B" Ref="R?"  Part="1" 
AR Path="/5D7DEA89/5D80FD8B" Ref="R16"  Part="1" 
F 0 "R16" V 3150 3200 50  0000 C CNN
F 1 "0R" V 3350 3200 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 3250 3200 50  0001 C CNN
F 3 "~" H 3250 3200 50  0001 C CNN
	1    3250 3200
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint TP?
U 1 1 5D80FD91
P 3000 3050
AR Path="/5D80FD91" Ref="TP?"  Part="1" 
AR Path="/5D80E812/5D80FD91" Ref="TP?"  Part="1" 
AR Path="/5D8BFFCE/5D7D0A92/5D80FD91" Ref="TP?"  Part="1" 
AR Path="/5D815E09/5D82B40A/5D80FD91" Ref="TP?"  Part="1" 
AR Path="/5D8EDE42/5D82B40A/5D80FD91" Ref="TP?"  Part="1" 
AR Path="/5D7DEA89/5D7F286F/5D80FD91" Ref="TP?"  Part="1" 
AR Path="/5D7DEA89/5D80FD91" Ref="TP8"  Part="1" 
F 0 "TP8" H 3058 3168 50  0000 L CNN
F 1 " " H 3058 3077 50  0000 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 3200 3050 50  0001 C CNN
F 3 "~" H 3200 3050 50  0001 C CNN
	1    3000 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 3050 3000 3200
$Comp
L Connector:TestPoint TP?
U 1 1 5D80FD98
P 3400 3050
AR Path="/5D80FD98" Ref="TP?"  Part="1" 
AR Path="/5D80E812/5D80FD98" Ref="TP?"  Part="1" 
AR Path="/5D8BFFCE/5D7D0A92/5D80FD98" Ref="TP?"  Part="1" 
AR Path="/5D815E09/5D82B40A/5D80FD98" Ref="TP?"  Part="1" 
AR Path="/5D8EDE42/5D82B40A/5D80FD98" Ref="TP?"  Part="1" 
AR Path="/5D7DEA89/5D7F286F/5D80FD98" Ref="TP?"  Part="1" 
AR Path="/5D7DEA89/5D80FD98" Ref="TP9"  Part="1" 
F 0 "TP9" H 3458 3168 50  0000 L CNN
F 1 " " H 3458 3077 50  0000 L CNN
F 2 "TestPoint:TestPoint_Loop_D2.54mm_Drill1.5mm_Beaded" H 3600 3050 50  0001 C CNN
F 3 "~" H 3600 3050 50  0001 C CNN
	1    3400 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 3050 3400 3200
Wire Wire Line
	3400 3200 3700 3200
Wire Wire Line
	3350 3200 3400 3200
Connection ~ 3400 3200
Wire Wire Line
	3000 3200 3150 3200
Connection ~ 3000 3200
Text Label 8050 2850 0    50   ~ 0
nRST
Text Label 5500 5850 2    50   ~ 0
SYS_JTDO-SWO
Text Label 6000 5850 0    50   ~ 0
SYS_JTRST
Text Label 5500 5750 2    50   ~ 0
SYS_JTDI
Text Label 6000 5750 0    50   ~ 0
SYS_JTCK-SWCLK
Text Label 6000 5650 0    50   ~ 0
SYS_JTMS-SWDIO
Text Label 5500 5650 2    50   ~ 0
nRST
Wire Wire Line
	3000 1850 3550 1850
Text Label 5500 4650 2    50   ~ 0
3V3_ECU
Text Label 5500 5150 2    50   ~ 0
PB0
Text Label 5500 5250 2    50   ~ 0
PB2
Text Label 5500 5550 2    50   ~ 0
PB15
Text Label 5500 5350 2    50   ~ 0
PB11
Text Label 5500 5450 2    50   ~ 0
PB13
Text Label 5500 4850 2    50   ~ 0
PA2
Text Label 5500 5050 2    50   ~ 0
PA6
Text Label 5500 4950 2    50   ~ 0
PA4
Text Label 5500 4750 2    50   ~ 0
PA1
Text Label 6000 5050 0    50   ~ 0
PA7
Text Label 6000 4950 0    50   ~ 0
PA5
Text Label 6000 4850 0    50   ~ 0
PA3
Text Label 6000 5150 0    50   ~ 0
PB1
Text Label 6000 5250 0    50   ~ 0
PB10
Wire Wire Line
	6000 4750 6350 4750
$Comp
L power:GND #PWR?
U 1 1 5D86C9AF
P 6350 4750
AR Path="/5D86C9AF" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5D86C9AF" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D86C9AF" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D86C9AF" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D86C9AF" Ref="#PWR071"  Part="1" 
F 0 "#PWR071" H 6350 4500 50  0001 C CNN
F 1 "GND" H 6355 4577 50  0000 C CNN
F 2 "" H 6350 4750 50  0001 C CNN
F 3 "" H 6350 4750 50  0001 C CNN
	1    6350 4750
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5D86C9B5
P 6000 4650
AR Path="/5D8BFFCE/5D86C9B5" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D86C9B5" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D86C9B5" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D86C9B5" Ref="#PWR070"  Part="1" 
F 0 "#PWR070" H 6000 4500 50  0001 C CNN
F 1 "+5V" H 6015 4823 50  0000 C CNN
F 2 "" H 6000 4650 50  0001 C CNN
F 3 "" H 6000 4650 50  0001 C CNN
	1    6000 4650
	1    0    0    -1  
$EndComp
Text Label 6000 5550 0    50   ~ 0
PA8
Text Label 6000 5350 0    50   ~ 0
PB12
Text Label 6000 5450 0    50   ~ 0
PB14
NoConn ~ 9650 4050
NoConn ~ 8400 3650
NoConn ~ 9650 5050
Wire Wire Line
	8550 1850 9050 1850
Wire Wire Line
	8950 2850 8950 2750
Wire Wire Line
	8950 2750 9050 2750
$Comp
L Connector:TestPoint TX?
U 1 1 5D9B30EB
P 1400 1750
AR Path="/5D9B30EB" Ref="TX?"  Part="1" 
AR Path="/5D80E812/5D9B30EB" Ref="TX?"  Part="1" 
AR Path="/5D8BFFCE/5D9B30EB" Ref="TX?"  Part="1" 
AR Path="/5D815E09/5D9B30EB" Ref="TX?"  Part="1" 
AR Path="/5D8EDE42/5D9B30EB" Ref="TX?"  Part="1" 
AR Path="/5D7DEA89/5D9B30EB" Ref="TX1"  Part="1" 
F 0 "TX1" V 1400 2000 50  0000 L CNN
F 1 " " H 1458 1777 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 1600 1750 50  0001 C CNN
F 3 "~" H 1600 1750 50  0001 C CNN
	1    1400 1750
	0    -1   -1   0   
$EndComp
$Comp
L Connector:TestPoint RX?
U 1 1 5D9B691A
P 1400 1850
AR Path="/5D9B691A" Ref="RX?"  Part="1" 
AR Path="/5D80E812/5D9B691A" Ref="RX?"  Part="1" 
AR Path="/5D8BFFCE/5D9B691A" Ref="RX?"  Part="1" 
AR Path="/5D815E09/5D9B691A" Ref="RX?"  Part="1" 
AR Path="/5D8EDE42/5D9B691A" Ref="RX?"  Part="1" 
AR Path="/5D7DEA89/5D9B691A" Ref="RX1"  Part="1" 
F 0 "RX1" V 1400 2100 50  0000 L CNN
F 1 " " H 1458 1877 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 1600 1850 50  0001 C CNN
F 3 "~" H 1600 1850 50  0001 C CNN
	1    1400 1850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	1400 1750 2000 1750
Wire Wire Line
	1400 1850 2000 1850
$Comp
L Connector_Generic:Conn_02x13_Odd_Even J?
U 1 1 5DEBB145
P 5700 5250
AR Path="/5D8BFFCE/5DEBB145" Ref="J?"  Part="1" 
AR Path="/5D815E09/5DEBB145" Ref="J?"  Part="1" 
AR Path="/5D8EDE42/5DEBB145" Ref="J?"  Part="1" 
AR Path="/5D7DEA89/5DEBB145" Ref="J6"  Part="1" 
F 0 "J6" H 5750 4550 50  0000 C CNN
F 1 " " H 5750 5976 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x13_P2.54mm_Vertical" H 5700 5250 50  0001 C CNN
F 3 "~" H 5700 5250 50  0001 C CNN
	1    5700 5250
	1    0    0    -1  
$EndComp
Text HLabel 8150 3450 0    50   Input ~ 0
BOOT0
Wire Wire Line
	9800 4950 9650 4950
$Comp
L Device:R_Small_US R?
U 1 1 5DF157FA
P 2900 3400
AR Path="/5DF157FA" Ref="R?"  Part="1" 
AR Path="/5D80E812/5DF157FA" Ref="R?"  Part="1" 
AR Path="/5D815E09/5DF157FA" Ref="R?"  Part="1" 
AR Path="/5D8EDE42/5DF157FA" Ref="R?"  Part="1" 
AR Path="/5D7DEA89/5DF157FA" Ref="R15"  Part="1" 
F 0 "R15" V 2800 3450 50  0000 C CNN
F 1 "10k" V 3000 3450 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 2900 3400 50  0001 C CNN
F 3 "~" H 2900 3400 50  0001 C CNN
	1    2900 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 3500 2900 3600
$Comp
L power:GND #PWR?
U 1 1 5DF15801
P 2900 3600
AR Path="/5DF15801" Ref="#PWR?"  Part="1" 
AR Path="/5D8BFFCE/5DF15801" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF15801" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF15801" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5DF15801" Ref="#PWR068"  Part="1" 
F 0 "#PWR068" H 2900 3350 50  0001 C CNN
F 1 "GND" H 2905 3427 50  0000 C CNN
F 2 "" H 2900 3600 50  0001 C CNN
F 3 "" H 2900 3600 50  0001 C CNN
	1    2900 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 3300 2900 3200
Connection ~ 2900 3200
Wire Wire Line
	2900 3200 3000 3200
Wire Wire Line
	9250 2750 9250 2850
$EndSCHEMATC
