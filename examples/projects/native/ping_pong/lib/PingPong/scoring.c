#include "scoring.h"
#include <string.h>

#define SCORE_TABLE_SIZE 100

typedef struct __attribute__((__packed__))
{
    uint16_t score;
    uint16_t player_id;
} msg_score_t;

score_t scores[SCORE_TABLE_SIZE];
score_table_t score_table;

void score_init(service_t *player, search_result_t *player_list)
{
    Luos_Subscribe(player, SCORE_TOPIC);
    memset(scores, 0, sizeof(scores));
    score_table.player_nb = player_list->result_nbr;
    score_table.scores    = scores;
    for (int i = 0; i < score_table.player_nb; i++)
    {
        scores[i].alias = player_list->result_table[i]->alias;
        scores[i].id    = player_list->result_table[i]->id;
        scores[i].score = 0;
    }
}

void score_increase(service_t *player, uint16_t winning_player_id)
{
    // Update the scores
    for (int i = 0; i < score_table.player_nb; i++)
    {
        if (scores[i].alias == NULL)
        {
            break;
        }
        if (scores[i].id == winning_player_id)
        {
            scores[i].score++;
            // Sort the score table
            while (i > 0 && scores[i].score > scores[i - 1].score)
            {
                score_t tmp   = scores[i];
                scores[i]     = scores[i - 1];
                scores[i - 1] = tmp;
                i--;
            }
            break;
        }
    }
    // Generate the score message value
    msg_score_t msg_score[score_table.player_nb];
    for (int i = 0; i < score_table.player_nb; i++)
    {
        msg_score[i].score     = scores[i].score;
        msg_score[i].player_id = scores[i].id;
    }

    msg_t msg;
    msg.header.target      = SCORE_TOPIC;
    msg.header.target_mode = TOPIC;
    msg.header.size        = sizeof(msg_score);
    msg.header.cmd         = SCORE_CMD;
    memcpy(msg.data, &msg_score, sizeof(msg_score));
    Luos_SendMsg(player, &msg);
}

void score_update(const msg_t *msg)
{
    score_t tmp_score[SCORE_TABLE_SIZE];
    const msg_score_t *msg_score = (const msg_score_t *)msg->data;
    // Transform the message value into a score table
    for (int i = 0; i < score_table.player_nb; i++)
    {
        tmp_score[i].score = msg_score[i].score;
        tmp_score[i].id    = msg_score[i].player_id;
        for (int j = 0; j < score_table.player_nb; j++)
        {
            if (tmp_score[i].id == scores[j].id)
            {
                tmp_score[i].alias = scores[j].alias;
                break;
            }
        }
    }
    // Update the score table
    for (int i = 0; i < score_table.player_nb; i++)
    {
        scores[i].score = tmp_score[i].score;
        scores[i].id    = tmp_score[i].id;
        scores[i].alias = tmp_score[i].alias;
    }
}

score_table_t *get_score(void)
{
    return &score_table;
}
