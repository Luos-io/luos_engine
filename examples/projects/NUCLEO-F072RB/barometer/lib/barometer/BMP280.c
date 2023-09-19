#include "BMP280.h"

/* Registers */
uint8_t Sub1_Temperature1_Reg             = 0x88;
uint8_t Sub2_Temperature1_Reg             = 0x89;
uint8_t Sub1_Temperature2_Reg             = 0x8A;
uint8_t Sub2_Temperature2_Reg             = 0x8B;
uint8_t Sub1_Temperature3_Reg             = 0x8C;
uint8_t Sub2_Temperature3_Reg             = 0x8D;
uint8_t Sub1_Compensation_Temperature_Reg = 0xFC;
uint8_t Sub2_Compensation_Temperature_Reg = 0xFB;
uint8_t Sub3_Compensation_Temperature_Reg = 0xFA;

uint8_t Sub1_Pressure1_Reg             = 0x8E;
uint8_t Sub2_Pressure1_Reg             = 0x8F;
uint8_t Sub1_Pressure2_Reg             = 0x90;
uint8_t Sub2_Pressure2_Reg             = 0x91;
uint8_t Sub1_Pressure3_Reg             = 0x92;
uint8_t Sub2_Pressure3_Reg             = 0x93;
uint8_t Sub1_Pressure4_Reg             = 0x94;
uint8_t Sub2_Pressure4_Reg             = 0x95;
uint8_t Sub1_Pressure5_Reg             = 0x96;
uint8_t Sub2_Pressure5_Reg             = 0x97;
uint8_t Sub1_Pressure6_Reg             = 0x98;
uint8_t Sub2_Pressure6_Reg             = 0x99;
uint8_t Sub1_Pressure7_Reg             = 0x9A;
uint8_t Sub2_Pressure7_Reg             = 0x9B;
uint8_t Sub1_Pressure8_Reg             = 0x9C;
uint8_t Sub2_Pressure8_Reg             = 0x9D;
uint8_t Sub1_Pressure9_Reg             = 0x9E;
uint8_t Sub2_Pressure9_Reg             = 0x9F;
uint8_t Sub1_Compensation_Pressure_Reg = 0xF9;
uint8_t Sub2_Compensation_Pressure_Reg = 0xF8;
uint8_t Sub3_Compensation_Pressure_Reg = 0xF7;

uint8_t Reset_Register         = 0x60;
uint8_t W_Config_meas_Register = 0x74;
uint8_t R_Config_meas_Register = 0xF4;
uint8_t W_Config_Register      = 0x75;
uint8_t R_Config_Register      = 0xF5;
uint8_t Status_Register        = 0xF3;

/* Initialization */
uint8_t Reset_Command = 0xB6;
uint8_t Config_meas   = 0x57;
uint8_t Config        = 0x90;
uint8_t Status        = 0;

/* Data needed for calculation */
uint16_t Temperature1            = 0;
int16_t Temperature2             = 0;
int16_t Temperature3             = 0;
int32_t Compensation_Temperature = 0;
uint16_t Pressure1               = 0;
int16_t Pressure2                = 0;
int16_t Pressure3                = 0;
int16_t Pressure4                = 0;
int16_t Pressure5                = 0;
int16_t Pressure6                = 0;
int16_t Pressure7                = 0;
int16_t Pressure8                = 0;
int16_t Pressure9                = 0;
int32_t Compensation_Pressure    = 0;

/* Temporary storage */
uint8_t Sub_Data1;
uint8_t Sub_Data2;
uint8_t Sub_Data3;

/* Calculation */
float Var1_T         = 0;
float Var2_T         = 0;
float t_fine         = 0;
float Var11_P        = 0;
float Var12_P        = 0;
float Var13_P        = 0;
float Var14_P        = 0;
float Var21_P        = 0;
float Var22_P        = 0;
float Var23_P        = 0;
float Var24_P        = 0;
float Pressure_temp1 = 0;
float Pressure_temp2 = 0;

/* Final Data */
float Temperature   = 0;
pressure_t Pressure = {0.0};

static void BMP280_MsgHandler(service_t *service, const msg_t *msg);

/* Receive and reply to an external Message */
static void BMP280_MsgHandler(service_t *service, const msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        msg_t pub_msg;
        pub_msg.header.cmd         = PRESSURE;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(pressure_t);
        memcpy(pub_msg.data, &Pressure, sizeof(pressure_t));
        /* Comment this line when using Pyluos to simulate pressure evolution */
        Luos_SendMsg(service, &pub_msg);
    }
    if (msg->header.cmd == PARAMETERS)
    {
        msg_t pub_msg;
        pub_msg.header.cmd         = PRESSURE;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(pressure_t);
        PressureOD_PressureToMsg(&Pressure, &pub_msg);
        Luos_SendMsg(service, &pub_msg);
        return;
    }
}

void BMP280_Init()
{
    SPI_Driver_Init();
    /* Resets the device */
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Reset_Register, 1);
    Send_SPI_Message(&Reset_Command, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);

    /* Write in the config register: sets time between 2 measurements, filter applied, enable or not 3 wire SPI */
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&W_Config_Register, 1);
    Send_SPI_Message(&Config, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);

    /* Write in the config_meas register: sets temperature data oversampling, pressure data oversampling, power mode */
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&W_Config_meas_Register, 1);
    Send_SPI_Message(&Config_meas, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);

    /* permits to see if the device is measuring or not */
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Status_Register, 1);
    Receive_SPI_Message(&Status, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);

    /* Creation of Luos Service */
    revision_t revision = {{{1, 0, 0}}};
    Luos_CreateService(BMP280_MsgHandler, PRESSURE_TYPE, "BMP280", revision);
}

void BMP280_Loop()
{
    SPI_Driver_Loop();
    /* Permits to retrieve data from a register */
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Temperature1_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    /* Data is divided in two bytes so we need to assemble it */
    Temperature1 = Sub_Data2 << 8;
    Temperature1 = Temperature1 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Temperature2_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Temperature2 = Sub_Data2 << 8;
    Temperature2 = Temperature2 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Temperature3_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Temperature3 = Sub_Data2 << 8;
    Temperature3 = Temperature3 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub3_Compensation_Temperature_Reg, 1);
    Receive_SPI_Message(&Sub_Data3, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Compensation_Temperature = Sub_Data1 >> 4;
    Compensation_Temperature = Compensation_Temperature | Sub_Data2 << 4;
    Compensation_Temperature = Compensation_Temperature | Sub_Data3 << 12;

    /* Determination of temperature with several calculations according to documentation */
    Var1_T      = (((float)Compensation_Temperature / (float)16384) - ((float)Temperature1 / (float)1024)) * (float)Temperature2;
    Var2_T      = (((float)Compensation_Temperature / (float)131072) - (float)Temperature1 / (float)8192) * (((float)Compensation_Temperature / (float)131072) - (float)Temperature1 / (float)8192) * (float)Temperature3;
    t_fine      = Var1_T + Var2_T;
    Temperature = (Var1_T + Var2_T) / 5120;

    /* Same process for Pressure */
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure1_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure1 = Sub_Data2 << 8;
    Pressure1 = Pressure1 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure2_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure2 = Sub_Data2 << 8;
    Pressure2 = Pressure2 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure3_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure3 = Sub_Data2 << 8;
    Pressure3 = Pressure3 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure4_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure4 = Sub_Data2 << 8;
    Pressure4 = Pressure4 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure5_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure5 = Sub_Data2 << 8;
    Pressure5 = Pressure5 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure6_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure6 = Sub_Data2 << 8;
    Pressure6 = Pressure6 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure7_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure7 = Sub_Data2 << 8;
    Pressure7 = Pressure7 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure8_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure8 = Sub_Data2 << 8;
    Pressure8 = Pressure8 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub1_Pressure9_Reg, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Pressure9 = Sub_Data2 << 8;
    Pressure9 = Pressure9 | Sub_Data1;

    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 0);
    Send_SPI_Message(&Sub3_Compensation_Pressure_Reg, 1);
    Receive_SPI_Message(&Sub_Data3, 1);
    Receive_SPI_Message(&Sub_Data2, 1);
    Receive_SPI_Message(&Sub_Data1, 1);
    HAL_GPIO_WritePin(SPIx_CS_GPIO_PORT, SPIx_CS_PIN, 1);
    Compensation_Pressure = Sub_Data1 >> 4;
    Compensation_Pressure = Compensation_Pressure | Sub_Data2 << 4;
    Compensation_Pressure = Compensation_Pressure | Sub_Data3 << 12;

    Var11_P        = ((float)t_fine / 2) - 64000;
    Var21_P        = Var11_P * Var11_P * ((float)Pressure6) / 32768;
    Var22_P        = Var21_P + Var11_P * ((float)Pressure5) * 2;
    Var23_P        = (Var22_P / 4) + (((float)Pressure4) * 65536);
    Var12_P        = (((float)Pressure3) * Var11_P * Var11_P / 524288 + ((float)Pressure2) * Var11_P) / 524288;
    Var13_P        = (1 + Var12_P / 32768) * ((float)Pressure1);
    Pressure_temp1 = 1048576 - (float)Compensation_Pressure;
    Pressure_temp2 = (Pressure_temp1 - (Var23_P / 4096)) * 6250 / Var13_P;
    Var14_P        = ((float)Pressure9) * Pressure_temp2 * Pressure_temp2 / 2147483648;
    Var24_P        = Pressure_temp2 * ((float)Pressure8) / 32768;
    Pressure       = PressureOD_PressureFrom_Pa(Pressure_temp2 + (Var14_P + Var24_P + ((float)Pressure7)) / 16);
}
