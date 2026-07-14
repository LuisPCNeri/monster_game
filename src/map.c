#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#include "map.h"
#include "player/player.h"
#include "utils/term_colors.h"

SDL_Rect select_tile[16];

static void MapLoadSelectionTiles(){
    for(uint32_t i=0; i < 4; i++){
        for(uint32_t k=0; k < 4; k++){
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
    for(int32_t i = 0; i < map->width; i++){
        map->tile_data[i] = calloc(map->height, sizeof(int));
    }

    fseek(map_file, 0, SEEK_SET);

    MapLoadSelectionTiles();

    u_int32_t row = 0;
    u_int32_t max_col = 0;
    char line[1024];

    while(fgets(line, sizeof(line), map_file)){
        if(row >= TILE_MAP_MAX_Y) break;

        u_int32_t col = 0;
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
    int32_t start_col = viewport.x / 32;
    int32_t end_col = (viewport.x + viewport.w)/ 32;
    int32_t start_row = viewport.y / 32;
    int32_t end_row = (viewport.y + viewport.h) / 32;

    if (start_col < 0) start_col = 0;
    if (start_row < 0) start_row = 0;
    if (end_col >= map->width) end_col = map->width - 1;
    if (end_row >= map->height) end_row = map->height - 1;

    for(int32_t x = start_col; x <= end_col; x++){
        for(int32_t y = start_row; y <= end_row; y++){
            if(x < 0 || x > map->width || y < 0 || y > map->height) continue;
            int8_t tile_id = map->tile_data[x][y];

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

void MapUpdateViewport(SDL_Rect* viewport, player_t* player, int32_t map_w_px, int32_t map_h_px, int32_t screen_w, int32_t screen_h){
    viewport->x = player->x_pos - (screen_w / 2);
    viewport->y = player->y_pos - (screen_h / 2);

    if(viewport->x < 0) viewport->x = 0;
    if(viewport->y < 0) viewport->y = 0;

    if (viewport->x > map_w_px - viewport->w) viewport->x = map_w_px - viewport->w;
    if (viewport->y > map_h_px - viewport->h) viewport->y = map_h_px - viewport->h;
}

void MapDestroy(map_t* map){
    if(!map) return;

    for(int32_t i = 0; i < map->width; i++){
        free(map->tile_data[i]);
    }

    free(map->tile_data);
    SDL_DestroyTexture(map->tile_sheet);
    free(map);
}

int8_t GetCurrentTileType(int32_t x_pos, int32_t y_pos, map_t* map){
    // If somehow someway one of the positions is negative and we want to check the tile type
    // return an error value 
    if(x_pos < 0 || y_pos < 0 || x_pos >= TILE_MAP_MAX_X || y_pos >= TILE_MAP_MAX_Y) return -1;

    return map->tile_data[x_pos][y_pos];
}

/*
  =================================
  ====== CHUNK LOADING LOGIC ====== (NEW)
  =================================
*/

typedef struct map_header_t {
    uint32_t magic_number;

    uint32_t map_width_chunks;
    uint32_t map_height_chunks;

    int32_t origin_x;
    int32_t origin_y;

} map_header_t;

static int64_t chunk_file_offset(uint32_t cx, uint32_t cy, uint32_t w_chunks) {
    return HEADER_SERIALIZED_SIZE + (int64_t)(cy * w_chunks + cx) * CHUNK_SERIALIZED_SIZE;
}

/// \brief Finds the first slot available to be reused in the loaded chunks sliding window.
/// \param map The current in use map struct.
/// \param start_range The first chunk currently being rendered or used in the screen.
/// \param end_range The last chunk currently being rendered or used in the screen.
/// \return The index of the first evictable slot in the map window. 
static int8_t MapFindEvictableSlot(chunked_map_t* map, int8_t start_range, int8_t end_range) {
    return 0;
}

/// \brief Reads the Map Header from map.bin
/// \param f Map binary file
/// \return Returns a pointer to a map header exits the program when can't read map header memory was corrupted or map file does not exist.
static map_header_t* read_map_header(FILE* f) {
    map_header_t* header = (map_header_t*) malloc(sizeof(map_header_t));

    if(fread(header, 1, HEADER_SERIALIZED_SIZE, f) <= 0) {
        free(header);

        printf(ANSI_COLOR_RED   "[!] Read 0 or less bytes from map header.\n"  ANSI_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    return header;
}

chunked_map_t* MapInit(const char* fpath, SDL_Renderer* renderer) {

    FILE* f = fopen("../data/map.bin", "rb");
    if(!f) {
        printf(ANSI_COLOR_RED   "[!] Could not open map file.\n"  ANSI_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    map_header_t* header = read_map_header(f);

    chunked_map_t* map = (chunked_map_t*) malloc(sizeof(chunked_map_t));
    map->bin = f;

    SDL_Surface* tile_map_surface = SDL_LoadBMP("./resources/tiles.bmp");
    if(!tile_map_surface) {
        printf(ANSI_COLOR_RED"[!] Error loading tiles.bmp: %s\n"ANSI_COLOR_RESET, SDL_GetError());
        exit(EXIT_FAILURE);
    }
    map->tile_sheet = SDL_CreateTextureFromSurface(renderer, tile_map_surface);
    SDL_FreeSurface(tile_map_surface);

    map->w_chunks = header->map_width_chunks;
    map->h_chunks = header->map_height_chunks;

    /// Uninitialized value to be replaced by the value on the map header
    map->origin_x = header->origin_x;
    map->origin_y = header->origin_y;

    map->window_capacity = MAP_SIZE_CHUNK;
    map->window = (loaded_chunk_slot_t*) calloc(map->window_capacity, sizeof(loaded_chunk_slot_t));

    return map;
}

#define COORDS_TO_CHUNK_COORD(coord) ((int32_t) ((coord/TILE_SIZE)/CHUNK_SIZE))

/// \brief Reads a chunk from the map binary file. File pointer needs to be at the START of the chunk's data. 
/// \param f File to read chunk from.
/// \param chunk Pointer to an address to store the chunk data.
/// \return 0 on FAILURE 1 on SUCCESS
static int8_t read_chunk(FILE* f, chunk_t* chunk) {
    for (int32_t x = 0; x < CHUNK_SIZE; x++) {
        for (int32_t y = 0; y < CHUNK_SIZE; y++) {
            bin_tile_t* t = &chunk->tiles[x][y];

            if (fread(&t->spawn_id_count, sizeof(uint8_t), 1, f) != 1) return 0;
            if (fread(t->spawn_ids, sizeof(int16_t), MAX_SPAWN_IDS, f) != MAX_SPAWN_IDS) return 0;
            if (fread(&t->texture_id, sizeof(uint16_t), 1, f) != 1) return 0;
        }
    }

    return 1;
}

/// TODO Check as to not load chunks if current chunk == last frame chunk to save on CPU
void MapLoadChunks(chunked_map_t* m, player_t* p) {
    int32_t pcx, pcy;

    pcx = COORDS_TO_CHUNK_COORD(p->x_pos);
    pcy = COORDS_TO_CHUNK_COORD(p->y_pos);

    int64_t byte_offset = chunk_file_offset(pcx, pcy, m->w_chunks);

    fseek(m->bin, byte_offset, SEEK_SET);
    loaded_chunk_slot_t* pchunk = &m->window[0];

    if(fread(pchunk, 1, CHUNK_SERIALIZED_SIZE, m->bin) <= 0) {
        printf(ANSI_COLOR_RED   "[!] Could not read chunk at (%d,%d)\n"  ANSI_COLOR_RESET, pcx, pcy);
        exit(EXIT_FAILURE);
    }
}

/// TODO FreeMap
void FreeMap(chunked_map_t* m){
    return;
}