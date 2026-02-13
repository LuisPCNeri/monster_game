#ifndef __TRAINER_H__
#define __TRAINER_H__

#include <SDL2/SDL.h>
#include "player/player.h"
#include "monsters/monster.h"

#define TRAINER_SPRITE_SIZE 128

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
    SDL_Texture* texture;
    monster_t party[PARTY_SIZE];
} trainer_t;

// Load trainers into memory
void TrainersInit();

/*
    Returns 1 if specified trainer is in the screen or close to it and 0 if not.
    \param t Trainer to check if is visible or not.
    \param offset_x Camera's offset on the x axis.
    \param offset_y Camera's offset on the y axis.
*/
int TrainerIsVisible(trainer_t* t, int offset_x, int offset_y);
void TrainerDraw(int offset_x, int offset_y);

/*
    Returns 1 if player is in LoS of the trainer and 0 if not.
    \param player Active player to track.
*/
int TrainerCheckAggro(player_t* player);

/*
    Takes care of carrying out the little animation that shows the exclamation mark above the trainer when it sees you.
    \param player Active player to keep track of.
    \param dt Time difference in between frames.
*/
void TrainerUpdateAggro(player_t* player, Uint32 dt);
int TrainerCheckPartyIsDead(trainer_t* trainer);
void TrainerRestoreParty(trainer_t* trainer);
void TrainerRenderNotifBox(trainer_t* t, int offset_x, int offset_y, Uint32 dt);

/*
    Returns 1 if player is overlapping with the closest trainer's rectangle.
    Returns 0 otherwise.

    \param *player Pointer to current active player.
*/
int TrainerIsCollingWithPlayer(player_t* player);

#endif