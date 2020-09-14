/*
 * detection.c
 *
 *  Author: Nicolas Rabault
 *  Abstract: detection state machine.
 */
#include <robus.h>
#include "sys_msg.h"
#include "hal.h"

/**
 * \fn ptp_handler(branch_t branch)
 * \brief all ptp interrupt handler
 *
 * \param branch branch id
 */
void ptp_handler(branch_t branch)
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
                    if (poke(branch))
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
                    reset_PTP(branch);
                }
                reset_detection();
            }
        }
    }
    else if (ctx.detection.expect == POKE)
    {
        // we receive a poke, pull the line to notify your presence
        set_PTP(branch);
        ctx.detection.keepline = branch;
    }
}

/**
 * \fn unsigned char poke(branch_t branch)
 * \brief detect the next module
 *
 * \param branch branch id
 * \return 1 if there is someone, 0 if not
 */
unsigned char poke(branch_t branch)
{
    // push the ptp line
    set_PTP(branch);
    // wait a little just to be sure everyone can read it
    for (volatile unsigned int i = 0; i < TIMERVAL; i++)
        ;
    // release the ptp line
    reset_PTP(branch);
    for (volatile unsigned int i = 0; i < TIMERVAL; i++)
        ;
    // read the line state
    if (get_PTP(branch))
    {
        // Someone reply, reverse the detection to wake up on line release
        reverse_detection(branch);
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

/**
 * \fn void poke_next_branch(void)
 * \brief find the next branch to poke and poke it
 */
void poke_next_branch(void)
{
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        if (ctx.detection.branches[branch] == 0)
        {
            // this branch have not been poked
            if (poke(branch))
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
        reset_PTP(branch);
    }
    reset_detection();
    return;
}

/**
 * \fn void reset_detection(void)
 * \brief reinit the detection state machine
 *
 * \param *vm virtual module who start the detection
 * \return return the number of detected module
 */
void reset_detection(void)
{
    ctx.detection.keepline = NO_BRANCH;
    ctx.detection.detected_vm = 0;
    ctx.detection_mode = NO_DETECT;
    ctx.detection.expect = POKE;
    ctx.detection.activ_branch = NO_BRANCH;
}

unsigned char reset_network_detection(vm_t *vm)
{
    for (unsigned char branch = 0; branch < NO_BRANCH; branch++)
    {
        reset_PTP(branch);
        ctx.detection.branches[branch] = 0;
    }
    reset_detection();
    msg_t msg;

    msg.header.target = BROADCAST_VAL;
    msg.header.target_mode = BROADCAST;
    msg.header.cmd = RESET_DETECTION;
    msg.header.size = 0;

    //we don't have any way to tell every modules to reset their detection do it twice to be sure
    if (robus_send_sys(vm, &msg))
        return 1;
    if (robus_send_sys(vm, &msg))
        return 1;

    // Reinit VM id
    for (int i = 0; i < ctx.vm_number; i++)
    {
        ctx.vm_table[i].id = DEFAULTID;
    }
    return 0;
}
