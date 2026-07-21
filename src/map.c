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
#include "utils/glbl_asset_manager.h"

extern glbl_asset_manager* asset_manager;

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
/// \param min_cx Minimum visible chunk X.
/// \param min_cy Minimum visible chunk Y.
/// \param max_cx Maximum visible chunk X.
/// \param max_cy Maximum visible chunk Y.
/// \return The index of the first evictable slot in the map window or -1 if none available. 
static int8_t MapFindEvictableSlot(chunked_map_t* map, int32_t min_cx, int32_t max_cx, int32_t min_cy, int32_t max_cy) {

    for (int32_t i = 0; i < map->window_capacity; i++) {
        if (map->window[i].cx < 0) return i;
    }

    for (int32_t i = 0; i < map->window_capacity; i++) {
        loaded_chunk_slot_t* slot = &map->window[i];

        if (slot->cx < min_cx || slot->cx > max_cx || slot->cy < min_cy || slot->cy > max_cy) {
            return i;
        }
    }

    return -1;
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

chunked_map_t* MapInit(SDL_Renderer* renderer) {

    FILE* f = fopen("data/map.bin", "rb");
    if(!f) {
        printf(ANSI_COLOR_RED   "[!] Could not open map file.\n"  ANSI_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    map_header_t* header = read_map_header(f);

    chunked_map_t* map = (chunked_map_t*) malloc(sizeof(chunked_map_t));
    map->bin = f;

    SDL_Surface* tile_map_surface = SDL_LoadBMP("data/tiles.bmp");
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

    for(int32_t i = 0; i < map->window_capacity; i++) {
        map->window[i].cx = -1;
        map->window[i].cy = -1;
    }

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

static loaded_chunk_slot_t* MapFindSlot(chunked_map_t* m, int32_t cx, int32_t cy) {
    for(int32_t i = 0; i < m->window_capacity; i++) {
        if(m->window[i].cx == cx && m->window[i].cy == cy) return &m->window[i];
    }

    return NULL;
}

void MapUpdateStreaming(chunked_map_t* m, player_t* p) {
    int32_t pcx = COORDS_TO_CHUNK_COORD(p->x_pos);
    int32_t pcy = COORDS_TO_CHUNK_COORD(p->y_pos);
    
    int32_t visible_min_cx = pcx, visible_max_cx = pcx;
    int32_t visible_min_cy = pcy, visible_max_cy = pcy;

    /// Taking into account the padding chunks that are not necessarily visible
    int32_t load_min_cx = pcx - STREAM_MARGIN_CHUNKS;
    int32_t load_max_cx = pcx + STREAM_MARGIN_CHUNKS;
    int32_t load_min_cy = pcy - STREAM_MARGIN_CHUNKS;
    int32_t load_max_cy = pcy + STREAM_MARGIN_CHUNKS;

    if (load_min_cx < 0) load_min_cx = 0;
    if (load_min_cy < 0) load_min_cy = 0;
    if (load_max_cx >= (int32_t)m->w_chunks) load_max_cx = (int32_t)m->w_chunks - 1;
    if (load_max_cy >= (int32_t)m->h_chunks) load_max_cy = (int32_t)m->h_chunks - 1;

    // evict anything outside the load range
    for (int32_t i = 0; i < m->window_capacity; i++) {
        loaded_chunk_slot_t* slot = &m->window[i];
        if (slot->cx < 0) continue;
        if (slot->cx < load_min_cx || slot->cx > load_max_cx ||
            slot->cy < load_min_cy || slot->cy > load_max_cy) {
            slot->cx = -1;
            slot->cy = -1;
        }
    }

    // load anything in the load range that isn't already resident
    for (int32_t cy = load_min_cy; cy <= load_max_cy; cy++) {
        for (int32_t cx = load_min_cx; cx <= load_max_cx; cx++) {
            loaded_chunk_slot_t* slot = MapFindSlot(m, cx, cy);
            if (slot) {
                slot->data.is_visible = (cx >= visible_min_cx && cx <= visible_max_cx &&
                                          cy >= visible_min_cy && cy <= visible_max_cy);
                continue;
            }

            int32_t idx = MapFindEvictableSlot(m, load_min_cx, load_max_cx, load_min_cy, load_max_cy);
            if (idx < 0) {
                printf(ANSI_COLOR_RED "[!] No evictable slot for chunk (%d,%d)\n" ANSI_COLOR_RESET, cx, cy);
                continue;
            }
            slot = &m->window[idx];

            int64_t byte_offset = chunk_file_offset((uint32_t)cx, (uint32_t)cy, m->w_chunks);
            fseek(m->bin, (long)byte_offset, SEEK_SET);

            if (read_chunk(m->bin, &slot->data)) {
                slot->cx = cx;
                slot->cy = cy;
                slot->data.is_visible = (cx >= visible_min_cx && cx <= visible_max_cx &&
                                          cy >= visible_min_cy && cy <= visible_max_cy);
            } else {
                slot->cx = -1;
                slot->cy = -1;
            }
        }
    }
}

/// TODO Calculate Atlas size automatically
#define ATLAS_SIZE 2
#define ATLAS_TILE_SIZE 32

void MapRender(chunked_map_t* m, SDL_Renderer* r, SDL_Rect v) {
    for(int32_t i = 0; i < m->window_capacity; i++) {
        loaded_chunk_slot_t* chunk = &m->window[i];

        if(chunk->cx < 0) continue;

        int32_t c_origin_x = chunk->cx * CHUNK_SIZE * RENDER_TILE_SIZE;
        int32_t c_origin_y = chunk->cy * CHUNK_SIZE * RENDER_TILE_SIZE;

        for(int32_t x = 0; x < CHUNK_SIZE; x++) {
            for(int32_t y = 0; y < CHUNK_SIZE; y++) {
                
                int32_t idx = chunk->data.tiles[x][y].texture_id - 1;

                SDL_Rect src = {
                    .x = (idx % ATLAS_SIZE) * ATLAS_TILE_SIZE,
                    .y = (idx / ATLAS_SIZE) * ATLAS_TILE_SIZE,
                    .w = ATLAS_TILE_SIZE, 
                    .h = ATLAS_TILE_SIZE
                };

                SDL_Rect dst = {
                    c_origin_x + (x * RENDER_TILE_SIZE) - v.x,
                    c_origin_y + (y * RENDER_TILE_SIZE) - v.y,
                    RENDER_TILE_SIZE, RENDER_TILE_SIZE
                };

                SDL_RenderCopy(r, m->tile_sheet, &src, &dst);
            }
        }
    }
}

bin_tile_t* MapGetCurrentTile(chunked_map_t* map, int32_t px, int32_t py) {

    // convert pixel position to tile coords
    int32_t tile_x = px / TILE_SIZE;
    int32_t tile_y = py / TILE_SIZE;

    // convert tile coords to chunk coords
    int32_t cx = tile_x / CHUNK_SIZE;
    int32_t cy = tile_y / CHUNK_SIZE;

    // local position within the chunk
    int32_t local_x = tile_x % CHUNK_SIZE;
    int32_t local_y = tile_y % CHUNK_SIZE;

    loaded_chunk_slot_t* slot = MapFindSlot(map, cx, cy);

    if (!slot) {
        printf(ANSI_COLOR_RED "No chunk loaded for tile (%d,%d) chunk (%d,%d)\n" ANSI_COLOR_RESET,
               tile_x, tile_y, cx, cy);
        return NULL;
    }

    /*
    printf(ANSI_COLOR_GREEN "Found tile (%d,%d) in chunk (%d,%d) local (%d,%d) textureId: %d\n" ANSI_COLOR_RESET,
           tile_x, tile_y, cx, cy, local_x, local_y,
           slot->data.tiles[local_x][local_y].texture_id);
    */

    return &slot->data.tiles[local_x][local_y];
}

/// TODO FreeMap
void FreeMap(chunked_map_t* m){
    free(m->window);

    fclose(m->bin);
    SDL_DestroyTexture(m->tile_sheet);

    free(m);

    return;
}