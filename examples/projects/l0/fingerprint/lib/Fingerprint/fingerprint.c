/******************************************************************************
 * @file fingerprint
 * @brief This is a driver example for a fingerprint sensor. It will work as is
 * but if you want to see it inside of a project, I am using it to create a
 * biometric security system using Luos. You can go check this project on my github :
 * https://github.com/mariebidouille/STM32F0-Luos-Biometric-Security-System
 * @author mariebidouille
 * @version 0.0.0
 ******************************************************************************/
#include "fingerprint.h"
#include "fingerprint_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    FINGERPRINT_TYPE = LUOS_LAST_TYPE
};

typedef enum
{
    CHECK = LUOS_LAST_STD_CMD,
    ENROLL,
    DELETE
};

/*******************************************************************************
 * Variables
 ******************************************************************************/
static service_t *service_fingerprint;

/*******************************************************************************
 * Functions
 ******************************************************************************/
void Fingerprint_MsgHandler(service_t *service, const msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Fingerprint_Init(void)
{
    revision_t revision = {0};

    FingerprintDrv_Init();

    service_fingerprint = Luos_CreateService(Fingerprint_MsgHandler, FINGERPRINT_TYPE, "fingerprint", revision);
}

/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Fingerprint_Loop(void)
{
}

/******************************************************************************
 * @brief msg handler must be call when creating service
 * @param service
 * @param msg
 * @return None
 ******************************************************************************/
void Fingerprint_MsgHandler(service_t *service, const msg_t *msg)
{
    msg_t pub_msg;
    pub_msg.header.target      = msg->header.source;
    pub_msg.header.target_mode = msg->header.target_mode;
    pub_msg.header.cmd         = msg->header.cmd;
    pub_msg.header.size        = sizeof(char);

    switch (msg->header.cmd)
    {
        case ENROLL:
            pub_msg.data[0] = FingerprintDrv_Enroll();
            Luos_SendMsg(service, &pub_msg);
            break;

        case DELETE:
            pub_msg.data[0] = FingerprintDrv_DeleteAll();
            Luos_SendMsg(service, &pub_msg);
            break;

        case CHECK:
            pub_msg.data[0] = FingerprintDrv_CheckAuth();
            Luos_SendMsg(service, &pub_msg);
            break;

        case REINIT:
            FingerprintDrv_DeleteAll();
            FingerprintDrv_Init();
            break;

        default:
            break;
    }
}
