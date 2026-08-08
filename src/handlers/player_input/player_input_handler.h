#ifndef __PLAYER_INPUT_HANDLER__
#define __PLAYER_INPUT_HANDLER__

#include "SDL_events.h"
#include "player/player.h"
#include "map.h"

void HandlePlayerInput(player_t* player, chunked_map_t* cm, int8_t* is_game_running, int32_t* world_x, int32_t* world_y, SDL_Event event);

#endif
