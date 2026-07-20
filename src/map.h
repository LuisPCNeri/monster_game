#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#include "player/player.h"

#define TILE_MAP_MAX_X 100
#define TILE_MAP_MAX_Y 100
#define TILE_SIZE 32
#define RENDER_TILE_SIZE 96

#define CHUNK_SIZE 16

#define MAX_SPAWN_IDS 16
#define MAP_MAGIC_NUMBER        0x4D415031
#define TILE_SERIALIZED_SIZE    (1 + MAX_SPAWN_IDS * 2 + 2)                         /// 67 bytes
#define CHUNK_SERIALIZED_SIZE   (CHUNK_SIZE * CHUNK_SIZE * TILE_SERIALIZED_SIZE)    /// 17152 bytes
#define HEADER_SERIALIZED_SIZE  (sizeof(uint32_t)*3 + sizeof(int32_t)*2)            /// 20 bytes

/// 9 16x16 chunks of 32x32 tiles
/// Total area of map in chunks
#define MAP_SIZE_CHUNK 25
#define RENDERED_MAP_SIZE_CHUNK 9
#define STREAM_MARGIN_CHUNKS 2

typedef struct bin_tile_t {
    uint8_t spawn_id_count;
    int16_t spawn_ids[MAX_SPAWN_IDS];
    uint16_t texture_id;
    
} bin_tile_t;

typedef struct chunk_t {
    bin_tile_t tiles[CHUNK_SIZE][CHUNK_SIZE];
    int8_t is_visible;
} chunk_t;

typedef struct loaded_chunk_slot_t {
    /// if slot is empty both cx, cy = -1
    int32_t cx, cy;
    chunk_t data;
} loaded_chunk_slot_t;


typedef struct chunked_map_t {
    SDL_Texture* tile_sheet;
    uint32_t w_chunks, h_chunks;
    int32_t origin_x, origin_y;

    FILE* bin;
    loaded_chunk_slot_t* window;
    int32_t window_capacity;
} chunked_map_t;

/*
    \brief Function to get the tile the player, or object, is currently on, only works for objects present in the map chunk window.
    Objects that are not loaded in will return NULL.

    \param map The currently loaded map object.
    \param px The x position of the object.
    \param py The y position of the object.
    \return A pointer to the bin_tile_t struct holding the tile data, or NULL.
*/
bin_tile_t* MapGetCurrentTile(chunked_map_t* map, int32_t px, int32_t py);

/*
    \brief Initializes a chunked_map_t struct using the map binary file at fpath. 
    This function will not return errors and IS a cancelation point, i.e. the program will exit if there was an error.
    \param fpath Path to the map binary file.
    \param renderer Current SDL_Renderer*.
    \return A pointer to a chunked_map_t variable.
*/
chunked_map_t* MapInit(SDL_Renderer* renderer);

/*
    \brief Loads the chunks around the the player.
    \param m Current chunked_map_t data.
    \param p Active player.
*/
void MapUpdateStreaming(chunked_map_t* m, player_t* p);

void MapRender(chunked_map_t* m, SDL_Renderer* r, SDL_Rect v);

/*
    \brief Free all memory occupied by a chunked_map_t struct.
    \param m Map to free.
*/
void FreeMap(chunked_map_t* m);

#endif