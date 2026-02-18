/******************************************************************************
 * @file luos
 * @brief User functionalities of the Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdio.h>
#include "luos_engine.h"
#include "luos_hal.h"
#include "_timestamp.h"
#include "filter.h"
#include "service.h"
#include "struct_engine.h"
#include "luos_io.h"
#include "_luos_phy.h"
#include "stats.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
const revision_t luos_version = {.major = 3, .minor = 0, .build = 0};
package_t package_table[MAX_LOCAL_SERVICE_NUMBER + LOCAL_PHY_NB];
uint16_t package_number = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static error_return_t Luos_Send(service_t *service, msg_t *msg);
static inline void Luos_PackageInit(void);
static inline void Luos_PackageLoop(void);

/******************************************************************************
 * @brief Luos init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Init(void)
{
    Service_Init();
    Node_Init();
    LuosHAL_Init();
    LuosIO_Init();

#ifdef WITH_BOOTLOADER
    if (APP_START_ADDRESS == (uint32_t)FLASH_BASE)
    {
        if (LuosHAL_GetMode() != JUMP_TO_APP_MODE)
        {
            LuosHAL_JumpToAddress(BOOT_START_ADDRESS);
        }
    }
#endif
}
/******************************************************************************
 * @brief Luos Loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Loop(void)
{
    static uint32_t last_loop_date;
    phy_job_t *job = NULL;

#ifdef WITH_BOOTLOADER
    // After 3 Luos_Loop, consider this application as safe and write a flag to let the booloader know it can jump to the application safely.
    static uint8_t loop_count = 0;
    if (loop_count < 3)
    {
        loop_count++;
        if (loop_count == 3)
        {
            LuosHAL_SetMode((uint8_t)JUMP_TO_APP_MODE);
        }
    }
#endif

    // Check loop call time stat
    // Get node stats
    luos_stats_t *luos_stats = Stats_GetLuos();
    if ((LuosHAL_GetSystick() - last_loop_date) > luos_stats->max_loop_time_ms)
    {
        luos_stats->max_loop_time_ms = LuosHAL_GetSystick() - last_loop_date;
    }
    Node_Loop();
    LuosIO_Loop();
    // Look at all received jobs
    LUOS_MUTEX_LOCK
    while (LuosIO_GetNextJob(&job) != FAILED)
    {
        // We got a job
        // Try to deliver it to the services
        if (Service_Deliver(job) != FAILED)
        {
            // Services consume this job. remove it
            LuosIO_RmJob(job);
        }
    }
    LUOS_MUTEX_UNLOCK
    // manage timed auto update
    Service_AutoUpdateManager();
    // save loop date
    last_loop_date = LuosHAL_GetSystick();
}

/******************************************************************************
 * @brief Luos clear statistic
 * @param None
 * @return None
 ******************************************************************************/
void Luos_ResetStatistic(void)
{
    luos_stats_t *luos_stats = Stats_GetLuos();
    memset(&luos_stats->unmap[0], 0, sizeof(luos_stats_t));
    void Service_ResetStatistics(void);
}

/******************************************************************************
 * @brief return the Luos version
 * @param None
 * @return Luos version
 ******************************************************************************/
const revision_t *Luos_GetVersion(void)
{
    return &luos_version;
}

/******************************************************************************
 * @brief enable or disable IRQ
 * @param state : true to enable, false to disable
 * @return None
 ******************************************************************************/
void Luos_SetIrqState(bool state)
{
    Phy_SetIrqState(state);
}

/******************************************************************************
 * @brief Send msg through network
 * @param Service : Who send
 * @param Message : To send
 * @return None
 ******************************************************************************/
error_return_t Luos_SendMsg(service_t *service, msg_t *msg)
{
    LUOS_ASSERT(msg != 0);
    // set protocol version
    msg->header.config = BASE_PROTOCOL;
    return Luos_Send(service, msg);
}

/******************************************************************************
 * @brief Send msg through network
 * @param service : Who send
 * @param msg : Message to send
 * @param timestamp
 * @return SUCCEED : If the message is sent, else FAILED or PROHIBITED
 ******************************************************************************/
error_return_t Luos_SendTimestampMsg(service_t *service, msg_t *msg, time_luos_t timestamp)
{
    // set timestamp in message
    Timestamp_EncodeMsg(msg, timestamp);
    return Luos_Send(service, msg);
}

/******************************************************************************
 * @brief Send msg through network
 * @param service : Who send
 * @param msg : Message to send
 * @param timestamp
 * @return SUCCEED : If the message is sent, else FAILED or PROHIBITED
 ******************************************************************************/
static error_return_t Luos_Send(service_t *service, msg_t *msg)
{
    LUOS_ASSERT(msg != 0);
    if (service == 0)
    {
        // There is no service specified here, take the first one
        service = &Service_GetTable()[0];
    }
    if ((service->id == 0) && (msg->header.cmd >= LUOS_LAST_RESERVED_CMD))
    {
        // We are in detection mode and this command come from user
        // We can't send it
        return PROHIBITED;
    }

    // ********** Prepare the message ********************
    if (service->id != 0)
    {
        msg->header.source = service->id;
    }
    else
    {
        msg->header.source = Node_Get()->node_id;
    }
    if (LuosIO_Send(service, msg) == FAILED)
    {
        return FAILED;
    }
    return SUCCEED;
}

/******************************************************************************
 * @brief Read last message
 * @param service : The service asking for a message
 * @param msg_to_write : Message where the received message will be copied
 * @return SUCCEED : If a message is passed to the user, else FAILED
 ******************************************************************************/
error_return_t Luos_ReadMsg(service_t *service, msg_t *msg_to_write)
{
    LUOS_ASSERT((msg_to_write != 0) && (service != 0));
    uint8_t service_index = Service_GetIndex(service);
    phy_job_t *job        = NULL;
    LUOS_MUTEX_LOCK
    while (LuosIO_GetNextJob(&job) != FAILED)
    {
        // We got a job
        // Check if our service is concerned by this job
        if (((*(uint8_t *)job->phy_data) >> service_index) & 0x01)
        {
            uint16_t msg_size = job->msg_pt->header.size;
            // This job is for our service, copy the job message to the user message
            if (msg_size > MAX_DATA_MSG_SIZE)
            {
                msg_size = MAX_DATA_MSG_SIZE;
            }
            if (Luos_IsMsgTimstamped(job->msg_pt) == true)
            {
                memcpy(msg_to_write, job->msg_pt, sizeof(header_t) + msg_size + sizeof(time_luos_t));
            }
            else
            {
                memcpy(msg_to_write, job->msg_pt, sizeof(header_t) + msg_size);
            }
            // Remove this service from the job filter
            *(uint8_t *)job->phy_data &= ~(1 << service_index);
            // Services consume this job. try to remove it
            LuosIO_RmJob(job);
            LUOS_MUTEX_UNLOCK
            return SUCCEED;
        }
    }
    LUOS_MUTEX_UNLOCK
    return FAILED;
}

/******************************************************************************
 * @brief Read last msg from a specific service id
 * @param service : The service asking for a message
 * @param id : Who sent the message we are looking for
 * @param msg_to_write : Message where the received message will be copied
 * @return SUCCEED : If a message is passed to the user, else FAILED
 ******************************************************************************/
error_return_t Luos_ReadFromService(service_t *service, uint16_t id, msg_t *msg_to_write)
{

    LUOS_ASSERT((msg_to_write != 0) && (service != 0) && (id != 0));
    phy_job_t *job        = NULL;
    uint8_t service_index = Service_GetIndex(service);
    LUOS_MUTEX_LOCK
    while (LuosIO_GetNextJob(&job) != FAILED)
    {
        // We got a job
        // Check if our service is concerned by this job
        if ((*(service_filter_t *)job->phy_data >> service_index & 0x01) && (job->msg_pt->header.source == id))
        {
            uint16_t msg_size = job->msg_pt->header.size;
            // This job is for our service, copy the job message to the user message
            if (msg_size > MAX_DATA_MSG_SIZE)
            {
                msg_size = MAX_DATA_MSG_SIZE;
            }
            if (Luos_IsMsgTimstamped(job->msg_pt) == true)
            {
                memcpy(msg_to_write, job->msg_pt, sizeof(header_t) + msg_size + sizeof(time_luos_t));
            }
            else
            {
                // memcpy(msg_to_write, job->msg_pt, sizeof(header_t) + job->msg_pt->header.size);
                memcpy(msg_to_write, job->msg_pt, sizeof(header_t) + msg_size);
            }
            // Remove this service from the job filter
            *(service_filter_t *)job->phy_data &= ~(1 << service_index);
            // Services consume this job. try to remove it
            LuosIO_RmJob(job);
            LUOS_MUTEX_UNLOCK
            return SUCCEED;
        }
    }
    LUOS_MUTEX_UNLOCK
    return FAILED;
}

/******************************************************************************
 * @brief Read last msg with a specific command
 * @param service : The service asking for a message
 * @param cmd : Who sent the message we are looking for
 * @param msg_to_write : Message where the received message will be copied
 * @return SUCCEED : If a message is passed to the user, else FAILED
 ******************************************************************************/
error_return_t Luos_ReadFromCmd(service_t *service, uint8_t cmd, msg_t *msg_to_write)
{

    LUOS_ASSERT((msg_to_write != 0) && (service != 0));
    phy_job_t *job        = NULL;
    uint8_t service_index = Service_GetIndex(service);
    LUOS_MUTEX_LOCK
    while (LuosIO_GetNextJob(&job) != FAILED)
    {
        // We got a job
        // Check if our service is concerned by this job and if the command is the one we are looking for
        if ((*(service_filter_t *)job->phy_data >> service_index & 0x01) && (job->msg_pt->header.cmd == cmd))
        {
            uint16_t msg_size = job->msg_pt->header.size;
            // This job is for our service, copy the job message to the user message
            if (msg_size > MAX_DATA_MSG_SIZE)
            {
                msg_size = MAX_DATA_MSG_SIZE;
            }
            if (Luos_IsMsgTimstamped(job->msg_pt) == true)
            {
                memcpy(msg_to_write, job->msg_pt, sizeof(header_t) + msg_size + sizeof(time_luos_t));
            }
            else
            {
                memcpy(msg_to_write, job->msg_pt, sizeof(header_t) + msg_size);
            }
            // Remove this service from the job filter
            *(service_filter_t *)job->phy_data &= ~(1 << service_index);
            // Services consume this job. try to remove it
            LuosIO_RmJob(job);
            LUOS_MUTEX_UNLOCK
            return SUCCEED;
        }
    }
    LUOS_MUTEX_UNLOCK
    return FAILED;
}

/******************************************************************************
 * @brief Send large among of data and formating to send into multiple msg
 * @param service : Who send
 * @param message : Message to send
 * @param bin_data : Pointer to the message data table
 * @param size : Size of the data to transmit
 * @return None
 ******************************************************************************/
void Luos_SendData(service_t *service, msg_t *msg, void *bin_data, uint16_t size)
{
    LUOS_ASSERT((msg != 0) && (bin_data != 0) && (size != 0));
    // Compute number of message needed to send this data
    uint16_t msg_number = 1;
    uint16_t sent_size  = 0;
    if (size > MAX_DATA_MSG_SIZE)
    {
        msg_number = (size / MAX_DATA_MSG_SIZE);
        msg_number += (msg_number * MAX_DATA_MSG_SIZE < size);
    }

    // Send messages one by one
    for (uint16_t chunk = 0; chunk < msg_number; chunk++)
    {
        // Compute chunk size
        uint16_t chunk_size = 0;
        if ((size - sent_size) > MAX_DATA_MSG_SIZE)
        {
            chunk_size = MAX_DATA_MSG_SIZE;
        }
        else
        {
            chunk_size = size - sent_size;
        }

        // Copy data into message
        memcpy(msg->data, (uint8_t *)bin_data + sent_size, chunk_size);
        msg->header.size = size - sent_size;

        // Send message
        uint32_t tickstart = Luos_GetSystick();

        while (Luos_SendMsg(service, msg) == FAILED)
        {
            // No more memory space available
            // 500ms of timeout after start trying to load our data in memory. Perhaps the buffer is full of RX messages try to increase the buffer size.
            LUOS_ASSERT(((volatile uint32_t)Luos_GetSystick() - tickstart) < 500);
        }

        // Save current state
        sent_size = sent_size + chunk_size;
    }
}

/******************************************************************************
 * @brief Receive a multi msg data
 * @param service : who receive
 * @param msg : Message chunk received
 * @param bin_data : Pointer to data
 * @return Valid data received (negative values are errors)
 ******************************************************************************/
int Luos_ReceiveData(service_t *service, const msg_t *msg, void *bin_data)
{
    // Manage buffer session (one per service)
    static uint32_t data_size[MAX_LOCAL_SERVICE_NUMBER]       = {0};
    static uint32_t total_data_size[MAX_LOCAL_SERVICE_NUMBER] = {0};
    static uint16_t last_msg_size                             = 0;

    // When this function receive a data from a NULL service it is an error and we should reinit the reception state
    if (service == NULL)
    {
        memset(data_size, 0, sizeof(data_size));
        memset(total_data_size, 0, sizeof(total_data_size));
        last_msg_size = 0;
        return -1;
    }
    LUOS_ASSERT((msg != 0) && (bin_data != 0));

    uint16_t id = Service_GetIndex(service);

    // store total size of a msg
    if (total_data_size[id] == 0)
    {
        total_data_size[id] = msg->header.size;
    }

    LUOS_ASSERT(msg->header.size <= total_data_size[id]);

    // check message integrity
    if ((last_msg_size > 0) && (last_msg_size - MAX_DATA_MSG_SIZE > msg->header.size))
    {
        // we miss a message (a part of the data),
        // reset session and return an error.
        data_size[id] = 0;
        last_msg_size = 0;
        return -1;
    }

    // Get chunk size
    uint16_t chunk_size = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
    {
        chunk_size = MAX_DATA_MSG_SIZE;
    }
    else
    {
        chunk_size = msg->header.size;
    }

    // Copy data into buffer
    memcpy(bin_data + data_size[id], msg->data, chunk_size);

    // Save buffer session
    data_size[id] = data_size[id] + chunk_size;
    last_msg_size = msg->header.size;

    // check
    LUOS_ASSERT(data_size[id] <= total_data_size[id]);

    // Check end of data
    if (msg->header.size <= MAX_DATA_MSG_SIZE)
    {
        // Data collection finished, reset buffer session state
        data_size[id]       = 0;
        last_msg_size       = 0;
        uint32_t backup     = total_data_size[id];
        total_data_size[id] = 0;
        return backup;
    }
    return 0;
}

/******************************************************************************
 * @brief Return the number of messages available
 * @param None
 * @return The number of messages
 ******************************************************************************/
uint16_t Luos_NbrAvailableMsg(void)
{
    return LuosIO_GetJobNb();
}

/******************************************************************************
 * @brief Check if all Tx message are complete
 * @param None
 * @return SUCCEED : If Tx message are complete
 ******************************************************************************/
error_return_t Luos_TxComplete(void)
{
    return LuosIO_TxAllComplete();
}

/******************************************************************************
 * @brief Register a new package
 * @param Init : Init function name
 * @param Loop : Loop function name
 * @return None
 ******************************************************************************/
void Luos_AddPackage(void (*Init)(void), void (*Loop)(void))
{
    package_table[package_number].Init = Init;
    package_table[package_number].Loop = Loop;

    package_number += 1;
}

/******************************************************************************
 * @brief Run each package Init()
 * @param None
 * @return None
 ******************************************************************************/
void Luos_PackageInit(void)
{
    uint16_t package_index = 0;
    if (package_number)
    {
        while (package_index < package_number)
        {
            package_table[package_index].Init();
            package_index += 1;
        }
    }
    else
    {
        // Create an emply node service
        Luos_CreateService(0, VOID_TYPE, "empty_node", luos_version);
    }
}

/******************************************************************************
 * @brief Run each package Loop()
 * @param None
 * @return None
 ******************************************************************************/
void Luos_PackageLoop(void)
{
    uint16_t package_index = 0;
    while (package_index < package_number)
    {
        package_table[package_index].Loop();
        package_index += 1;
    }
}

/******************************************************************************
 * @brief Luos high level state machine
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Run(void)
{
    static bool node_run = false;

    if (!node_run)
    {
        Luos_Init();
        Luos_PackageInit();
        // go to run state after initialization
        node_run = true;
    }
    else
    {
        Luos_Loop();
        Luos_PackageLoop();
    }
}

/******************************************************************************
 * @brief Demand a detection
 * @param service : Service that launched the detection
 * @return None
 ******************************************************************************/
void Luos_Detect(service_t *service)
{
    msg_t detect_msg;

    if (Node_GetState() < LOCAL_DETECTION)
    {
        if (service->id == 0)
        {
            // We don't have any ID yet, let's create one at least for the detector
            // Set the detection launcher id to 1
            service->id = 1;
            // Update the filter just to accept our detector id
            Phy_AddLocalServices(1, 1);
        }

        // Send ask detection message
        detect_msg.header.target_mode = SERVICEIDACK;
        detect_msg.header.cmd         = ASK_DETECTION;
        detect_msg.header.size        = 0;
        detect_msg.header.target      = service->id;
        Luos_SendMsg(service, &detect_msg);
    }
}
