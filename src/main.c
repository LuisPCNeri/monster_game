#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_timer.h>

#include "map.h"

#define WINDOW_TITLE "WINDOW"
#define CHARACTER_SPEED 300

int main(void)
{
    // Offset variables
    int offset_x = 0, offset_y = 0;

    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win = SDL_CreateWindow("GAME", // creates a window
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1000, 1000, SDL_WINDOW_FULLSCREEN_DESKTOP);

    // triggers the program that controls
    // your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;

    // creates a renderer to render our images
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    
    // Create the map texture
    SDL_Texture* map_tex = create_map(rend);

    // Get the actual size of the generated map for boundary checks
    int map_w, map_h;
    SDL_QueryTexture(map_tex, NULL, NULL, &map_w, &map_h);

    // creates a surface to load an image into the main memory
    SDL_Surface* surface;

    // please provide a path for your image
    surface = IMG_Load("test.jpeg");
    if (!surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        return 1;
    }

    // loads image to our graphics hardware memory.
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surface);

    // clears main-memory
    SDL_FreeSurface(surface);

    // let us control our image position
    // so that we can move it with our keyboard.
    SDL_Rect player_rect = {0, 0, 0, 0};

    // connects our texture with player_rect to control position
    SDL_QueryTexture(tex, NULL, NULL, &player_rect.w, &player_rect.h);

    // adjust height and width of our image box.
    player_rect.w /= 6;
    player_rect.h /= 6;

    // Get screen dimensions safely from renderer
    int screen_w, screen_h;
    SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

    // sets initial x-position of object
    player_rect.x = (screen_w - player_rect.w) / 2;

    // sets initial y-position of object
    player_rect.y = (screen_h - player_rect.h) / 2;

    while (1) {
        SDL_Event event;

        // Events management
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

            case SDL_KEYDOWN:
                // keyboard API for key pressed
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    offset_y += CHARACTER_SPEED / 30;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    offset_x += CHARACTER_SPEED / 30;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    offset_y -= CHARACTER_SPEED / 30;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    offset_x -= CHARACTER_SPEED / 30;
                    break;
                case SDL_SCANCODE_ESCAPE:
                    // Exit the program
                    SDL_Quit();
                    return EXIT_SUCCESS;
                    break;
                default:
                    break;
                }
            }
        }

        // Update screen dimensions in case of window resize or resolution change
        SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

        // Keep the player centered on the screen
        player_rect.x = (screen_w - player_rect.w) / 2;
        player_rect.y = (screen_h - player_rect.h) / 2;

        // Set boundaries for the camera
        if (offset_x > 0) offset_x = 0;
        if (offset_x < screen_w - map_w) offset_x = screen_w - map_w;
        if (offset_y > 0) offset_y = 0;
        if (offset_y < screen_h - map_h) offset_y = screen_h - map_h;

        // clears the screen
        SDL_RenderClear(rend);

        // Create new rectangle with same w and h as map
        // But starts at the offset points
        SDL_Rect map_dest = {offset_x, offset_y, map_w, map_h};
        // Copy the map's texture to the new map variable
        SDL_RenderCopy(rend, map_tex, NULL, &map_dest);
        SDL_RenderCopy(rend, tex, NULL, &player_rect);

        // triggers the double buffers
        // for multiple rendering
        SDL_RenderPresent(rend);

        // calculates to 60 fps
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyTexture(map_tex);
    // destroy texture
    SDL_DestroyTexture(tex);

    // destroy renderer
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();

    return 0;
}