#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

#include "map.h"
#include "player/player.h"
#include "menus/menu.h"
#include "monsters/monster.h"
#include "monsters/battle/battle.h"
#include "trainers/trainer.h"

#define WINDOW_TITLE "WINDOW"
#define CHARACTER_SPEED 300
#define FONT_SIZE 32

SDL_Renderer* rend;
TTF_Font* game_font;
int screen_w;
int screen_h;

int main(int argc, char* argv[])
{
    MonstersInit();
    TrainersInit();
    player_t* player = (player_t*) calloc(1, sizeof(player_t));
    player->x_pos = 0;
    player->y_pos = 0;
    player->inv_isOpen = 0;
    // Player's absolute position in the world
    int world_x = 0, world_y = 0;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) 
        printf("error initializing SDL: %s\n", SDL_GetError());
    if( TTF_Init() != 0 )            
        printf("Error initializing TTF: %s\n", TTF_GetError());
    if( MIX_INIT_MP3 != Mix_Init(MIX_INIT_MP3)) 
        printf("ERROR on MIXER: %s\n", Mix_GetError());

    SDL_Window* win = SDL_CreateWindow("GAME", // creates a window
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1000, 1000, SDL_WINDOW_FULLSCREEN_DESKTOP);

    // Frame rate is capped at the monitor's refresh rate because of SDL_RENDERER_PRESENTVSYNC
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC;
    rend = SDL_CreateRenderer(win, -1, render_flags);
    SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

    // Set up TTF
    game_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", FONT_SIZE);
    Mix_OpenAudio(22050, AUDIO_S16SYS, 1, 1024);

    player->inv = InventoryCreateEmpty();
    catch_device_t ball = { 1, 0, 1, "Ball", "" };
    InventoryAddItem(player->inv, &ball, 15);
    restore_item_t potion = {4, 1, 10, "Potion", ""};
    InventoryAddItem(player->inv, &potion, 5);

    SDL_Texture* map_tex = CreateGameMap(rend);

    int map_w, map_h;
    SDL_QueryTexture(map_tex, NULL, NULL, &map_w, &map_h);

    SDL_Surface* surface;

    surface = IMG_Load("./resources/test.jpeg");
    if (!surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        PlayerDestroy(player);
        SDL_DestroyTexture(map_tex);
        TTF_CloseFont(game_font);
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(win);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture* player_texture = SDL_CreateTextureFromSurface(rend, surface);
    SDL_FreeSurface(surface);

    // let us control our image position
    // so that we can move it with our keyboard.
    SDL_Rect player_rect = {0, 0, 0, 0};
    // connects our texture with player_rect to control position
    SDL_QueryTexture(player_texture, NULL, NULL, &player_rect.w, &player_rect.h);

    player_rect.w = PLAYER_SPRITE_SIZE;
    player_rect.h = PLAYER_SPRITE_SIZE;

    int screen_w, screen_h;
    SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

    // sets initial position of object in the world
    world_x = (map_w - player_rect.w) / 2;
    world_y = (map_h - player_rect.h) / 2;

    player->running = 1;

    PlayerSetStarters(player);

    int running = 1;
    Uint32 last_time = SDL_GetTicks();
    int frame_count = 0;
    Uint32 last_frame_time = SDL_GetTicks();
    int fps = 0;
    char fps_str[32] = "0";

    SDL_Rect fps_rect = {5, 5, 0, 0};
    SDL_Rect fps_box = {.x = 0, .y = 0, .w = 0, .h = 0};

    SDL_Color fps_color = {.r = 0, .g = 255, .b = 0, .a = 255};
    SDL_Surface* fps_surf = TTF_RenderText_Solid(game_font, fps_str, fps_color);
    SDL_Texture* fps_text = SDL_CreateTextureFromSurface(rend, fps_surf);
    SDL_FreeSurface(fps_surf);
    SDL_QueryTexture(fps_text, NULL, NULL, &fps_rect.w, &fps_rect.h);
    fps_box.w = fps_rect.w + 10;
    fps_box.h = fps_rect.h + 10;

    while (running) {
        Uint32 current_frame_time = SDL_GetTicks();
        Uint32 dt = current_frame_time - last_frame_time;
        last_frame_time = current_frame_time;

        SDL_Event event;
        // Events management
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                switch(player->game_state){
                    case STATE_EXPLORING:
                    // keyboard API for key pressed
                        switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_UP:
                            if(TrainerIsCollingWithPlayer(player) == NORTH) break;
                            player->facing_direction = NORTH;
                            world_y -= CHARACTER_SPEED / 30;
                            player->y_pos = world_y + (player_rect.h / 2);
                            if(!TrainerCheckAggro(player)) TrySpawnMonster(player);
                            break;
                        case SDL_SCANCODE_LEFT:
                            if(TrainerIsCollingWithPlayer(player) == WEST) break;
                            player->facing_direction = WEST;
                            world_x -= CHARACTER_SPEED / 30;
                            player->x_pos = world_x + (player_rect.w / 2);
                            if(!TrainerCheckAggro(player)) TrySpawnMonster(player);
                            break;
                        case SDL_SCANCODE_DOWN:
                            if(TrainerIsCollingWithPlayer(player) == SOUTH) break;
                            player->facing_direction = SOUTH;
                            world_y += CHARACTER_SPEED / 30;
                            player->y_pos = world_y + (player_rect.h / 2);
                            if(!TrainerCheckAggro(player)) TrySpawnMonster(player);
                            break;
                        case SDL_SCANCODE_RIGHT:
                            if(TrainerIsCollingWithPlayer(player) == EAST) break;
                            player->facing_direction = EAST;
                            world_x += CHARACTER_SPEED / 30;
                            player->x_pos = world_x + (player_rect.w / 2);
                            if(!TrainerCheckAggro(player)) TrySpawnMonster(player);
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            running = 0;
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
                    // In this case the player's inputs should not matter
                    break;
                    case STATE_AGGRO:
                    break;
                }   
                
                
            }
        }

        SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

        // Clamp player to map boundaries
        if (world_x < 0) world_x = 0;
        if (world_x > map_w - player_rect.w) world_x = map_w - player_rect.w;
        if (world_y < 0) world_y = 0;
        if (world_y > map_h - player_rect.h) world_y = map_h - player_rect.h;

        // Calculate camera offset to center the player
        int offset_x = (screen_w / 2) - (player_rect.w / 2) - world_x;
        int offset_y = (screen_h / 2) - (player_rect.h / 2) - world_y;

        // Set boundaries for the camera
        if (offset_x > 0) offset_x = 0;
        if (offset_x < screen_w - map_w) offset_x = screen_w - map_w;
        if (offset_y > 0) offset_y = 0;
        if (offset_y < screen_h - map_h) offset_y = screen_h - map_h;

        // Calculate player screen position based on world position and camera offset
        player_rect.x = world_x + offset_x;
        player_rect.y = world_y + offset_y;

        // Set player's position to the center of it's sprite
        player->x_pos = world_x + (player_rect.w / 2);
        player->y_pos = world_y + (player_rect.h / 2);

        SDL_RenderClear(rend);

        if(player->game_state == STATE_EXPLORING || player->game_state == STATE_AGGRO){
            SDL_Rect map_src = {-offset_x, -offset_y, screen_w, screen_h};
            SDL_RenderCopy(rend, map_tex, &map_src, NULL);

            SDL_RenderCopy(rend, player_texture, NULL, &player_rect);
            
            TrainerDraw(offset_x, offset_y);

            if(player->game_state == STATE_AGGRO){
                if(player->aggro_trainer){
                    TrainerRenderNotifBox(player->aggro_trainer, offset_x, offset_y, dt);
                    TrainerUpdateAggro(player, dt);
                }
                if(player->aggro_monster){
                    PlayerRenderNotifBox(player, offset_x, offset_y, dt);
                    MonsterUpdateAggro(player, dt);
                }
            }
        }
        else if(player->game_state == STATE_IN_MENU || player->game_state == STATE_LOCKED){
            if(player->current_menu) player->current_menu->draw(dt);
        }

        frame_count++;
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_time >= 1000) {
            fps = frame_count;
            frame_count = 0;
            last_time = current_time;
            sprintf(fps_str, "%d", fps);

            SDL_DestroyTexture(fps_text);
            fps_surf = TTF_RenderText_Solid(game_font, fps_str, fps_color);
            fps_text = SDL_CreateTextureFromSurface(rend, fps_surf);
            SDL_FreeSurface(fps_surf);
            SDL_QueryTexture(fps_text, NULL, NULL, &fps_rect.w, &fps_rect.h);
            fps_box.w = fps_rect.w + 10;
            fps_box.h = fps_rect.h + 10;
        }

        SDL_RenderFillRect(rend, &fps_box);

        SDL_RenderCopy(rend, fps_text, NULL, &fps_rect);

        // triggers the double buffers
        // for multiple rendering
        SDL_RenderPresent(rend);

        // calculates to 60 fps
        //SDL_Delay(1000 / 30);
    }

    player->running = 0;
    SDL_DestroyTexture(fps_text);

    BattleQuit();
    PlayerDestroy(player);

    SDL_DestroyTexture(map_tex);
    SDL_DestroyTexture(player_texture);
    SDL_DestroyRenderer(rend);
    TTF_CloseFont(game_font);
    SDL_DestroyWindow(win);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}