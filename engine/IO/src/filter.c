/******************************************************************************
 * @file filter.c
 * @brief Calculate filter for Phy and compare filter
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "filter.h"
#include "luos_utils.h"
#include "luos_hal.h"
#include "service.h"
#include "node.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    uint8_t IDMask[ID_MASK_SIZE];
    uint16_t IDShiftMask;
    uint8_t TopicMask[TOPIC_MASK_SIZE]; /*!< multicast target bank. */
} filter_ctx_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
filter_ctx_t filter_ctx;

/*******************************************************************************
 * Functions
 ******************************************************************************/

/******************************************************************************
 * @brief Reset Masks
 * @param None
 * @return None
 ******************************************************************************/
void Filter_IdInit(void)
{
    // Id mask init
    filter_ctx.IDShiftMask = 0;
    for (uint16_t i = 0; i < ID_MASK_SIZE; i++)
    {
        filter_ctx.IDMask[i] = 0;
    }
}

void Filter_TopicInit(void)
{
    // Multicast mask init
    for (uint16_t i = 0; i < TOPIC_MASK_SIZE; i++)
    {
        filter_ctx.TopicMask[i] = 0;
    }
}

/******************************************************************************
 * @brief ID Mask calculation
 * @param service_id ID of the first service
 * @param service_number Number of the services on the node
 * @return None
 ******************************************************************************/
void Filter_AddServiceId(uint16_t service_id, uint16_t service_number)
{
    // 4096 bit address 512 byte possible
    // Create a mask of only possibility in the node
    //--------------------------->|__________|
    //	Shift byte		            byte Mask of bit address

    LUOS_ASSERT((service_id > 0)
                && (service_id <= 4096 - MAX_SERVICE_NUMBER)
                && (service_number <= MAX_SERVICE_NUMBER));
    Filter_IdInit();
    uint16_t tempo         = 0;
    filter_ctx.IDShiftMask = (service_id - 1) / 8; // aligned to byte

    // Create a mask of bit corresponding to ID number in the node
    for (uint16_t i = 0; i < service_number; i++)
    {
        tempo = (((service_id - 1) + i) - (8 * filter_ctx.IDShiftMask));
        filter_ctx.IDMask[tempo / 8] |= 1 << ((tempo) % 8);
    }
}

/******************************************************************************
 * @brief Add a Topic on the Mask
 * @param topic_id
 * @return None
 ******************************************************************************/
void Filter_AddTopic(uint16_t topic_id)
{
    LUOS_ASSERT(topic_id < LAST_TOPIC);
    // Add 1 to the bit corresponding to the topic in multicast mask
    filter_ctx.TopicMask[(topic_id / 8)] |= 1 << (topic_id - ((int)(topic_id / 8)) * 8);
}

/******************************************************************************
 * @brief Remove a Topic on the Mask
 * @param topic_id
 * @return None
 ******************************************************************************/
void Filter_RmTopic(uint16_t topic_id)
{
    LUOS_ASSERT(topic_id < LAST_TOPIC);
    // Remove 1 to the bit corresponding to the topic in multicast mask
    filter_ctx.TopicMask[(topic_id / 8)] &= ~(1 << (topic_id - ((int)(topic_id / 8)) * 8));
}

/******************************************************************************
 * @brief Parse msg to find a service concerne
 * @param header of message
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL bool Filter_ServiceID(uint16_t service_id)
{
    //--------------------------->|__________|
    //	      Shift byte		  byte Mask of bit address
    // In an node, service ID are consecutive
    // MaskID is byte field wich have the size of MAX_SERVICE_NUMBER
    // Shift depend od ID of first service in Node (shift = NodeID/8)
    LUOS_ASSERT(service_id <= 4096);
    uint16_t compare = 0;

    if ((service_id > (8 * filter_ctx.IDShiftMask))) // IDMask aligned byte
    {
        // Calcul ID mask for ID received
        compare = ((service_id - 1) - ((8 * filter_ctx.IDShiftMask)));
        // Check if compare and internal mask match
        if ((filter_ctx.IDMask[compare / 8] & (1 << (compare % 8))) != 0)
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * @brief Parse multicast mask to find if target exists
 * @param topic_id of message
 * @return bool true if there is one false if not
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL bool Filter_Topic(uint16_t topic_id)
{
    uint8_t compare = 0;
    // Make sure there is a topic that can be received by the node
    if (topic_id < LAST_TOPIC)
    {
        compare = topic_id - ((topic_id / 8) * 8);
        // Search if topic exists in mask
        if ((filter_ctx.TopicMask[(topic_id / 8)] & (1 << compare)) != 0)
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * @brief Parse all services type to find if target exists
 * @param type_id of message
 * @return bool true if there is one false if not
 * We can't do this the same way other are done because identifiers don't have any continuity
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL bool Filter_Type(uint16_t type_id)
{
    LUOS_ASSERT(type_id <= 4096);
    // Check all service type
    for (int i = 0; i < Service_GetNumber(); i++)
    {
        if (type_id == Service_GetTable()[i].type)
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * @brief Parse msg to find a service concerne
 * @param header of message
 * @return None
 * _CRITICAL function call in IRQ
 ******************************************************************************/
_CRITICAL uint8_t Filter_GetPhyTarget(header_t *header)
{
    // Find if we are concerned by this message.
    // check if we need to filter all the messages

    switch (header->target_mode)
    {
        case SERVICEIDACK:
        case SERVICEID:
            // Check all service id
            if (Filter_ServiceID(header->target))
            {
                // This concerns Luos phy only
                return 0x01;
            }
            break;
        case TYPE:

            if (Filter_Type(header->target))
            {
                // This concerns Luos phy and Robus
                return 0x01 | (0x01 << 1);
            }
            break;
        case BROADCAST:
            // This concerns Luos phy and Robus
            return 0x01 | (0x01 << 1);
            break;
        case NODEIDACK:
        case NODEID:
            // If the target is our node and our node ave a node_id or if we don't have a node_id and we are waiting for one.
            if (((header->target == Node_Get()->node_id) && (header->target != 0))
                || ((header->target == 0) && (Node_WaitId() == true)))
            {
                // This concerns Luos phy only
                return 0x01;
            }
            break;
        case TOPIC:
            if (Filter_Topic(header->target))
            {
                // This concerns Luos phy and Robus
                return 0x01 | (0x01 << 1);
            }
            break;
        default:
            // We can be here in case of corrupted message.
            // In this case nobody is concerned by this message.
            return 0x00;
            break;
    }
    // This concerns Robus only by default
    return 0x01 << 1;
}
