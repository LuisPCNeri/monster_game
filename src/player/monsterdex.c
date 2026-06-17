#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "monsters/monster.h"
#include "monsters/battle/battle.h"
#include "monsterdex.h"
#include "menus/menu.h"

extern SDL_Renderer* rend;
extern TTF_Font* game_font;

static SDL_Texture* monster_tex = NULL;

static menu_t* menu = NULL;

// TODO : Create a menu object to hold the 4 move Rects
// Maybe store that menu in the player struct

static void RenderRectCorners(SDL_Rect rect, int32_t padding, int32_t seg_length){
    int32_t x = rect.x - padding;
    int32_t y = rect.y - padding;
    int32_t rX = rect.x + rect.w + padding;
    int32_t bY = rect.y + rect.h + padding;

    /// top left
    SDL_RenderDrawLine(rend, x, y, x + seg_length, y);
    SDL_RenderDrawLine(rend, x, y, x, y + seg_length);

    /// top right
    SDL_RenderDrawLine(rend, rX - seg_length, y, rX, y);
    SDL_RenderDrawLine(rend, rX, y, rX, y + seg_length);

    /// btm left
    SDL_RenderDrawLine(rend, x, bY, x + seg_length, bY);
    SDL_RenderDrawLine(rend, x, bY - seg_length, x, bY);
    
    /// Btm right
    SDL_RenderDrawLine(rend, rX - seg_length, bY, rX, bY);
    SDL_RenderDrawLine(rend, rX, bY - seg_length, rX, bY);
}

static void RenderMove(move_t* move, SDL_Rect main_box, SDL_Rect* move_box, int8_t index){
    int32_t move_box_w = main_box.w - 40;
    // 25 margin up 25 margin down
    int32_t move_box_h = (main_box.h / 4) - 40;

    // just for safety purposes
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    
    move_box->x = main_box.x + 20;
    move_box->y = main_box.y + 20 + (index * (main_box.h / 4));
    move_box->w = move_box_w;
    move_box->h = move_box_h;

    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* text_surf = TTF_RenderText_Solid(game_font, move->move_name, text_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surf);
    SDL_FreeSurface(text_surf);

    int32_t text_w, text_h;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
    SDL_Rect text_rect = {
        move_box->x + (move_box_w - text_w) / 2,
        move_box->y + (move_box_h - text_h) / 2,
        text_w, text_h
    };

    SDL_RenderCopy(rend, text_texture, NULL, &text_rect);
    SDL_DestroyTexture(text_texture);
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, move_box);
}

// IMPORTANT : Create the menu_t object for the level up menu, FIX the menu as it is all over the place
// IMPORTANT : Do the actual move learning logic (getting the index and switching one move for the other)

void DexDrawMonsterInfo(player_t* player ,monster_t* monster, int32_t screen_w, int32_t screen_h, int32_t offset_x){
    int8_t sel_index = player->selected_menu_itm;

    SDL_Rect main_rect = {
        HORIZONTAL_MARGIN + offset_x, 
        VERTICAL_MARGIN, 
        screen_w - HORIZONTAL_MARGIN * 2, 
        screen_h - VERTICAL_MARGIN * 2
    };

    if(!menu){
        menu = MenuCreate(4, 1, 0, BattleDraw, NULL);
        menu->back = BattleMenuBack;
    }

    SDL_RenderFillRect(rend, &main_rect);
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, &main_rect);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);

    SDL_Rect monster_box_rect = {main_rect.x, main_rect.y, main_rect.w/2, main_rect.h};
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, &monster_box_rect);

    /*if(!monster_tex){
        SDL_Surface* surf = IMG_Load(monster->sprite_path);
        monster_tex = SDL_CreateTextureFromSurface(rend, surf);
        SDL_FreeSurface(surf);
    }
    int32_t w,h;
    SDL_QueryTexture(monster_tex, NULL, NULL, &w, &h);

    SDL_Rect monster_rect = {
        HORIZONTAL_MARGIN + monster_box_rect.w/2 - w/2, 
        VERTICAL_MARGIN + monster_box_rect.h/2 - h/2, 
        w, h
    };*/
    SDL_Rect moves_main_box = {
        main_rect.x + monster_box_rect.w, 
        main_rect.y, 
        monster_box_rect.w, 
        monster_box_rect.h
    };
    //SDL_RenderCopy(rend, monster_tex, NULL, &monster_rect);
    
    for(int8_t i = 0; i < 4; i++){
        RenderMove(&monster->moves[i], moves_main_box, &menu->menu_items[i], i);
    }

    //RenderRectCorners(menu->menu_items[sel_index], 10, 15);

    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}