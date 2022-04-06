EESchema Schematic File Version 4
LIBS:l0_button-cache
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
$Comp
L Switch:SW_Push SW1
U 1 1 5A859ABB
P 5600 4050
F 0 "SW1" H 5600 4335 50  0000 C CNN
F 1 "ESE-20D321" H 5600 4244 50  0000 C CNN
F 2 "l0_Kicad_Footprint:ESE-20D321" H 5600 4250 50  0001 C CNN
F 3 "" H 5600 4250 50  0001 C CNN
F 4 "Farnell" H 5600 4050 50  0001 C CNN "Fournisseur"
F 5 "2079605" H 5600 4050 50  0001 C CNN "CodeCommande"
	1    5600 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 3200 6300 3200
Wire Wire Line
	6300 3200 6300 4050
Wire Wire Line
	6300 4050 5800 4050
Wire Wire Line
	5050 3500 4900 3500
Wire Wire Line
	4900 3500 4900 4050
Wire Wire Line
	4900 4050 5400 4050
$EndSCHEMATC
