#ifndef __MAP_H__
#define __MAP_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#define TILE_MAP_MAX_X 100
#define TILE_MAP_MAX_Y 100

// Loads the map texture to the screen
// SDL_Window* window is the window in which the map will be rendered on
SDL_Texture* create_map(SDL_Renderer* renderer);

#endif