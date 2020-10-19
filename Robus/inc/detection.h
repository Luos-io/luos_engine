/******************************************************************************
 * @file detection
 * @brief detection state machine.
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef _DETECTION_H_
#define _DETECTION_H_

#include <robus.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TIMERVAL ((unsigned int)(0.00002 * MCUFREQ))

typedef enum
{
    POKE,
    RELEASE
} expected_detection_t;

typedef struct
{
    uint8_t keepline;         /*!< last keepline status on PTP lines . */
    uint16_t detected_vm; /*!< Virtual Container number. */
    uint8_t activ_branch;
} detection_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
void Detec_InitDetection(void);
void Detec_PtpHandler(uint8_t PTPNbr);
uint8_t Detect_PokeBranch(uint8_t PTPNbr);
void Detect_PokeNextBranch(void);
uint8_t Detec_OnePokedPTP(void);

#endif /* _DETECTION_H_ */
