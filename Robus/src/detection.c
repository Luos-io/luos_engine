/******************************************************************************
 * @file detection
 * @brief detection state machine.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
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
            if (ctx.detection.detection_end)
            {
                for (int branch = 0; branch < NO_BRANCH; branch++)
                {
                	LuosHAL_SetPTPDefaultState(branch);
                }
                Detec_ResetDetection();
            }
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
    else
    {
        // Nobodies reply to our poke
        // Save branch as empty
        ctx.detection.branches[branch] = 0xFFFF;
    }
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
/******************************************************************************
 * @brief reinit the detection state machine
 * @param None
 * @return None
 ******************************************************************************/
uint8_t Detec_ResetNetworkDetection(vm_t *vm)
{
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        LuosHAL_SetPTPDefaultState(branch);
        ctx.detection.branches[branch] = 0;
    }
    Detec_ResetDetection();
    msg_t msg;

    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd = RESET_DETECTION;
    msg.header.size = 0;

    //we don't have any way to tell every modules to reset their detection do it twice to be sure
    if (Transmit_RobusSendSys(vm, &msg))
        return 1;
    if (Transmit_RobusSendSys(vm, &msg))
        return 1;

    // Reinit VM id
    for (int i = 0; i < ctx.vm_number; i++)
    {
        ctx.vm_table[i].id = DEFAULTID;
    }
    return 0;
}
