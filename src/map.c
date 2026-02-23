#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// SDL2 includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#include "map.h"

int tile_map[TILE_MAP_MAX_X][TILE_MAP_MAX_Y];
SDL_Rect select_tile[16];

SDL_Texture* MapCreateFromFile(FILE* map_file, SDL_Renderer* renderer){
    // Ensure we are at the start of the file
    fseek(map_file, 0, SEEK_SET);

    // Initialize tile_map to 0
    memset(tile_map, 0, sizeof(tile_map));

    for(int i=0; i < 4; i++){
        for(int k=0; k < 4; k++){
            // Go row by row in the image
            select_tile[i*4 + k].x = TILE_SIZE * k;
            select_tile[i*4 + k].y = TILE_SIZE * i;
            select_tile[i*4 + k].w = TILE_SIZE;
            select_tile[i*4 + k].h = TILE_SIZE;
        }
    }

    int row = 0;
    int max_col = 0;
    char line[1024];

    while(fgets(line, sizeof(line), map_file)){
        if(row >= TILE_MAP_MAX_Y) break;

        int col = 0;
        char* token = strtok(line, " \n\r;,");
        while(token){
            if(col < TILE_MAP_MAX_X){
                // Store as [col][row] (x, y)
                tile_map[col][row] = atoi(token);
            }
            col++;
            token = strtok(NULL, " \n\r;,");
        }
        if(col > max_col) max_col = col;
        // Only increment row if we actually read something
        if(col > 0) row++;
    }

    // Create surface for the tile examples
    SDL_Surface* tile_map_surface = SDL_LoadBMP("./resources/tiles.bmp");
    if(!tile_map_surface) {
        printf("Error loading tiles.bmp: %s\n", SDL_GetError());
        return NULL;
    }
    // Create texture from surface
    SDL_Texture* tile_texture = SDL_CreateTextureFromSurface(renderer, tile_map_surface);
    // Textures were loaded into GPU memory no need to keep them here
    SDL_FreeSurface(tile_map_surface);

    // Calculate texture dimensions based on actual map size
    if (max_col > TILE_MAP_MAX_X) max_col = TILE_MAP_MAX_X;
    int w = max_col * TILE_SIZE;
    int h = row * TILE_SIZE;
    
    // Create a target texture to draw the map onto
    SDL_Texture* map_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetRenderTarget(renderer, map_texture);

    SDL_RenderClear(renderer);

    for(int x=0; x < max_col; x++){
        for(int y=0; y < row; y++){
            int tile_id = tile_map[x][y];
            if(tile_id > 0 && tile_id <= 16) {
                SDL_Rect dest_rect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, tile_texture, &select_tile[tile_id - 1], &dest_rect);
            }
        }
    }

    printf("TEXTURES COPIED\n");

    // Reset render target to the window
    SDL_SetRenderTarget(renderer, NULL);

    SDL_DestroyTexture(tile_texture);
    return map_texture;    
}

SDL_Texture* CreateGameMap(SDL_Renderer* renderer){
    // Set the map dimensions to acount for the number of tiles defined in the map.h file
    int w = TILE_MAP_MAX_X * TILE_SIZE;
    int h = TILE_MAP_MAX_Y * TILE_SIZE;

    printf("X TILES: %d\n", TILE_MAP_MAX_X);
    printf("Y TILES: %d\n", TILE_MAP_MAX_Y);

    for(int i=0; i < 4; i++){
        for(int k=0; k < 4; k++){
            // Go row by row in the image
            select_tile[i*4 + k].x = TILE_SIZE * k;
            select_tile[i*4 + k].y = TILE_SIZE * i;
            select_tile[i*4 + k].w = TILE_SIZE;
            select_tile[i*4 + k].h = TILE_SIZE;
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
    // TODO : In theory I will have a file with the map's tile information (the tile type number)
    // And will load the tiles based on what is in that file

    srand(time(NULL));
    // 2D array with the tiles
    // TODO : May be better off allocating the memory in the heap when populating the map
    
    
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
            tile[x][y].x = x*TILE_SIZE;
            tile[x][y].y = y*TILE_SIZE;
            tile[x][y].w = TILE_SIZE;
            tile[x][y].h = TILE_SIZE;
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

/* IMPORTANT : Next in order of business -> Allocate the map in the heap and make it dynamic
i.e. when a certain row/column exits the screen free that memory and when a new row/column enters the screen allocate it and render it */

int GetCurrentTileType(int x_pos, int y_pos){
    // If somehow someway one of the positions is negative and we want to check the tile type
    // return an error value 
    if(x_pos < 0 || y_pos < 0 || x_pos >= TILE_MAP_MAX_X || y_pos >= TILE_MAP_MAX_Y) return -1;

    return tile_map[x_pos][y_pos];
}