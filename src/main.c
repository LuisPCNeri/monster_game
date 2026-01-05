#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

#include "map.h"
#include "player/player.h"
#include "menus/menu.h"
#include "monsters/monster.h"
#include "monsters/battle/battle.h"

#define WINDOW_TITLE "WINDOW"
#define CHARACTER_SPEED 300
#define FONT_SIZE 32

SDL_Renderer* rend;
TTF_Font* game_font;

int main(void)
{
    MonstersInit();
    player_t* player = (player_t*) malloc(sizeof(player_t));
    player->x_pos = 0;
    player->y_pos = 0;
    // Player's absolute position in the world
    int world_x = 0, world_y = 0;
    
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    if( TTF_Init() != 0 ) printf("Error initializing TTF: %s\n", TTF_GetError());
    SDL_Window* win = SDL_CreateWindow("GAME", // creates a window
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1000, 1000, SDL_WINDOW_FULLSCREEN_DESKTOP);

    // Set up TTF
    game_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", FONT_SIZE);

    // triggers the program that controls
    // your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    // creates a renderer to render our images
    rend = SDL_CreateRenderer(win, -1, render_flags);
    // Create the map texture
    SDL_Texture* map_tex = CreateGameMap(rend);

    // Get the actual size of the generated map for boundary checks
    int map_w, map_h;
    SDL_QueryTexture(map_tex, NULL, NULL, &map_w, &map_h);

    // creates a surface to load an image into the main memory
    SDL_Surface* surface;

    // please provide a path for your image
    surface = IMG_Load("./resources/test.jpeg");
    if (!surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        SDL_DestroyTexture(map_tex);
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(win);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // loads image to our graphics hardware memory.
    SDL_Texture* player_texture = SDL_CreateTextureFromSurface(rend, surface);
    // clears main-memory
    SDL_FreeSurface(surface);

    // let us control our image position
    // so that we can move it with our keyboard.
    SDL_Rect player_rect = {0, 0, 0, 0};
    // connects our texture with player_rect to control position
    SDL_QueryTexture(player_texture, NULL, NULL, &player_rect.w, &player_rect.h);

    // adjust height and width of our image box.
    player_rect.w /= 6;
    player_rect.h /= 6;

    // Get screen dimensions safely from renderer
    int screen_w, screen_h;
    SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

    // sets initial position of object in the world (center of map)
    world_x = (map_w - player_rect.w) / 2;
    world_y = (map_h - player_rect.h) / 2;

    // CREATEAS a Thread to keep trying to spawn monsters in the background
    player->running = 1;
    pthread_t spawn_thread;
    pthread_create(&spawn_thread, NULL, &TrySpawnMonster, player);

    player->game_state = STATE_EXPLORING;

    PlayerSetStarter(player);

    int running = 1;
    while (running) {
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
                            world_y -= CHARACTER_SPEED / 30;
                            break;
                        case SDL_SCANCODE_LEFT:
                            world_x -= CHARACTER_SPEED / 30;
                            break;
                        case SDL_SCANCODE_DOWN:
                            world_y += CHARACTER_SPEED / 30;
                            break;
                        case SDL_SCANCODE_RIGHT:
                            world_x += CHARACTER_SPEED / 30;
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            // Clean Up
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
                                player->game_state = STATE_EXPLORING;
                                break;
                            case SDL_SCANCODE_KP_ENTER:
                                //TODO MenuSelectCurrentItem logic
                                break;
                            default:
                                break;
                        }
                        break;
                }   
                
                
            }
        }

        // Update screen dimensions in case of window resize or resolution change
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

        player->x_pos = world_x;
        player->y_pos = world_y;

        // clears the screen
        SDL_RenderClear(rend);

        if(player->game_state == STATE_EXPLORING){
            // Create new rectangle with same w and h as map
            // But starts at the offset points
            SDL_Rect map_dest = {offset_x, offset_y, map_w, map_h};
            // Copy the map's texture to the new map variable
            SDL_RenderCopy(rend, map_tex, NULL, &map_dest);
            SDL_RenderCopy(rend, player_texture, NULL, &player_rect);
        }
        else if(player->game_state == STATE_IN_MENU){
            player->current_menu->draw(player->current_menu);
        }

        // triggers the double buffers
        // for multiple rendering
        SDL_RenderPresent(rend);

        // calculates to 60 fps
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyTexture(map_tex);
    // destroy texture
    SDL_DestroyTexture(player_texture);
    // destroy renderer
    SDL_DestroyRenderer(rend);
    // Destroy font
    TTF_CloseFont(game_font);
    // destroy window
    SDL_DestroyWindow(win);
    
    player->running = 0;
    pthread_join(spawn_thread, NULL);
    free(player);

    // close SDL
    BattleQuit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}