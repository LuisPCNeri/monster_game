#ifndef __MAP_H__
#define __MAP_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#include "player/player.h"

#define TILE_MAP_MAX_X 100
#define TILE_MAP_MAX_Y 100
#define TILE_SIZE 32

typedef struct map_t{
    int** tile_data;
    int height;
    int width;
    SDL_Texture* tile_sheet;
} map_t;

// TILE TYPE DEFENITIONS
// This is just to prevent magic numbers and make the code more readable

// Tile to spawn GRASS, BUG, POISON, FAIRY, NORMAL TYPES
#define SPAWNABLE_TALL_GRASS 10
#define SPAWNABLE_ROCK_GROUND 11

/*
    Returns a pointer to the map texture created from a file.
    The caller is responsible for freeing the memory allocated for the map.
    \param map_file File from which the map will be created
    \param renderer SDL_Renderer for the window map is in
*/
map_t* MapCreateFromFile(FILE* map_file, SDL_Renderer* renderer);

/*
    Renders the map within the dimensions of viewport
    \param map Map to draw
    \param rend SDL_Renderer for the window map is being drawn at
    \param viewport SDL_Rect with the dimensions of the screen that tells the MapDraw func what rows and collumns of the map to load
*/
void MapDraw(map_t* map, SDL_Renderer* rend, SDL_Rect viewport);
/*
    Updates the SDL_Rect viewport taking into account the borders and camera clamping
    \param viewport The SDL_Rect viewport to update
    \param player Current active player
    \param max_w_px Total width of map in pixels
    \param max_h_px Total height of map in pixels
    \param screen_w Screen witdh in pixels
    \param screen_h Screen height in pixels
*/
void MapUpdateViewport(SDL_Rect* viewport, player_t* player, int map_w_px, int map_h_px, int screen_w, int screen_h);

// Frees memory allocated for map
void MapDestroy(map_t* map);

// Takes int x_pos and y_pos
// Returns the int corresponding to the type of the tile x_pos and y_pos are in or -1 if it fails
int GetCurrentTileType(int x_pos, int y_pos, map_t* map);

#endif