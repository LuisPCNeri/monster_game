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

typedef struct map_t{
    SDL_Texture* tile_sheet;
    int32_t height;
    int32_t width;
    int8_t** tile_data;
} map_t;

#define MAX_SPAWN_IDS 32
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
void MapUpdateViewport(SDL_Rect* viewport, player_t* player, int32_t map_w_px, int32_t map_h_px, 
    int32_t screen_w, int32_t screen_h);

// Frees memory allocated for map
void MapDestroy(map_t* map);

// Takes int x_pos and y_pos
// Returns the int corresponding to the type of the tile x_pos and y_pos are in or -1 if it fails
int8_t GetCurrentTileType(int32_t x_pos, int32_t y_pos, map_t* map);

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