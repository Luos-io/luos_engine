/******************************************************************************
 * @file filter.c
 * @brief Calculate filter for Phy and compare filter
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "filter.h"
#include "luos_utils.h"
#include "luos_hal.h"
#include "node.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    uint8_t TopicMask[TOPIC_MASK_SIZE]; /*!< multicast target bank. */
} filter_ctx_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
filter_ctx_t filter_ctx;

/*******************************************************************************
 * Functions
 ******************************************************************************/

void Filter_TopicInit(void)
{
    // Multicast mask init
    for (uint16_t i = 0; i < TOPIC_MASK_SIZE; i++)
    {
        filter_ctx.TopicMask[i] = 0;
    }
}

/******************************************************************************
 * @brief Add a Topic on the Mask
 * @param topic_id
 * @return None
 ******************************************************************************/
void Filter_AddTopic(uint16_t topic_id)
{
    LUOS_ASSERT(topic_id < MAX_LOCAL_TOPIC_NUMBER);
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
    LUOS_ASSERT(topic_id < MAX_LOCAL_TOPIC_NUMBER);
    // Remove 1 to the bit corresponding to the topic in multicast mask
    filter_ctx.TopicMask[(topic_id / 8)] &= ~(1 << (topic_id - ((int)(topic_id / 8)) * 8));
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
    if (topic_id < MAX_LOCAL_TOPIC_NUMBER)
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
