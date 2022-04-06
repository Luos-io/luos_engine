EESchema Schematic File Version 4
LIBS:power_isolator-cache
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
Text Label 3500 2800 0    60   ~ 0
GND
Text Label 3450 3300 0    60   ~ 0
GND
Text Label 3500 3200 0    60   ~ 0
PTP
$Comp
L Common_Lib:DF11-8DP-2DS(24) J1
U 1 1 5BE410E4
P 3000 3050
F 0 "J1" H 3081 3637 60  0000 C CNN
F 1 "DF11-8DP-2DS(24)" H 3081 3531 60  0000 C CNN
F 2 "Common_Footprint:DF11-8DP-2DS(24)" H 3000 3650 60  0001 C CNN
F 3 "https://www.hirose.com/product/en/download_file/key_name/DF11%2D8DP%2D2DS%2824%29/category/Drawing%20(2D)/doc_file_id/39437/?file_category_id=6&item_id=05430535724&is_series=" H 3600 2450 60  0001 C CNN
F 4 "Farnell" H 3050 3600 50  0001 C CNN "Fournisseur"
F 5 "1688360" H 3150 3700 50  0001 C CNN "CodeCommande"
	1    3000 3050
	1    0    0    -1  
$EndComp
$Comp
L Common_Lib:DF11-8DP-2DS(24) J2
U 1 1 5BE41126
P 4200 3050
F 0 "J2" H 4073 3103 60  0000 R CNN
F 1 "DF11-8DP-2DS(24)" H 4073 2997 60  0000 R CNN
F 2 "Common_Footprint:DF11-8DP-2DS(24)" H 4200 3650 60  0001 C CNN
F 3 "https://www.hirose.com/product/en/download_file/key_name/DF11%2D8DP%2D2DS%2824%29/category/Drawing%20(2D)/doc_file_id/39437/?file_category_id=6&item_id=05430535724&is_series=" H 4800 2450 60  0001 C CNN
F 4 "Farnell" H 4250 3600 50  0001 C CNN "Fournisseur"
F 5 "1688360" H 4350 3700 50  0001 C CNN "CodeCommande"
	1    4200 3050
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3250 2800 3950 2800
Wire Wire Line
	3250 2900 3950 2900
Wire Wire Line
	3250 3000 3950 3000
Wire Wire Line
	3250 3100 3950 3100
Wire Wire Line
	3250 3200 3950 3200
Wire Wire Line
	3250 3300 3950 3300
Text Label 3500 2900 0    60   ~ 0
PTP
$EndSCHEMATC
