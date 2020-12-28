EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Screens - RAMN: Resistant Automotive Miniature Network V1"
Date "2020-12-02"
Rev "A"
Comp "Copyright (c) 2020 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED."
Comment1 ""
Comment2 ""
Comment3 "License: CC BY-SA 4.0"
Comment4 "https://github.com/toyotainfotech/ramn"
$EndDescr
$Comp
L Connector_Generic:Conn_02x13_Odd_Even J1
U 1 1 5D87B279
P 1900 1450
F 0 "J1" H 1950 2067 50  0000 C CNN
F 1 " " H 1950 1976 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x13_P2.54mm_Horizontal" H 1900 1450 50  0001 C CNN
F 3 "~" H 1900 1450 50  0001 C CNN
	1    1900 1450
	1    0    0    -1  
$EndComp
Text Label 2200 1450 0    50   ~ 0
PB10
Text Label 1700 850  2    50   ~ 0
3V3_ECU
Wire Wire Line
	2200 950  2550 950 
$Comp
L power:GND #PWR02
U 1 1 5D9A7138
P 2550 950
AR Path="/5D9A7138" Ref="#PWR02"  Part="1" 
AR Path="/5D8BFFCE/5D9A7138" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D9A7138" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D9A7138" Ref="#PWR?"  Part="1" 
F 0 "#PWR02" H 2550 700 50  0001 C CNN
F 1 "GND" H 2650 950 50  0000 C CNN
F 2 "" H 2550 950 50  0001 C CNN
F 3 "" H 2550 950 50  0001 C CNN
	1    2550 950 
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5D9A713E
P 2200 850
AR Path="/5D8BFFCE/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5D9A713E" Ref="#PWR?"  Part="1" 
AR Path="/5D9A713E" Ref="#PWR01"  Part="1" 
F 0 "#PWR01" H 2200 700 50  0001 C CNN
F 1 "+5V" H 2215 1023 50  0000 C CNN
F 2 "" H 2200 850 50  0001 C CNN
F 3 "" H 2200 850 50  0001 C CNN
	1    2200 850 
	1    0    0    -1  
$EndComp
Text Label 1700 1450 2    50   ~ 0
PB2
Text Label 1700 1750 2    50   ~ 0
PB15
Text Label 1700 1550 2    50   ~ 0
PB11
Text Label 1700 1650 2    50   ~ 0
PB13
Text Label 2200 1550 0    50   ~ 0
PB12
Text Label 2200 1650 0    50   ~ 0
PB14
$Comp
L Connector_Generic:Conn_01x08 J4
U 1 1 5D87ACF7
P 2450 3400
F 0 "J4" H 2530 3392 50  0000 L CNN
F 1 "326" H 2530 3301 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical" H 2450 3400 50  0001 C CNN
F 3 "~" H 2450 3400 50  0001 C CNN
	1    2450 3400
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x13 J3
U 1 1 5DEE213B
P 8100 1600
F 0 "J3" H 8180 1592 50  0000 L CNN
F 1 "4086" H 8180 1501 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x13_P2.54mm_Vertical" H 8100 1600 50  0001 C CNN
F 3 "~" H 8100 1600 50  0001 C CNN
	1    8100 1600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x11 J5
U 1 1 5DEE979F
P 7150 3700
F 0 "J5" H 7230 3692 50  0000 L CNN
F 1 "4383" H 7230 3601 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x11_P2.54mm_Vertical" H 7150 3700 50  0001 C CNN
F 3 "~" H 7150 3700 50  0001 C CNN
	1    7150 3700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x11 J2
U 1 1 5DEEAF27
P 5400 1450
F 0 "J2" H 5480 1442 50  0000 L CNN
F 1 "1431" H 5480 1351 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x11_P2.54mm_Vertical" H 5400 1450 50  0001 C CNN
F 3 "~" H 5400 1450 50  0001 C CNN
	1    5400 1450
	1    0    0    -1  
$EndComp
Text GLabel 5050 1150 0    50   Input ~ 0
DC
Text GLabel 5050 1450 0    50   Input ~ 0
SD_CS
Text GLabel 5050 1350 0    50   Input ~ 0
LCD_CS
Text GLabel 5050 1050 0    50   Input ~ 0
SCK
Text GLabel 5050 1550 0    50   Input ~ 0
MISO
Text GLabel 5050 950  0    50   Input ~ 0
MOSI
Text GLabel 1400 1750 0    50   Input ~ 0
MOSI
Wire Wire Line
	1700 1750 1400 1750
Wire Wire Line
	2200 1650 2500 1650
Text GLabel 2500 1650 2    50   Input ~ 0
MISO
Text GLabel 1400 1650 0    50   Input ~ 0
SCK
Wire Wire Line
	1400 1650 1700 1650
Text GLabel 2500 1550 2    50   Input ~ 0
LCD_CS
Wire Wire Line
	2200 1550 2500 1550
Text GLabel 1400 1450 0    50   Input ~ 0
SRAM_CS
Wire Wire Line
	1400 1550 1700 1550
Text GLabel 2500 1450 2    50   Input ~ 0
SD_CS
Wire Wire Line
	2500 1450 2200 1450
Text GLabel 1400 1550 0    50   Input ~ 0
DC
Wire Wire Line
	1400 1450 1700 1450
NoConn ~ 2200 1850
NoConn ~ 2200 1950
NoConn ~ 2200 2050
NoConn ~ 1700 2050
NoConn ~ 1700 1950
NoConn ~ 1700 1850
NoConn ~ 2200 1350
NoConn ~ 2200 1250
NoConn ~ 2200 1150
NoConn ~ 2200 1050
NoConn ~ 1700 1350
NoConn ~ 1700 1250
NoConn ~ 1700 1150
NoConn ~ 1700 1050
NoConn ~ 1700 950 
Text Notes 8400 1400 0    50   ~ 0
VIN|3V3|GND|SCK|MISO|MOSI|ECS|DC|SRCS|SDCS|RST|BUSY|ENA
Text Notes 7500 3700 0    50   ~ 0
LIT|SDCS|DC|RST|TFTCS|MOSI|MISO|SCK|GND|3V|VIN
Text Notes 7550 3800 0    50   ~ 0
reversed
Text Notes 2700 3300 0    50   ~ 0
Data|Clk|DC|Rst|CS|3v3|VIN|GND
Text Notes 4750 800  0    50   ~ 0
SI|CL|DC|R|OC|SC|SO|CO|3v|+|G
Text Label 7750 1000 2    50   ~ 0
3V3_ECU
Wire Wire Line
	7900 1000 7750 1000
NoConn ~ 7900 1100
Text GLabel 7800 1700 0    50   Input ~ 0
DC
Text GLabel 7800 1900 0    50   Input ~ 0
SD_CS
Text GLabel 7800 1800 0    50   Input ~ 0
SRAM_CS
Text GLabel 7800 1600 0    50   Input ~ 0
LCD_CS
Text GLabel 7800 1300 0    50   Input ~ 0
SCK
Text GLabel 7800 1400 0    50   Input ~ 0
MISO
Text GLabel 7800 1500 0    50   Input ~ 0
MOSI
$Comp
L power:GND #PWR03
U 1 1 5DF1AFA5
P 7450 1200
AR Path="/5DF1AFA5" Ref="#PWR03"  Part="1" 
AR Path="/5D8BFFCE/5DF1AFA5" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF1AFA5" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF1AFA5" Ref="#PWR?"  Part="1" 
F 0 "#PWR03" H 7450 950 50  0001 C CNN
F 1 "GND" H 7550 1200 50  0000 C CNN
F 2 "" H 7450 1200 50  0001 C CNN
F 3 "" H 7450 1200 50  0001 C CNN
	1    7450 1200
	0    1    1    0   
$EndComp
Wire Wire Line
	7900 1200 7450 1200
NoConn ~ 7900 2000
NoConn ~ 7900 2100
NoConn ~ 7900 2200
Wire Wire Line
	7900 1900 7800 1900
Wire Wire Line
	7800 1800 7900 1800
Wire Wire Line
	7900 1700 7800 1700
Wire Wire Line
	7800 1600 7900 1600
Wire Wire Line
	7900 1500 7800 1500
Wire Wire Line
	7800 1400 7900 1400
Wire Wire Line
	7900 1300 7800 1300
Text GLabel 6800 3400 0    50   Input ~ 0
DC
Text GLabel 6800 3300 0    50   Input ~ 0
SD_CS
Text GLabel 6800 3600 0    50   Input ~ 0
LCD_CS
Text GLabel 6800 3900 0    50   Input ~ 0
SCK
Text GLabel 6800 3800 0    50   Input ~ 0
MISO
Text GLabel 6800 3700 0    50   Input ~ 0
MOSI
NoConn ~ 6950 3200
NoConn ~ 6950 3500
$Comp
L power:GND #PWR08
U 1 1 5DF2171D
P 6500 4000
AR Path="/5DF2171D" Ref="#PWR08"  Part="1" 
AR Path="/5D8BFFCE/5DF2171D" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF2171D" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF2171D" Ref="#PWR?"  Part="1" 
F 0 "#PWR08" H 6500 3750 50  0001 C CNN
F 1 "GND" H 6600 4000 50  0000 C CNN
F 2 "" H 6500 4000 50  0001 C CNN
F 3 "" H 6500 4000 50  0001 C CNN
	1    6500 4000
	0    1    1    0   
$EndComp
NoConn ~ 6950 4100
$Comp
L power:+5V #PWR?
U 1 1 5DF221FC
P 6100 4150
AR Path="/5D8BFFCE/5DF221FC" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF221FC" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF221FC" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5DF221FC" Ref="#PWR?"  Part="1" 
AR Path="/5DF221FC" Ref="#PWR09"  Part="1" 
F 0 "#PWR09" H 6100 4000 50  0001 C CNN
F 1 "+5V" H 6115 4323 50  0000 C CNN
F 2 "" H 6100 4150 50  0001 C CNN
F 3 "" H 6100 4150 50  0001 C CNN
	1    6100 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 4200 6100 4200
Wire Wire Line
	6100 4200 6100 4150
Wire Wire Line
	6950 4000 6500 4000
Wire Wire Line
	6800 3900 6950 3900
Wire Wire Line
	6800 3800 6950 3800
Wire Wire Line
	6800 3700 6950 3700
Wire Wire Line
	6950 3600 6800 3600
Wire Wire Line
	6800 3400 6950 3400
Wire Wire Line
	6950 3300 6800 3300
Text GLabel 1900 3300 0    50   Input ~ 0
DC
Text GLabel 1950 3500 0    50   Input ~ 0
SD_CS
Text GLabel 1900 3200 0    50   Input ~ 0
SCK
Text GLabel 1900 3100 0    50   Input ~ 0
MOSI
$Comp
L power:+5V #PWR?
U 1 1 5DF2712F
P 1400 3150
AR Path="/5D8BFFCE/5DF2712F" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF2712F" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF2712F" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5DF2712F" Ref="#PWR?"  Part="1" 
AR Path="/5DF2712F" Ref="#PWR06"  Part="1" 
F 0 "#PWR06" H 1400 3000 50  0001 C CNN
F 1 "+5V" H 1415 3323 50  0000 C CNN
F 2 "" H 1400 3150 50  0001 C CNN
F 3 "" H 1400 3150 50  0001 C CNN
	1    1400 3150
	1    0    0    -1  
$EndComp
NoConn ~ 2250 3400
NoConn ~ 2250 3600
Wire Wire Line
	2250 3700 1400 3700
Wire Wire Line
	1400 3700 1400 3150
$Comp
L power:GND #PWR07
U 1 1 5DF2A69C
P 2150 4000
AR Path="/5DF2A69C" Ref="#PWR07"  Part="1" 
AR Path="/5D8BFFCE/5DF2A69C" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF2A69C" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF2A69C" Ref="#PWR?"  Part="1" 
F 0 "#PWR07" H 2150 3750 50  0001 C CNN
F 1 "GND" H 2250 4000 50  0000 C CNN
F 2 "" H 2150 4000 50  0001 C CNN
F 3 "" H 2150 4000 50  0001 C CNN
	1    2150 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 3800 2150 3800
Wire Wire Line
	2150 3800 2150 4000
Wire Wire Line
	1900 3100 2250 3100
Wire Wire Line
	2250 3200 1900 3200
Wire Wire Line
	1900 3300 2250 3300
Wire Wire Line
	2250 3500 1950 3500
NoConn ~ 5200 1250
NoConn ~ 5200 1650
NoConn ~ 5200 1750
$Comp
L power:+5V #PWR?
U 1 1 5DF346FF
P 4550 1750
AR Path="/5D8BFFCE/5DF346FF" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF346FF" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF346FF" Ref="#PWR?"  Part="1" 
AR Path="/5D7DEA89/5DF346FF" Ref="#PWR?"  Part="1" 
AR Path="/5DF346FF" Ref="#PWR04"  Part="1" 
F 0 "#PWR04" H 4550 1600 50  0001 C CNN
F 1 "+5V" H 4565 1923 50  0000 C CNN
F 2 "" H 4550 1750 50  0001 C CNN
F 3 "" H 4550 1750 50  0001 C CNN
	1    4550 1750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 5DF35B96
P 4800 2000
AR Path="/5DF35B96" Ref="#PWR05"  Part="1" 
AR Path="/5D8BFFCE/5DF35B96" Ref="#PWR?"  Part="1" 
AR Path="/5D815E09/5DF35B96" Ref="#PWR?"  Part="1" 
AR Path="/5D8EDE42/5DF35B96" Ref="#PWR?"  Part="1" 
F 0 "#PWR05" H 4800 1750 50  0001 C CNN
F 1 "GND" H 4900 2000 50  0000 C CNN
F 2 "" H 4800 2000 50  0001 C CNN
F 3 "" H 4800 2000 50  0001 C CNN
	1    4800 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 1950 4800 1950
Wire Wire Line
	4800 1950 4800 2000
Wire Wire Line
	5200 1850 4550 1850
Wire Wire Line
	4550 1850 4550 1750
Wire Wire Line
	5200 1550 5050 1550
Wire Wire Line
	5050 1450 5200 1450
Wire Wire Line
	5200 1350 5050 1350
Wire Wire Line
	5050 1150 5200 1150
Wire Wire Line
	5200 1050 5050 1050
Wire Wire Line
	5050 950  5200 950 
NoConn ~ 2200 1750
$EndSCHEMATC
