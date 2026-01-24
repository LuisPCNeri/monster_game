#ifndef __TRAINER_H__
#define __TRAINER_H__

#include "player/player.h"
#include "monsters/monster.h"

typedef struct trainer_t
{   
    int x_pos;
    int y_pos;
    int facing_direction;

    // TODO : Trainer inventory

    MonsterTypes type;
    monster_t party[PARTY_SIZE];
} trainer_t;

trainer_t* TrainerCreate(int x_pos, int y_pos, int facing_direction, MonsterTypes type, monster_t* party);
trainer_t* TrainerFromJSON();


#endif