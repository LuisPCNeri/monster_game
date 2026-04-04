#ifndef __TRAINER_H__
#define __TRAINER_H__

#include <stdint.h>
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

typedef struct trainer_t{
    monster_t party[PARTY_SIZE];

    char* intro_msg;
    char* name;
    char* sprite_path;
    SDL_Texture* texture;
    
    int32_t x_pos;
    int32_t y_pos;

    FacingDirection facing_direction;
    MonsterTypes type;  

    int8_t was_defeated;  
} trainer_t;

// Load trainers into memory
void TrainersInit();

/*
    Returns 1 if specified trainer is in the screen or close to it and 0 if not.
    \param t Trainer to check if is visible or not.
    \param offset_x Camera's offset on the x axis.
    \param offset_y Camera's offset on the y axis.
*/
int8_t TrainerIsVisible(trainer_t* t, int32_t offset_x, int32_t offset_y);
void TrainerDraw(int32_t offset_x, int32_t offset_y);

/*
    Returns 1 if player is in LoS of the trainer and 0 if not.
    \param player Active player to track.
*/
int8_t TrainerCheckAggro(player_t* player);

/*
    Takes care of carrying out the little animation that shows the exclamation mark above the trainer when it sees you.
    \param player Active player to keep track of.
    \param dt Time difference in between frames.
*/
void TrainerUpdateAggro(player_t* player, Uint32 dt);
int8_t TrainerCheckPartyIsDead(trainer_t* trainer);
void TrainerRestoreParty(trainer_t* trainer);
void TrainerRenderNotifBox(trainer_t* t, int32_t offset_x, int32_t offset_y, Uint32 dt);

/*
    Returns 1 if player is overlapping with the closest trainer's rectangle.
    Returns 0 otherwise.

    \param *player Pointer to current active player.
*/
int8_t TrainerIsCollingWithPlayer(player_t* player);

#endif