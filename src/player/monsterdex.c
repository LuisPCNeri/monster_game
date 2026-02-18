#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "monsters/monster.h"
#include "monsters/battle/battle.h"
#include "monsterdex.h"

extern SDL_Renderer* rend;
extern TTF_Font* game_font;

static SDL_Texture* monster_tex = NULL;

// TODO : Create a menu object to hold the 4 move Rects
// Maybe store that menu in the player struct

static void RenderMove(move_t* move, int main_box_x, int main_box_y, int main_box_h, int main_box_w){
    int move_box_w = main_box_w - 25;
    // 25 margin up 25 margin down
    int move_box_h = main_box_h/4 - 25;

    // just for safety purposes
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_Rect move_box = {main_box_x + 25, main_box_y + 25, move_box_w, move_box_h};

    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* text_surf = TTF_RenderText_Solid(game_font, move->move_name, text_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surf);
    SDL_FreeSurface(text_surf);

    int text_w, text_h;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
    SDL_Rect text_rect = {
        move_box.x + (move_box_w - text_w) / 2,
        move_box.y + (move_box_h - text_h) / 2,
        text_w, text_h
    };

    SDL_RenderCopy(rend, text_texture, NULL, &text_rect);
    SDL_DestroyTexture(text_texture);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderDrawRect(rend, &text_rect);
}

void DexDrawMonsterInfo(monster_t* monster, int screen_w, int screen_h, int offset_x){
    SDL_Rect main_rect = {HORIZONTAL_MARGIN + offset_x, VERTICAL_MARGIN, screen_w - HORIZONTAL_MARGIN*2, screen_h - VERTICAL_MARGIN*2};
    SDL_RenderFillRect(rend, &main_rect);

    SDL_Rect monster_box_rect = {main_rect.x, main_rect.y, main_rect.w/2, main_rect.h};
    SDL_RenderDrawRect(rend, &monster_box_rect);

    if(!monster_tex){
        SDL_Surface* surf = IMG_Load(monster->sprite_path);
        monster_tex = SDL_CreateTextureFromSurface(rend, surf);
        SDL_FreeSurface(surf);
    }
    int w,h;
    SDL_QueryTexture(monster_tex, NULL, NULL, &w, &h);

    SDL_Rect monster_rect = {
        HORIZONTAL_MARGIN + monster_box_rect.w/2 - w, 
        VERTICAL_MARGIN + monster_box_rect.h/2 - h + 400, 
        w, h
    };
    SDL_Rect moves_main_box = {
        main_rect.x + monster_box_rect.w, 
        main_rect.y, 
        monster_box_rect.w, 
        monster_box_rect.h
    };
    SDL_RenderCopy(rend, monster_tex, NULL, &monster_rect);
    
    for(int i = 0; i < 4; i++){
        RenderMove(&monster->usable_moves[i], moves_main_box.x, moves_main_box.y, moves_main_box.w, moves_main_box.h);
    }

}