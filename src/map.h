#ifndef __MAP_H__
#define __MAP_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#define TILE_MAP_MAX_X 100
#define TILE_MAP_MAX_Y 100

// TILE TYPE DEFENITIONS
// This is just to prevent magic numbers and make the code more readable

// Tile to spawn GRASS, BUG, POISON, FAIRY, NORMAL TYPES
#define SPAWNABLE_TALL_GRASS 10
#define SPAWNABLE_ROCK_GROUND 11

// Loads the map texture to the screen
// SDL_Window* window is the window in which the map will be rendered on
SDL_Texture* CreateGameMap(SDL_Renderer* renderer);

// Takes int x_pos and y_pos
// Returns the int corresponding to the type of the tile x_pos and y_pos are in or -1 if it fails
int GetCurrentTileType(int x_pos, int y_pos);

#endif