#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "monsters/monster.h"

typedef enum GameState{
    STATE_EXPLORING,
    STATE_BATTLE
} GameState;

// Struct to keep track of the items the player has
typedef struct{

} player_inventory_t;

typedef struct player_t{
    int x_pos;
    int y_pos;

    // Pointers to the monsters the player keeps with him and can use for battles
    monster_t* monster_party[5];

    GameState game_state;
} player_t;

#endif