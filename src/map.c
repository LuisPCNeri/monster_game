#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#include "map.h"
#include "player/player.h"

SDL_Rect select_tile[16];

static void MapLoadSelectionTiles(){
    for(int i=0; i < 4; i++){
        for(int k=0; k < 4; k++){
            // Go row by row in the image
            select_tile[i*4 + k].x = TILE_SIZE * k;
            select_tile[i*4 + k].y = TILE_SIZE * i;
            select_tile[i*4 + k].w = TILE_SIZE;
            select_tile[i*4 + k].h = TILE_SIZE;
        }
    }
}

map_t* MapCreateFromFile(FILE* map_file, SDL_Renderer* renderer){

    map_t* map = (map_t*) malloc(sizeof(map_t));
    map->height = TILE_MAP_MAX_Y;
    map->width = TILE_MAP_MAX_X;

    map->tile_data = malloc(map->width * sizeof(int*));
    for(int i = 0; i < map->width; i++){
        map->tile_data[i] = calloc(map->height, sizeof(int));
    }

    fseek(map_file, 0, SEEK_SET);

    MapLoadSelectionTiles();

    int row = 0;
    int max_col = 0;
    char line[1024];

    while(fgets(line, sizeof(line), map_file)){
        if(row >= TILE_MAP_MAX_Y) break;

        int col = 0;
        char* token = strtok(line, " \n\r;,");
        while(token){
            if(col < TILE_MAP_MAX_X){
                map->tile_data[col][row] = atoi(token);
            }
            col++;
            token = strtok(NULL, " \n\r;,");
        }
        if(col > max_col) max_col = col;
        if(col > 0) row++;
    }

    SDL_Surface* tile_map_surface = SDL_LoadBMP("./resources/tiles.bmp");
    if(!tile_map_surface) {
        printf("Error loading tiles.bmp: %s\n", SDL_GetError());
        return NULL;
    }
    map->tile_sheet = SDL_CreateTextureFromSurface(renderer, tile_map_surface);
    SDL_FreeSurface(tile_map_surface);
    return map;    
}

void MapDraw(map_t* map, SDL_Renderer* rend, SDL_Rect viewport){
    int start_col = viewport.x / 32;
    int end_col = (viewport.x + viewport.w)/ 32;
    int start_row = viewport.y / 32;
    int end_row = (viewport.y + viewport.h) / 32;

    if (start_col < 0) start_col = 0;
    if (start_row < 0) start_row = 0;
    if (end_col >= map->width) end_col = map->width - 1;
    if (end_row >= map->height) end_row = map->height - 1;

    for(int x = start_col; x <= end_col; x++){
        for(int y = start_row; y <= end_col; y++){
            if(x < 0 || x > map->width || y < 0 || y > map->height) continue;
            int tile_id = map->tile_data[x][y];

            if(tile_id <= 0 || tile_id > 16) continue;

            SDL_Rect dst = {
                (x * TILE_SIZE) - viewport.x,
                (y * TILE_SIZE) - viewport.y,
                TILE_SIZE, TILE_SIZE
            };

            SDL_RenderCopy(rend, map->tile_sheet, &select_tile[tile_id - 1], &dst);
        }
    }
}

void MapUpdateViewport(SDL_Rect* viewport, player_t* player, int map_w_px, int map_h_px, int screen_w, int screen_h){
    viewport->x = player->x_pos - (screen_w / 2);
    viewport->y = player->y_pos - (screen_h / 2);

    if(viewport->x < 0) viewport->x = 0;
    if(viewport->y < 0) viewport->y = 0;

    if (viewport->x > map_w_px - viewport->w) viewport->x = map_w_px - viewport->w;
    if (viewport->y > map_h_px - viewport->h) viewport->y = map_h_px - viewport->h;
}

void MapDestroy(map_t* map){
    if(!map) return;

    for(int i = 0; i < map->width; i++){
        free(map->tile_data[i]);
    }

    free(map->tile_data);
    SDL_DestroyTexture(map->tile_sheet);
    free(map);
}

int GetCurrentTileType(int x_pos, int y_pos, map_t* map){
    // If somehow someway one of the positions is negative and we want to check the tile type
    // return an error value 
    if(x_pos < 0 || y_pos < 0 || x_pos >= TILE_MAP_MAX_X || y_pos >= TILE_MAP_MAX_Y) return -1;

    return map->tile_data[x_pos][y_pos];
}