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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

static void Detec_ResetDetection(void);
/******************************************************************************
 * @brief all ptp interrupt handler
 * @param branch id
 * @return None
 ******************************************************************************/
void Detec_PtpHandler(branch_t branch)
{
    if (ctx.detection.expect == RELEASE)
    {
        // the line was released
        ctx.detection.keepline = NO_BRANCH;
        ctx.detection.expect = POKE;
        // Check if every line have been poked and poke it if not
        if (ctx.detection_mode != MASTER_DETECT)
        {
            for (int branch = 0; branch < NO_BRANCH; branch++)
            {
                if (ctx.detection.branches[branch] == 0)
                {
                    // this branch have not been detected
                    if (Detect_PokeBranch(branch))
                    {
                        //we get someone, go back to let the detection continue.
                        return;
                    }
                }
            }
            // if it is finished reset all lines
            for (int branch = 0; branch < NO_BRANCH; branch++)
            {
                LuosHAL_SetPTPDefaultState(branch);
            }
            Detec_ResetDetection();
        }
    }
    else if (ctx.detection.expect == POKE)
    {
        // we receive a poke, pull the line to notify your presence
        LuosHAL_PushPTP(branch);
        ctx.detection.keepline = branch;
    }
}
/******************************************************************************
 * @brief detect module by poke ptp line
 * @param branch id
 * @return None
 ******************************************************************************/
uint8_t Detect_PokeBranch(branch_t branch)
{
    // push the ptp line
    LuosHAL_PushPTP(branch);
    // wait a little just to be sure everyone can read it
    for (volatile unsigned int i = 0; i < TIMERVAL; i++)
        ;
    // release the ptp line
    LuosHAL_SetPTPDefaultState(branch);
    for (volatile unsigned int i = 0; i < TIMERVAL; i++)
        ;
    // Save branch as empty by default
    ctx.detection.branches[branch] = 0xFFFF;
    // read the line state
    if (LuosHAL_GetPTPState(branch))
    {
        // Someone reply, reverse the detection to wake up on line release
        LuosHAL_SetPTPReverseState(branch);
        ctx.detection.expect = RELEASE;
        ctx.detection.keepline = branch;
        // enable activ branch to get the next ID and save it into this branch number.
        ctx.detection.activ_branch = branch;
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
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        if (ctx.detection.branches[branch] == 0)
        {
            // this branch have not been poked
            if (Detect_PokeBranch(branch))
            {
                return;
            }
            else
            {
                // nobody is here
                ctx.detection.branches[branch] = 0xFFFF;
            }
        }
    }
    // no more branch need to be poked
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
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
    ctx.detection.keepline = NO_BRANCH;
    ctx.detection.detected_vm = 0;
    ctx.detection_mode = NO_DETECT;
    ctx.detection.expect = POKE;
    ctx.detection.activ_branch = NO_BRANCH;
}

void Detec_InitDetection(void)
{
    // reset all PTP
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        LuosHAL_SetPTPDefaultState(branch);
        ctx.detection.branches[branch] = 0;
    }
    Detec_ResetDetection();
    // Reinit VM id
    for (int i = 0; i < ctx.vm_number; i++)
    {
        ctx.vm_table[i].id = DEFAULTID;
    }
}
