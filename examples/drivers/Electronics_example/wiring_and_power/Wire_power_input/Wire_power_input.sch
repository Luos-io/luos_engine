EESchema Schematic File Version 4
LIBS:Wire_power_input-cache
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
L Common_Lib:DF11-8DP-2DS(24) J1
U 1 1 5AB292F2
P 2750 1250
F 0 "J1" H 2831 1837 60  0000 C CNN
F 1 "DF11-8DP-2DS(24)" H 2831 1731 60  0000 C CNN
F 2 "Common_Footprint:DF11-8DP-2DS(24)" H 2750 1850 60  0001 C CNN
F 3 "https://www.hirose.com/product/en/download_file/key_name/DF11%2D8DP%2D2DS%2824%29/category/Drawing%20(2D)/doc_file_id/39437/?file_category_id=6&item_id=05430535724&is_series=" H 3350 650 60  0001 C CNN
F 4 "Farnell" H 2800 1800 50  0001 C CNN "Fournisseur"
F 5 "1688360" H 2900 1900 50  0001 C CNN "CodeCommande"
	1    2750 1250
	1    0    0    -1  
$EndComp
$Comp
L Common_Lib:DF11-8DP-2DS(24) J2
U 1 1 5AB29375
P 4350 1250
F 0 "J2" H 4431 1837 60  0000 C CNN
F 1 "DF11-8DP-2DS(24)" H 4431 1731 60  0000 C CNN
F 2 "Common_Footprint:DF11-8DP-2DS(24)" H 4350 1850 60  0001 C CNN
F 3 "https://www.hirose.com/product/en/download_file/key_name/DF11%2D8DP%2D2DS%2824%29/category/Drawing%20(2D)/doc_file_id/39437/?file_category_id=6&item_id=05430535724&is_series=" H 4950 650 60  0001 C CNN
F 4 "Farnell" H 4400 1800 50  0001 C CNN "Fournisseur"
F 5 "1688360" H 4500 1900 50  0001 C CNN "CodeCommande"
	1    4350 1250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5AB29433
P 4600 1500
F 0 "#PWR0103" H 4600 1250 50  0001 C CNN
F 1 "GND" V 4605 1372 50  0000 R CNN
F 2 "" H 4600 1500 50  0001 C CNN
F 3 "" H 4600 1500 50  0001 C CNN
	1    4600 1500
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5AB29452
P 4600 1000
F 0 "#PWR0104" H 4600 750 50  0001 C CNN
F 1 "GND" V 4605 872 50  0000 R CNN
F 2 "" H 4600 1000 50  0001 C CNN
F 3 "" H 4600 1000 50  0001 C CNN
	1    4600 1000
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 5AB29465
P 3000 1500
F 0 "#PWR0105" H 3000 1250 50  0001 C CNN
F 1 "GND" V 3005 1372 50  0000 R CNN
F 2 "" H 3000 1500 50  0001 C CNN
F 3 "" H 3000 1500 50  0001 C CNN
	1    3000 1500
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5AB29478
P 3000 1000
F 0 "#PWR0106" H 3000 750 50  0001 C CNN
F 1 "GND" V 3005 872 50  0000 R CNN
F 2 "" H 3000 1000 50  0001 C CNN
F 3 "" H 3000 1000 50  0001 C CNN
	1    3000 1000
	0    -1   -1   0   
$EndComp
Text Label 3000 900  0    50   ~ 0
Robus_PW
Text Label 4600 900  0    50   ~ 0
Robus_PW
Text Label 3000 1600 0    50   ~ 0
Robus_PW
Text Label 4600 1600 0    50   ~ 0
Robus_PW
Text Label 5750 1050 0    50   ~ 0
DC_Power
Text Label 5450 1050 2    50   ~ 0
Robus_PW
$Comp
L Common_Lib:D_Schottky D1
U 1 1 5AB3713C
P 5600 1050
F 0 "D1" H 5600 1266 50  0000 C CNN
F 1 "HTA5L45" H 5600 1175 50  0000 C CNN
F 2 "Common_Footprint:D_SMA" H 5500 1050 50  0001 C CNN
F 3 "~" H 5600 1050 50  0001 C CNN
F 4 "Farnell" H 5700 1250 50  0001 C CNN "Fournisseur"
F 5 "2750940" H 5800 1350 50  0001 C CNN "CodeCommande"
	1    5600 1050
	1    0    0    -1  
$EndComp
Text Label 5550 1450 2    50   ~ 0
DC_Power
$Comp
L power:GND #PWR0101
U 1 1 5AB369A1
P 5500 1650
F 0 "#PWR0101" H 5500 1400 50  0001 C CNN
F 1 "GND" H 5505 1477 50  0000 C CNN
F 2 "" H 5500 1650 50  0001 C CNN
F 3 "" H 5500 1650 50  0001 C CNN
	1    5500 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 1550 5500 1550
Wire Wire Line
	5500 1550 5500 1650
Text Label 3000 1100 0    50   ~ 0
PTP
Text Label 3000 1400 0    50   ~ 0
PTP
Text Label 4600 1400 0    50   ~ 0
PTP
Text Label 4600 1100 0    50   ~ 0
PTP
Text Label 3000 1200 0    50   ~ 0
B_RS_485_P
Text Label 4600 1200 0    50   ~ 0
B_RS_485_P
Text Label 3000 1300 0    50   ~ 0
A_RS_485_N
Text Label 4600 1300 0    50   ~ 0
A_RS_485_N
$Comp
L Common_Lib:Screw_Terminal_01x02-Connector_Specialized J3
U 1 1 5BE6D8BA
P 5750 1450
F 0 "J3" H 5830 1442 50  0000 L CNN
F 1 "Screw_Terminal_01x02-Connector_Specialized" H 5830 1351 50  0000 L CNN
F 2 "Common_Footprint:1776275-2" H 5750 1450 50  0001 C CNN
F 3 "" H 5750 1450 50  0001 C CNN
	1    5750 1450
	1    0    0    -1  
$EndComp
$EndSCHEMATC
