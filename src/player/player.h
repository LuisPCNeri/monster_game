#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdint.h>
#include <SDL2/SDL.h>
#include "monsters/monster.h"
#include "inventory.h"

#define PARTY_SIZE 5
#define BLINK_FRAMES 500
#define CHARACTER_SPEED 300
#define PLAYER_SPRITE_SIZE 64
#define PLAYER_AGGRO_TIMER 1500

// Forward declaration to allow pointer usage without circular dependency
typedef struct menu_t menu_t;
typedef struct trainer_t trainer_t;

typedef enum Orientation{
    SOUTH,
    NORTH,
    WEST,
    EAST,
} Orientation;

typedef enum GameState{
    STATE_EXPLORING,
    STATE_IN_MENU,
    // State for when the player's movement is locked
    // Or for when the player is not supposed to move in between menus
    STATE_LOCKED,
    STATE_AGGRO
} GameState;

typedef struct player_t{
    // Pointers to the monsters the player keeps with him and can use for battles
    monster_t* monster_party[5];

    SDL_Texture* sprite_sheet;
    menu_t* current_menu;
    inventory_t* inv;
    trainer_t* aggro_trainer;
    monster_t* aggro_monster;

    GameState game_state;
    Orientation facing_direction;

    int32_t aggro_timer;
    int32_t x_pos;
    int32_t y_pos;

    SDL_Rect sprite_rect;

    int8_t inv_isOpen;
    // The index of the monster the player is currently using
    int8_t active_mon_index;
    // Supposed to help see if in a battle it is the player's turn or not
    int8_t is_player_turn;
    int8_t selected_menu_itm;
    int8_t running;
    int8_t sprite_stage;
} player_t;

/*
    Returns a pointer to a newly created player struct. The caller ir responsible for freeing the allocate memory.
*/
player_t* PlayerInit();

// This function is called once and used to have the player chose a starter
// Returns the monster the player chooses
void PlayerSetStarters(player_t* player);

// Function to draw the starter choosing menu
void PlayerStarterMenuDraw(Uint32 dt);

// When the enter key is pressed if this is the current menu this function is called
// Gets the monster at the index of the current selected item, takes care of it's stats
// And puts it in the first slot of the player's party
void PlayerMenuHandleSelect();

// Adds a monster to the player's party or the PC if the party is full
// Returns 0 if the party is full
int8_t PlayerAddMonsterToParty(monster_t* monster);

// Returns 1 if all of the player's monsters are dead and 0 if there is at least one alive
int8_t PlayerCheckIsPartyDead(player_t* player);

void PlayerRenderNotifBox(player_t* player, int32_t offset_x, int32_t offset_y, Uint32 dt);

/*
    Makes the player character move i.e. Player Character Controller
    \param player Active player struct
    \param direction Direction the player character will move
    \param *world_pos A pointer to the variable holding either the world_x pos or world_y pos of the player
*/
void PlayerMove(player_t* player, Orientation direction, int* world_pos);

SDL_Rect PlayerGetSheetWindow(player_t* player);

void PlayerDestroy(player_t* p);

#endif