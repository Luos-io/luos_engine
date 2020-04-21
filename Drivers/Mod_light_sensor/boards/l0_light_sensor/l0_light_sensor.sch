EESchema Schematic File Version 4
LIBS:l0_light_sensor-cache
EELAYER 26 0
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
L Common_Lib:l0_Shield_Socket J1
U 1 1 5A859A26
P 5500 3350
F 0 "J1" H 5575 3815 50  0000 C CNN
F 1 "l0_Socket" H 5575 3724 50  0000 C CNN
F 2 "Common_Footprint:l0_Shield_Socket" H 5500 2850 50  0001 C CNN
F 3 "" H 5500 3200 50  0001 C CNN
	1    5500 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 3200 6300 3200
Wire Wire Line
	5050 3600 4550 3600
Wire Wire Line
	4550 3600 4550 2750
Wire Wire Line
	5050 3100 4950 3100
$Comp
L Common_Lib:Q_Photo_NPN Q1
U 1 1 5BE998F1
P 5850 2400
F 0 "Q1" V 5535 2400 50  0000 C CNN
F 1 "Q_Photo_NPN" V 5626 2400 50  0000 C CNN
F 2 "Common_Footprint:TEMT6000" H 6050 2500 50  0001 C CNN
F 3 "https://www.vishay.com/docs/81579/temt6000.pdf" H 6050 2450 50  0001 C CNN
F 4 "Farnell" H 6150 2550 50  0001 C CNN "Fournisseur"
F 5 "1497671" H 6250 2650 50  0001 C CNN "CodeCommande"
	1    5850 2400
	0    1    1    0   
$EndComp
$Comp
L Common_Lib:R R1
U 1 1 5BE99AF7
P 5100 2500
F 0 "R1" V 4904 2500 50  0000 C CNN
F 1 "2K" V 4995 2500 50  0000 C CNN
F 2 "Common_Footprint:R_0402_NoSilk" H 5100 2500 50  0001 C CNN
F 3 "http://www.farnell.com/datasheets/2563624.pdf?_ga=2.1522225.1434462718.1543829447-506460182.1534760965" H 5130 2520 50  0001 C CNN
F 4 "Farnell" H 5230 2620 50  0001 C CNN "Fournisseur"
F 5 "2447147" H 5330 2720 50  0001 C CNN "CodeCommande"
	1    5100 2500
	0    1    1    0   
$EndComp
Wire Wire Line
	6300 2500 6050 2500
Wire Wire Line
	6300 2500 6300 3200
Wire Wire Line
	5650 2500 5400 2500
Wire Wire Line
	4950 2500 5000 2500
Wire Wire Line
	4950 2500 4950 3100
Wire Wire Line
	4550 2750 5400 2750
Wire Wire Line
	5400 2750 5400 2500
Connection ~ 5400 2500
Wire Wire Line
	5400 2500 5200 2500
Text Notes 4850 1800 0    50   ~ 0
Lux = (U x 2000000) / R\n\nif R = 10K => Lux = U x 200 Max Lux = 660\nif R = 2K => Lux = U x 1000 Max Lux = 3300\nif R = 1K => Lux = U x 2000 Max Lux = 6600\n\nhttps://www.vishay.com/docs/84154/appnotesensors.pdf
$Comp
L power:GND #PWR?
U 1 1 5BE9A0F2
P 4800 3100
F 0 "#PWR?" H 4800 2850 50  0001 C CNN
F 1 "GND" H 4805 2927 50  0000 C CNN
F 2 "" H 4800 3100 50  0001 C CNN
F 3 "" H 4800 3100 50  0001 C CNN
	1    4800 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 3100 4800 3100
Connection ~ 4950 3100
$EndSCHEMATC
