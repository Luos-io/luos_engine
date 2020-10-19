/******************************************************************************
 * @file detection
 * @brief detection state machine.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdbool.h>
#include "detection.h"
#include "sys_msg.h"
#include "context.h"
#include "LuosHAL.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
expected_detection_t PTP_ExpectedState = POKE;
/*******************************************************************************
 * Function
 ******************************************************************************/

static void Detec_ResetDetection(void);
/******************************************************************************
 * @brief init the detection state machine
 * @param None
 * @return None
 ******************************************************************************/
void Detec_InitDetection(void)
{
    // reset all PTP
    for (uint8_t branch = 0; branch < NBR_BRANCH; branch++)
    {
        LuosHAL_SetPTPDefaultState(branch);
        ctx.node.port_table[branch] = 0;
    }
    Detec_ResetDetection();
    // Reinit VM id
    for (uint8_t i = 0; i < ctx.vm_number; i++)
    {
        ctx.vm_table[i].id = DEFAULTID;
    }
}
/******************************************************************************
 * @brief all ptp interrupt handler
 * @param branch id
 * @return None
 ******************************************************************************/
void Detec_PtpHandler(uint8_t PTPNbr)
{
    if (PTP_ExpectedState == RELEASE)
    {
        // the line was released
        ctx.detection.keepline = NBR_BRANCH;
        PTP_ExpectedState = POKE;
        // Check if every line have been poked and poke it if not
        for (uint8_t branch = 0; branch < NBR_BRANCH; branch++)
        {
            if (ctx.node.port_table[branch] == 0)
            {
                return;
            }
        }
        // if it is finished reset all lines
        for (uint8_t branch = 0; branch < NBR_BRANCH; branch++)
        {
            LuosHAL_SetPTPDefaultState(branch);
        }
        Detec_ResetDetection();
    }
    else if (PTP_ExpectedState == POKE)
    {
        // we receive a poke, pull the line to notify your presence
        LuosHAL_PushPTP(PTPNbr);
        ctx.detection.keepline = PTPNbr;
    }
}
/******************************************************************************
 * @brief detect module by poke ptp line
 * @param branch id
 * @return None
 ******************************************************************************/
uint8_t Detect_PokeBranch(uint8_t PTPNbr)
{
    // push the ptp line
    LuosHAL_PushPTP(PTPNbr);
    // wait a little just to be sure everyone can read it
    for (volatile unsigned int i = 0; i < TIMERVAL; i++)
        ;
    // release the ptp line
    LuosHAL_SetPTPDefaultState(PTPNbr);
    for (volatile unsigned int i = 0; i < TIMERVAL; i++)
        ;
    // Save branch as empty by default
    ctx.node.port_table[PTPNbr] = 0xFFFF;
    // read the line state
    if (LuosHAL_GetPTPState(PTPNbr))
    {
        // Someone reply, reverse the detection to wake up on line release
        LuosHAL_SetPTPReverseState(PTPNbr);
        PTP_ExpectedState = RELEASE;
        ctx.detection.keepline = PTPNbr;
        // enable activ branch to get the next ID and save it into this branch number.
        ctx.detection.activ_branch = PTPNbr;
        return 1;
    }
    // Nobodies reply to our poke
    return 0;
}
/******************************************************************************
 * @brief detect the next module by poke ptp line
 * @param None
 * @return None
 ******************************************************************************/
void Detect_PokeNextBranch(void)
{
    for (uint8_t branch = 0; branch < NBR_BRANCH; branch++)
    {
        if (ctx.node.port_table[branch] == 0)
        {
            // this branch have not been poked
            if (Detect_PokeBranch(branch))
            {
                return;
            }
            else
            {
                // nobody is here
                ctx.node.port_table[branch] = 0xFFFF;
            }
        }
    }
    // no more branch need to be poked
    for (uint8_t branch = 0; branch < NBR_BRANCH; branch++)
    {
        LuosHAL_SetPTPDefaultState(branch);
    }
    Detec_ResetDetection();
    return;
}
/******************************************************************************
 * @brief reinit the detection state machine
 * @param None
 * @return None
 ******************************************************************************/
void Detec_ResetDetection(void)
{
    ctx.detection.keepline = NBR_BRANCH;
    ctx.detection.detected_vm = 0;
    PTP_ExpectedState = POKE;
    ctx.detection.activ_branch = NBR_BRANCH;
}
