#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "monsters/monster.h"
// Forward declaration to allow pointer usage without circular dependency
typedef struct menu_t menu_t;

typedef enum GameState{
    STATE_EXPLORING,
    STATE_IN_MENU
} GameState;

// Struct to keep track of the items the player has
typedef struct{

} player_inventory_t;

typedef struct player_t{
    int x_pos;
    int y_pos;

    // Pointers to the monsters the player keeps with him and can use for battles
    monster_t* monster_party[5];
    int selected_menu_itm;

    menu_t* current_menu;

    GameState game_state;
    int running;
} player_t;

// This function is called once and used to have the player chose a starter
// Returns the monster the player chooses
monster_t* PlayerSetStarter(player_t* player);

// Function to draw the starter choosing menu
void PlayerStarterMenuDraw(menu_t* menu);

#endif