/******************************************************************************
 * @file scoring
 * @brief the score management of the game
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef SCORING_H
#define SCORING_H

#include "luos_engine.h"

#define SCORE_TOPIC 1
#define SCORE_CMD   LUOS_LAST_STD_CMD + 1

typedef struct
{
    uint8_t score;
    char *alias;
    uint16_t id;
} score_t;

typedef struct
{
    uint8_t player_nb;
    score_t *scores;
} score_table_t;

void score_init(service_t *player, search_result_t *player_list);
void score_increase(service_t *player, uint16_t winning_player_id);
void score_update(const msg_t *msg);
score_table_t *get_score(void);

#endif /* SCORING_H */
