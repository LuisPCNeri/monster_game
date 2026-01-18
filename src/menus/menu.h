#ifndef __MENU_H__
#define __MENU_H__

#include <SDL2/SDL.h>
#include "player/player.h"

typedef struct player_t player_t;

typedef struct menu_t{
    SDL_Rect* menu_items;
    int items_amount;

    // If there is more than one row this should be >= 1
    int has_rows;
    // If there is more than one column this should be >= 1
    int has_columns;

    // Draw function that takes a menu_t menu as argument
    void (*draw)();

    // This is a pointer to the menu's specified select function
    // Should handle the select input from the player
    void (*select_routine)();

    // Generic function that goes back in the given menu
    void (*back)();
} menu_t;

// Creates a menu_items array with item_num elements literaly
// If the menu has more than one row has_rows >= 1
// If the menu has more than one column has_columns >= 1
// void* draw will be used as the function to draw the menu on the screen
// void* select_func will be the function called when the ENTER key is pressed and the menu is open 
menu_t* MenuCreate(int item_num, int has_rows, int has_columns, void* draw_func, void* select_func);

// Frees the space allocated for menu_t menu
void MenuDestroy(menu_t* menu);

// Renders the button's text and a rect for them
void MenuRenderItem(const char* btn_text, SDL_Rect* dst_rect);

// Wrapper function that just runs the current menu's select function
void MenuSelectCurrentItem(player_t* player);

void MenuHighlightBox(SDL_Rect* rect);
void MenuDeHighlightBox(SDL_Rect* rect);

void MenuItemKeyUp(player_t* player);
void MenuItemKeyDown(player_t* player);
void MenuItemKeyLeft(player_t* player);
void MenuItemKeyRight(player_t* player);

#endif