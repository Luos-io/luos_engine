EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Common_Lib:DRV8833 U1
U 1 1 5A9D7EF9
P 6350 1750
F 0 "U1" H 6050 2300 60  0000 C CNN
F 1 "DRV8833" H 6600 1150 60  0000 C CNN
F 2 "Common_Footprint:HTSSOP-16" H 6250 2800 60  0001 C CNN
F 3 "www.ti.com/lit/ds/symlink/drv8833.pdf" H 6300 2700 60  0001 C CNN
F 4 "Farnell" H 6325 2436 50  0001 C CNN "Fournisseur"
F 5 "2057085" H 6150 2400 60  0001 C CNN "CodeCommande"
	1    6350 1750
	1    0    0    -1  
$EndComp
$Comp
L Common_Lib:l0_Shield_Socket J1
U 1 1 5A9D7FAF
P 2700 1700
F 0 "J1" H 2725 2165 50  0000 C CNN
F 1 "l0_Shield_Socket" H 2725 2074 50  0000 C CNN
F 2 "Common_Footprint:l0_Shield_Socket" H 2800 1275 50  0001 C CNN
F 3 "" H 2700 1550 50  0001 C CNN
	1    2700 1700
	1    0    0    -1  
$EndComp
Text Label 3300 1450 0    50   ~ 0
Robus_PS
$Comp
L Device:C_Small C2
U 1 1 5A9D80E5
P 6900 900
F 0 "C2" H 6992 946 50  0000 L CNN
F 1 "10nF" H 6992 855 50  0000 L CNN
F 2 "Common_Footprint:C_0402_NoSilk" H 6900 900 50  0001 C CNN
F 3 "~" H 6900 900 50  0001 C CNN
F 4 "Farnell" H 6900 900 50  0001 C CNN "Fournisseur"
F 5 "1758924" H 6900 900 50  0001 C CNN "CodeCommande"
	1    6900 900 
	1    0    0    -1  
$EndComp
$Comp
L Device:CP_Small C1
U 1 1 5A9DDF73
P 6550 900
F 0 "C1" H 6638 946 50  0000 L CNN
F 1 "10µF" H 6638 855 50  0000 L CNN
F 2 "Common_Footprint:CP_Tantalum_Case-S_EIA-3216-12_Reflow" H 6550 900 50  0001 C CNN
F 3 "~" H 6550 900 50  0001 C CNN
F 4 "Farnell" H 6550 900 50  0001 C CNN "Fournisseur"
F 5 "1457413" H 6550 900 50  0001 C CNN "CodeCommande"
	1    6550 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 1150 6350 800 
Wire Wire Line
	6350 800  6550 800 
Wire Wire Line
	6550 800  6900 800 
Connection ~ 6550 800 
Wire Wire Line
	6900 1000 6900 1300
Wire Wire Line
	6900 1300 6750 1300
$Comp
L Device:R_Small R2
U 1 1 5A9DE2E6
P 6900 2350
F 0 "R2" H 6959 2396 50  0000 L CNN
F 1 "300m" H 6959 2305 50  0000 L CNN
F 2 "Common_Footprint:R_0603" H 6900 2350 50  0001 C CNN
F 3 "~" H 6900 2350 50  0001 C CNN
F 4 "Farnell" H 6900 2350 50  0001 C CNN "Fournisseur"
F 5 "2328095" H 6900 2350 50  0001 C CNN "CodeCommande"
	1    6900 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6750 2250 6900 2250
Wire Wire Line
	6750 2150 7200 2150
Wire Wire Line
	7200 2150 7200 2250
$Comp
L Device:C_Small C3
U 1 1 5A9DE5DC
P 7700 2350
F 0 "C3" H 7792 2396 50  0000 L CNN
F 1 "2,2µF" H 7792 2305 50  0000 L CNN
F 2 "Common_Footprint:C_0402_NoSilk" H 7700 2350 50  0001 C CNN
F 3 "~" H 7700 2350 50  0001 C CNN
F 4 "Farnell" H 7700 2350 50  0001 C CNN "Fournisseur"
F 5 "2362088" H 7700 2350 50  0001 C CNN "CodeCommande"
	1    7700 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	7700 2250 7700 2000
Wire Wire Line
	7700 2000 6750 2000
Wire Notes Line
	6800 2200 6800 2450
Wire Notes Line
	6800 2450 7500 2450
Wire Notes Line
	7500 2450 7500 2200
Wire Notes Line
	7500 2200 6800 2200
Text Notes 7450 2800 0    50   ~ 0
Current Limit : 750mA
Wire Notes Line
	7450 2450 7450 2750
$Comp
L Device:R_Small R1
U 1 1 5A9DF2DE
P 5800 2200
F 0 "R1" V 5700 2150 50  0000 L CNN
F 1 "20k" V 5900 2150 50  0000 L CNN
F 2 "Common_Footprint:R_0402_NoSilk" H 5800 2200 50  0001 C CNN
F 3 "~" H 5800 2200 50  0001 C CNN
F 4 "Farnell" H 0   0   50  0001 C CNN "Fournisseur"
F 5 "2447133" H 0   0   50  0001 C CNN "CodeCommande"
	1    5800 2200
	0    1    1    0   
$EndComp
Text Label 6350 800  2    50   ~ 0
Robus_PS
$Comp
L power:+3.3V #PWR0106
U 1 1 5A9DF5A7
P 3300 1550
F 0 "#PWR0106" H 3300 1400 50  0001 C CNN
F 1 "+3.3V" V 3300 1650 50  0000 L CNN
F 2 "" H 3300 1550 50  0001 C CNN
F 3 "" H 3300 1550 50  0001 C CNN
	1    3300 1550
	0    1    1    0   
$EndComp
Text Label 3300 1950 0    50   ~ 0
NC_PWM1
Text Label 3300 1650 0    50   ~ 0
NO_PWM1
Text Label 3300 1750 0    50   ~ 0
NC_PWM2
Text Label 3300 1850 0    50   ~ 0
NO_PWM2
Text Label 5900 1450 2    50   ~ 0
NC_PWM1
Text Label 5900 1550 2    50   ~ 0
NO_PWM1
Text Label 5900 1700 2    50   ~ 0
NC_PWM2
Text Label 5900 1800 2    50   ~ 0
NO_PWM2
Text Label 5900 2000 2    50   ~ 0
~nFAULT
Text Label 2250 1950 2    50   ~ 0
~nFAULT
Text Label 5700 2200 2    50   ~ 0
~nSLEEP
Text Label 2250 1850 2    50   ~ 0
~nSLEEP
$Comp
L Common_Lib:Screw_Terminal_01x02-Connector_Specialized J2
U 1 1 5A9E07C2
P 6950 1450
F 0 "J2" H 7030 1442 50  0000 L CNN
F 1 "1776275-2" H 7030 1351 50  0000 L CNN
F 2 "Common_Footprint:1776275-2" H 6950 1450 50  0001 C CNN
F 3 "~" H 6950 1450 50  0001 C CNN
F 4 "Farnell" H 6950 1450 50  0001 C CNN "Fournisseur"
F 5 "1098611" H 6950 1450 50  0001 C CNN "CodeCommande"
	1    6950 1450
	1    0    0    -1  
$EndComp
$Comp
L Common_Lib:Screw_Terminal_01x02-Connector_Specialized J3
U 1 1 5A9E0ACD
P 6950 1700
F 0 "J3" H 7030 1692 50  0000 L CNN
F 1 "1776275-2" H 7030 1601 50  0000 L CNN
F 2 "Common_Footprint:1776275-2" H 6950 1700 50  0001 C CNN
F 3 "~" H 6950 1700 50  0001 C CNN
F 4 "Farnell" H 6950 1700 50  0001 C CNN "Fournisseur"
F 5 "1098611" H 6950 1700 50  0001 C CNN "CodeCommande"
	1    6950 1700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5A9E136C
P 2250 1450
F 0 "#PWR0107" H 2250 1200 50  0001 C CNN
F 1 "GND" V 2250 1250 50  0000 C CNN
F 2 "" H 2250 1450 50  0001 C CNN
F 3 "" H 2250 1450 50  0001 C CNN
	1    2250 1450
	0    1    1    0   
$EndComp
Wire Wire Line
	6900 2600 6900 2450
Wire Wire Line
	6350 2400 6350 2600
Wire Wire Line
	6900 2600 7200 2600
Wire Wire Line
	7200 2600 7200 2450
Connection ~ 6900 2600
Wire Wire Line
	7200 2600 7700 2600
Wire Wire Line
	7700 2600 7700 2450
Connection ~ 7200 2600
$Comp
L Device:R_Small R3
U 1 1 5A9FF4C1
P 7200 2350
F 0 "R3" H 7259 2396 50  0000 L CNN
F 1 "300m" H 7259 2305 50  0000 L CNN
F 2 "Common_Footprint:R_0603" H 7200 2350 50  0001 C CNN
F 3 "~" H 7200 2350 50  0001 C CNN
F 4 "Farnell" H 7200 2350 50  0001 C CNN "Fournisseur"
F 5 "2328095" H 7200 2350 50  0001 C CNN "CodeCommande"
	1    7200 2350
	1    0    0    -1  
$EndComp
Text Notes 10550 7650 0    50   ~ 0
0.0.1
Wire Wire Line
	6350 2600 6900 2600
$Comp
L power:GND #PWR0101
U 1 1 5F98D186
P 6900 2600
F 0 "#PWR0101" H 6900 2350 50  0001 C CNN
F 1 "GND" V 6900 2400 50  0000 C CNN
F 2 "" H 6900 2600 50  0001 C CNN
F 3 "" H 6900 2600 50  0001 C CNN
	1    6900 2600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5F98E3D8
P 6550 1000
F 0 "#PWR0102" H 6550 750 50  0001 C CNN
F 1 "GND" V 6550 800 50  0000 C CNN
F 2 "" H 6550 1000 50  0001 C CNN
F 3 "" H 6550 1000 50  0001 C CNN
	1    6550 1000
	1    0    0    -1  
$EndComp
$Comp
L Common_Lib:D_Zener D1
U 1 1 5F9922A6
P 1200 1500
F 0 "D1" V 1154 1579 50  0000 L CNN
F 1 "D_Zener" V 1245 1579 50  0000 L CNN
F 2 "Common_Footprint:D_SOD-323F" H 1100 1500 50  0001 C CNN
F 3 "~" H 1200 1600 50  0001 C CNN
F 4 "Farnell" H 1300 1700 50  0001 C CNN "Fournisseur"
F 5 "2575197" H 1400 1800 50  0001 C CNN "CodeCommande"
	1    1200 1500
	0    1    1    0   
$EndComp
Text Label 1200 1350 0    50   ~ 0
Robus_PS
$Comp
L power:GND #PWR0103
U 1 1 5F9942C9
P 1200 1650
F 0 "#PWR0103" H 1200 1400 50  0001 C CNN
F 1 "GND" V 1200 1450 50  0000 C CNN
F 2 "" H 1200 1650 50  0001 C CNN
F 3 "" H 1200 1650 50  0001 C CNN
	1    1200 1650
	1    0    0    -1  
$EndComp
$EndSCHEMATC
