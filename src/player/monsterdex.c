#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "monsters/monster.h"
#include "monsters/battle/battle.h"
#include "monsterdex.h"
#include "menus/menu.h"
#include "utils/glbl_asset_manager.h"

extern SDL_Renderer* rend;
extern TTF_Font* game_font;
extern glbl_asset_manager* asset_manager;

static menu_t* menu = NULL;

// TODO : Create a menu object to hold the 4 move Rects
// Maybe store that menu in the player struct

menu_t* MonsterDexGetActiveMenu() {
    if(!menu){
        menu = MenuCreate(4, 1, 0, BattleDraw, BattleMenuHandleSelect);
        menu->back = BattleMenuBack;
    }

    return menu;
}

static void RenderMove(move_t* move, SDL_Rect main_box, SDL_Rect* move_box, int8_t index, int8_t is_selected){
    int32_t move_box_w = main_box.w - 40;
    // 25 margin up 25 margin down
    int32_t move_box_h = (main_box.h / 4) - 40;

    // just for safety purposes
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
 
    move_box->x = main_box.x + 20;
    move_box->y = main_box.y + 20 + (index * (main_box.h / 4));
    move_box->w = move_box_w;
    move_box->h = move_box_h;

    if(is_selected) {
        move_box->w += 20;
        move_box->h += 20;

        move_box->x -= 10;
        move_box->y -= 10;
    }

    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* text_surf = TTF_RenderText_Solid(game_font, move->move_name, text_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surf);
    SDL_FreeSurface(text_surf);

    char power[16];
    sprintf(power, "%dP", move->damage);
    SDL_Surface* move_power_surf = TTF_RenderText_Solid(game_font, power, text_color);
    SDL_Texture* move_power_text = SDL_CreateTextureFromSurface(rend, move_power_surf);
    SDL_FreeSurface(move_power_surf);

    char usages[16];
    sprintf(usages, "%d USES", move->max_uses);
    SDL_Surface* move_usages_surf = TTF_RenderText_Solid(game_font, usages, text_color);
    SDL_Texture* move_usages_text = SDL_CreateTextureFromSurface(rend, move_usages_surf);
    SDL_FreeSurface(move_usages_surf);

    int32_t text_w, text_h;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
    SDL_Rect text_rect = {
        move_box->x + (move_box_w - text_w) / 2,
        move_box->y + (move_box_h - text_h) / 2,
        text_w, text_h
    };

    int32_t w,h;
    SDL_QueryTexture(move_power_text, NULL, NULL, &w, &h);

    SDL_Rect power_rect = {
        .x = (move_box->x + move_box->w) - w - 10,
        .y = (move_box->y + move_box->h) - h - 10,
        .w = w, .h = h
    };

    SDL_QueryTexture(move_usages_text, NULL, NULL, &w, &h);
    SDL_Rect usages_rect = {
        .x = move_box->x + 10,
        .y = (move_box->y + move_box->h) - h - 10,
        .w = w, .h = h
    };

    if(is_selected) {
        text_rect.x += 10;
        text_rect.y += 10;
    }

    SDL_RenderCopy(rend, text_texture, NULL, &text_rect);
    SDL_RenderCopy(rend, move_power_text, NULL, &power_rect);
    SDL_RenderCopy(rend, move_usages_text, NULL, &usages_rect);

    SDL_DestroyTexture(text_texture);
    SDL_DestroyTexture(move_power_text);
    SDL_DestroyTexture(move_usages_text);

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
 
    /* YELLOW */
    if(is_selected) SDL_SetRenderDrawColor(rend, 255, 255, 0, 255);
    SDL_RenderDrawRect(rend, move_box);

    /* BLACK */
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
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
        menu = MenuCreate(4, 1, 0, BattleDraw, BattleMenuHandleSelect);
        menu->back = BattleMenuBack;
    }

    SDL_RenderFillRect(rend, &main_rect);
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, &main_rect);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);

    SDL_Rect monster_box_rect = {main_rect.x, main_rect.y, main_rect.w/2, main_rect.h};
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, &monster_box_rect);

    int32_t w = 1024, h = 1024;

    SDL_Rect monster_rect = {
        HORIZONTAL_MARGIN + monster_box_rect.w/2 - w/2,
        VERTICAL_MARGIN + monster_box_rect.h/2 - h/2 - 150, /* 150 to make the last third bigger*/
        w, h
    };

    SDL_Rect mon_src_rect = GetFromSpriteSheet(
        MON_FRONT_SPRITE_SIZE, SHEET_VERT_PADDING, 0, SHEET_COLS, monster->sprite_idx
    );

    if (SDL_RenderCopy(rend, asset_manager->mon_front_sheet, &mon_src_rect, &monster_rect) != 0) {
        printf("battle.c, BattleDraw: SDL_RenderCopy Error: %s\n", SDL_GetError());
    }

    SDL_Color fg = {255, 255, 255, 255};
    SDL_Surface* descripiton_surface = TTF_RenderText_Solid_Wrapped(game_font, monster->description, fg, monster_box_rect.w);
    SDL_Texture* description_texture = SDL_CreateTextureFromSurface(rend, descripiton_surface);
    SDL_FreeSurface(descripiton_surface);

    SDL_QueryTexture(description_texture, NULL, NULL, &w, &h);

    SDL_Rect description_box = {
        .x = HORIZONTAL_MARGIN + monster_box_rect.w/2 - w/2 + 20,
        .y = monster_rect.y + monster_rect.h - h,
        .w = w, .h = h
    };

    SDL_RenderCopy(rend, description_texture, NULL, &description_box);
    SDL_Rect moves_main_box = {
        main_rect.x + monster_box_rect.w, 
        main_rect.y, 
        monster_box_rect.w, 
        monster_box_rect.h
    };
    //SDL_RenderCopy(rend, monster_tex, NULL, &monster_rect);
    
    for(int8_t i = 0; i < USBALE_MOVES_AMOUNT; i++){
        RenderMove(&monster->moves[i], moves_main_box, &menu->menu_items[i], i, sel_index == i);
    }

    //RenderRectCorners(menu->menu_items[sel_index], 10, 15);

    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}
