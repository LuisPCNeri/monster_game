#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
#include "utils/glbl_asset_manager.h"
#include "handlers/player_input/player_input_handler.h"

#define WINDOW_TITLE "WINDOW"
#define FONT_SIZE 32

#define RENDER_SCALE (RENDER_TILE_SIZE / TILE_SIZE)

SDL_Renderer* rend;
TTF_Font* game_font;
int32_t screen_w;
int32_t screen_h;

glbl_asset_manager* asset_manager = NULL;

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) 
        printf("error initializing SDL: %s\n", SDL_GetError());
    if( TTF_Init() != 0 )            
        printf("Error initializing TTF: %s\n", TTF_GetError());
    if( MIX_INIT_MP3 != Mix_Init(MIX_INIT_MP3)) 
        printf("ERROR on MIXER: %s\n", Mix_GetError());

    SDL_Window* win = SDL_CreateWindow("GAME",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1280, 960, /*SDL_WINDOW_MAXIMIZED*/ SDL_WINDOW_FULLSCREEN_DESKTOP);

    // Frame rate is capped at the monitor's refresh rate because of SDL_RENDERER_PRESENTVSYNC
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC;
    rend = SDL_CreateRenderer(win, -1, render_flags);
    SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

    game_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", FONT_SIZE);
    Mix_OpenAudio(22050, AUDIO_S16SYS, 1, 1024);
    
    asset_manager = (glbl_asset_manager *) malloc(sizeof(glbl_asset_manager));

    MonstersInit();
    TrainersInit();
    player_t* player = PlayerInit();

    catch_device_t ball = { 1, 0, 1, "Ball", "" };
    union item_t ball_union = {.catch_device = &ball};
    InventoryAddItem(player->inv, ball_union, 15);

    restore_item_t potion = {4, 1, 10, "Potion", ""};
    union item_t pot_union = {.restore_item = &potion};
    InventoryAddItem(player->inv, pot_union, 5);

    chunked_map_t* cm = MapInit(rend);

    int32_t map_w = cm->w_chunks * CHUNK_SIZE * TILE_SIZE;
    int32_t map_h = cm->h_chunks * CHUNK_SIZE * TILE_SIZE;

    int32_t render_map_w = cm->w_chunks * CHUNK_SIZE * RENDER_TILE_SIZE;
    int32_t render_map_h = cm->h_chunks * CHUNK_SIZE * RENDER_TILE_SIZE;

    // Player's absolute position in the world
    int32_t world_x = 0, world_y = 0;

    SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

    // sets initial position of object in the world
    world_x = (map_w - player->sprite_rect.w) / 2;
    world_y = (map_h - player->sprite_rect.h) / 2;

    player->running = 1;

    PlayerSetStarters(player);

    int8_t running = 1;
    Uint32 last_time = SDL_GetTicks();
    int32_t frame_count = 0;
    Uint32 last_frame_time = SDL_GetTicks();

    /// --- Set up FPS Box ---

    int32_t fps = 0;
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

    /// --- Main game Loop ---

    while (running) {
        Uint32 current_frame_time = SDL_GetTicks();
        Uint32 dt = current_frame_time - last_frame_time;
        last_frame_time = current_frame_time;

        SDL_Event event;
        // Events management
        while (SDL_PollEvent(&event)) {
            HandlePlayerInput(player, cm, &running, &world_x, &world_y, event);
        }

        /// Logical units 32x32 tiles
        if (world_x < 0) world_x = 0;
        if (world_x > map_w - TILE_SIZE) world_x = map_w - TILE_SIZE;
        if (world_y < 0) world_y = 0;
        if (world_y > map_h - TILE_SIZE) world_y = map_h - TILE_SIZE;

        /// Render space 96x96 tiles
        int32_t render_x = world_x * RENDER_SCALE;
        int32_t render_y = world_y * RENDER_SCALE;

        int32_t offset_x = (screen_w / 2) - (player->sprite_rect.w / 2) - render_x;
        int32_t offset_y = (screen_h / 2) - (player->sprite_rect.h / 2) - render_y;

        if (offset_x > 0) offset_x = 0;
        if (offset_x < screen_w - render_map_w) offset_x = screen_w - render_map_w;
        if (offset_y > 0) offset_y = 0;
        if (offset_y < screen_h - render_map_h) offset_y = screen_h - render_map_h;

        player->sprite_rect.x = render_x + offset_x;
        player->sprite_rect.y = render_y + offset_y;

        player->x_pos = world_x + (PLAYER_SPRITE_SIZE / (RENDER_SCALE * 2));
        player->y_pos = world_y + (PLAYER_SPRITE_SIZE / (RENDER_SCALE * 2));

    
        MapUpdateStreaming(cm, player);
        SDL_RenderClear(rend);

        if(player->game_state == STATE_EXPLORING || player->game_state == STATE_AGGRO){

            SDL_Rect render_viewport = {
                -offset_x,
                -offset_y,
                screen_w, screen_h
            };

            MapRender(cm, rend, render_viewport);

            // Render player
            SDL_Rect window = PlayerGetSheetWindow(player);
            SDL_RenderCopy(rend, player->sprite_sheet, &window, &player->sprite_rect);
            
            /* DEBUG
            SDL_SetRenderDrawColor(rend, 0, 0, 255, 255);
            SDL_RenderDrawRect(rend, &player->sprite_rect);
            SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
            */

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
        SDL_RenderPresent(rend);

    }

    player->running = 0;
    SDL_DestroyTexture(fps_text);

    BattleQuit();
    PlayerDestroy(player);

    FreeMap(cm);

    SDL_DestroyRenderer(rend);
    TTF_CloseFont(game_font);
    SDL_DestroyWindow(win);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
