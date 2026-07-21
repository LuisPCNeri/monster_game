#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "text_gradual_render_handler.h"

#define CHAR_TIME_INTERVAL_MS 70

void RenderTextByChar(const char* text, SDL_Rect* dst, TTF_Font* font, uint32_t delta_time, SDL_Renderer* renderer ) {

    static struct {
        const char* last_text;
        uint32_t chars_shown;
        uint32_t accum_ms;
        int8_t is_done;
    } state = {NULL, 0, 0, 0};
    
    if( state.last_text == NULL || strcmp(text, state.last_text) != 0 ) {

        state.last_text = text;
        state.chars_shown = 0;
        state.accum_ms = 0;
        state.is_done = 0;

    }

    if(!state.is_done) {
        state.accum_ms += delta_time;
        uint32_t len = strlen(state.last_text);

        while(state.accum_ms >= CHAR_TIME_INTERVAL_MS && state.chars_shown < len) {
            state.chars_shown++;
            state.accum_ms -= CHAR_TIME_INTERVAL_MS;
        }

        if(state.chars_shown >= len) {
            state.chars_shown = len;
            state.is_done = 1;
        }
    }

    char* buf = (char*) malloc(state.chars_shown + 1);
    if (!buf) return;

    memcpy(buf, text, state.chars_shown);
    buf[state.chars_shown] = '\0';

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface* surf = TTF_RenderText_Solid(font, buf, white);
    free(buf);

    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

    SDL_Rect actual_dst = {
        dst->x, dst->y,
        surf->w, surf->h
    };

    SDL_FreeSurface(surf);
    if (!tex) return;

    SDL_RenderCopy(renderer, tex, NULL, &actual_dst);
    SDL_DestroyTexture(tex);

}