#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// SDL2 includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#include "map.h"

SDL_Texture* create_map(SDL_Renderer* renderer){
    // Set the map dimensions to acount for the number of tiles defined in the map.h file
    int w = TILE_MAP_MAX_X * 32;
    int h = TILE_MAP_MAX_Y * 32;

    printf("X TILES: %d\n", TILE_MAP_MAX_X);
    printf("Y TILES: %d\n", TILE_MAP_MAX_Y);

    // Create the array from where we will select the tiles
    SDL_Rect select_tile[16];

    for(int i=0; i < 4; i++){
        for(int k=0; k < 4; k++){
            // Go row by row in the image
            select_tile[i*4 + k].x = 32 * k;
            select_tile[i*4 + k].y = 32 * i;
            select_tile[i*4 + k].w = 32;
            select_tile[i*4 + k].h = 32;
        }
    }

    printf("SELECT TILE DONE\n");

    // Create surface for the tile examples
    SDL_Surface* tile_map_surface = SDL_LoadBMP("./resources/tiles.bmp");
    // Create texture from surface
    SDL_Texture* tile_texture = SDL_CreateTextureFromSurface(renderer, tile_map_surface);
    // Textures were loaded into GPU memory no need to keep them here
    SDL_FreeSurface(tile_map_surface);

    // PLACEHOLDER
    // TODO In theory I will have a file with the map's tile information (the tile type number)
    // And will load the tiles based on what is in that file

    srand(time(NULL));
    // 2D array with the tiles
    // TODO May be better off allocating the memory in the heap when populating the map
    int tile_map[TILE_MAP_MAX_X][TILE_MAP_MAX_Y];
    
    // Randomly assign the tiles
    for(int x=0; x < TILE_MAP_MAX_X; x++){
        for(int y=0; y < TILE_MAP_MAX_Y; y++){
            // Assign values from 1 to 16 to the tile
            tile_map[x][y] = rand() % 16 + 1;
        }
    }

    printf("TILE MAP INSTANCIALIZED\n");

    // Populate the actual screen with the tiles
    SDL_Rect tile[TILE_MAP_MAX_X][TILE_MAP_MAX_Y];
    for(int y=0; y < TILE_MAP_MAX_Y; y++){
        for(int x=0; x < TILE_MAP_MAX_X; x++){
            tile[x][y].x = x*32;
            tile[x][y].y = y*32;
            tile[x][y].w = 32;
            tile[x][y].h = 32;
        }
    }

    printf("SCREEN POPULATED\n");

    // Create a target texture to draw the map onto
    SDL_Texture* map_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetRenderTarget(renderer, map_texture);

    SDL_RenderClear(renderer);
    SDL_Delay(20);

    for(int x=0; x < TILE_MAP_MAX_X; x++){
        for(int y=0; y < TILE_MAP_MAX_Y; y++){
            SDL_RenderCopy(renderer, tile_texture, &select_tile[tile_map[x][y] - 1], &tile[x][y]);
        }
    }

    printf("TEXTURES COPIED\n");

    // Reset render target to the window
    SDL_SetRenderTarget(renderer, NULL);

    SDL_DestroyTexture(tile_texture);
    return map_texture;
}