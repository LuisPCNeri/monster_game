#include "handlers/player_input/player_input_handler.h"
#include "SDL_events.h"
#include "trainers/trainer.h"
#include "menus/menu.h"

#include <SDL2/SDL.h>

void HandlePlayerInput(player_t *player, chunked_map_t *cm, int8_t* running, int32_t* world_x, int32_t* world_y, SDL_Event event) {
    switch (event.type) {
    case SDL_QUIT:
        *running = 0;
        break;
    case SDL_KEYDOWN:
        switch(player->game_state){
            case STATE_EXPLORING:
            // keyboard API for key pressed
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_UP:
                    if(TrainerIsCollingWithPlayer(player) == NORTH) break;
                    PlayerMove(player, NORTH, world_y);
                    if(!TrainerCheckAggro(player)) MonsterTrySpawn(player, cm);
                    break;
                case SDL_SCANCODE_LEFT:
                    if(TrainerIsCollingWithPlayer(player) == WEST) break;
                    PlayerMove(player, WEST, world_x);
                    if(!TrainerCheckAggro(player)) MonsterTrySpawn(player, cm);
                    break;
                case SDL_SCANCODE_DOWN:
                    if(TrainerIsCollingWithPlayer(player) == SOUTH) break;
                    PlayerMove(player, SOUTH, world_y);
                    if(!TrainerCheckAggro(player)) MonsterTrySpawn(player, cm);
                    break;
                case SDL_SCANCODE_RIGHT:
                    if(TrainerIsCollingWithPlayer(player) == EAST) break;
                    PlayerMove(player, EAST, world_x);
                    if(!TrainerCheckAggro(player)) MonsterTrySpawn(player, cm);
                    break;
                case SDL_SCANCODE_ESCAPE:
                    *running = 0;
                    break;
                default:
                    break;
                }
                break;
            case STATE_IN_MENU:
                switch (event.key.keysym.scancode){
                    case SDL_SCANCODE_UP:
                        MenuItemKeyUp(player);
                        break;
                    case SDL_SCANCODE_LEFT:
                        MenuItemKeyLeft(player);
                        break;
                    case SDL_SCANCODE_DOWN:
                        MenuItemKeyDown(player);
                        break;
                    case SDL_SCANCODE_RIGHT:
                        MenuItemKeyRight(player);
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        if(!player->current_menu->back) break;
                        player->current_menu->back();
                        break;
                    case SDL_SCANCODE_RETURN:
                        MenuSelectCurrentItem(player);
                        break;
                    default:
                        break;
                }
                break;
            case STATE_LOCKED:
            break;
            case STATE_AGGRO:
            break;
        }
    }
}
