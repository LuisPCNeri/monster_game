#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "monsters/monster.h"
#include "inventory.h"

// Forward declaration to allow pointer usage without circular dependency
typedef struct menu_t menu_t;

typedef enum GameState{
    STATE_EXPLORING,
    STATE_IN_MENU,
    // State for when the player's movement is locked
    // Or for when the player is not supposed to move in between menus
    STATE_LOCKED
} GameState;

typedef struct player_t{
    int x_pos;
    int y_pos;

    // The index of the monster the player is currently using
    int active_mon_index;

    // Supposed to help see if in a battle it is the player's turn or not
    int is_player_turn;

    // Pointers to the monsters the player keeps with him and can use for battles
    monster_t* monster_party[5];
    int selected_menu_itm;

    menu_t* current_menu;

    GameState game_state;
    int running;

    inventory_t* inv;
} player_t;

// This function is called once and used to have the player chose a starter
// Returns the monster the player chooses
void PlayerSetStarters(player_t* player);

// Function to draw the starter choosing menu
void PlayerStarterMenuDraw();

// When the enter key is pressed if this is the current menu this function is called
// Gets the monster at the index of the current selected item, takes care of it's stats
// And puts it in the first slot of the player's party
void PlayerMenuHandleSelect();

// Adds a monster to the player's party or the PC if the party is full
// Returns 0 if the party is full
int PlayerAddMonsterToParty(monster_t* monster);

void PlayerDestroy(player_t* p);

#endif