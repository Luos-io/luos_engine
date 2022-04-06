EESchema Schematic File Version 4
LIBS:l0_potentiometer-cache
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
U 1 1 5A857108
P 5050 3100
F 0 "J1" H 5125 3565 50  0000 C CNN
F 1 "l0_Socket" H 5125 3474 50  0000 C CNN
F 2 "Common_Footprint:l0_Shield_Socket" H 5050 2600 50  0001 C CNN
F 3 "" H 5050 2950 50  0001 C CNN
	1    5050 3100
	1    0    0    -1  
$EndComp
$Comp
L l0_potentiometer-rescue:POT-Device RV1
U 1 1 5A8571BF
P 5150 3750
F 0 "RV1" V 4943 3750 50  0000 C CNN
F 1 "POT" V 5034 3750 50  0000 C CNN
F 2 "Potentiometers:Potentiometer_Alps_RK09K_Horizontal" H 5150 3750 50  0001 C CNN
F 3 "" H 5150 3750 50  0001 C CNN
F 4 "Farnell" V 5150 3750 50  0001 C CNN "Fournisseur"
F 5 "1191725" V 5150 3750 50  0001 C CNN "CodeCommande"
	1    5150 3750
	0    1    1    0   
$EndComp
Wire Wire Line
	5000 3750 4250 3750
Wire Wire Line
	4250 3750 4250 2850
Wire Wire Line
	4250 2850 4600 2850
Wire Wire Line
	5300 3750 5850 3750
Wire Wire Line
	5850 3750 5850 2950
Wire Wire Line
	5850 2950 5650 2950
Wire Wire Line
	5150 3900 5750 3900
Wire Wire Line
	5750 3900 5750 3350
Wire Wire Line
	5750 3350 5650 3350
$EndSCHEMATC
