#ifndef __TRAINER_H__
#define __TRAINER_H__

#include "player/player.h"
#include "monsters/monster.h"

typedef enum FacingDirection{
    FRONT,
    BACK,
    RIGHT,
    LEFT
} FacingDirection;

typedef struct trainer_t
{   
    int x_pos;
    int y_pos;
    int was_defeated;
    FacingDirection facing_direction;

    MonsterTypes type;

    char sprite_path[256];
    char name[512];
    char intro_msg[1024];
    monster_t party[PARTY_SIZE];
} trainer_t;

// Load trainers into memory
void TrainersInit();
int TrainerIsVisible(trainer_t* t, int offset_x, int offset_y);
void TrainerDraw(int offset_x, int offset_y);
// If a trainer has agrroed the player returns 1 if not returns 0
int TrainerCheckAggro(player_t* player);
void TrainerUpdateAggro(player_t* player);
int TrainerCheckPartyIsDead(trainer_t* trainer);
void TrainerRestoreParty(trainer_t* trainer);
void TrainerRenderNotifBox(trainer_t* t, int offset_x, int offset_y);

#endif