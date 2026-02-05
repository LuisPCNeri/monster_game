#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "player/inventory.h"
#include "player/player.h"
#include "menu.h"

extern SDL_Renderer* rend;
extern TTF_Font* game_font;

menu_t* MenuCreate(int item_num, int has_rows, int has_columns, void* draw_func, void* select_func){
    menu_t* menu = (menu_t*) malloc(sizeof(menu_t));

    menu->menu_items = (SDL_Rect*) malloc(sizeof(SDL_Rect)*item_num);
    menu->has_rows = has_rows;
    menu->has_columns = has_columns;
    menu->items_amount = item_num;
    menu->draw = draw_func;
    menu->select_routine = select_func;
    menu->back = NULL;

    return menu;
}

void MenuDestroy(menu_t* menu){
    if(!menu) return;
    free(menu->menu_items);
    free(menu);
}

void MenuRenderItem(const char* btn_text, SDL_Rect* dst_rect){
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, dst_rect);

    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* text_surface = TTF_RenderText_Solid(game_font, btn_text, text_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surface);
    SDL_FreeSurface(text_surface);

    int text_w, text_h;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);

    SDL_Rect text_rect;
    text_rect.x = dst_rect->x + (dst_rect->w - text_w) / 2;
    text_rect.y = dst_rect->y + (dst_rect->h - text_h) / 2;
    text_rect.w = text_w;
    text_rect.h = text_h;

    // Render the text box
    SDL_RenderCopy(rend, text_texture, NULL, &text_rect);
    // Draw the text_rect outline
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderDrawRect(rend, &text_rect);

    SDL_DestroyTexture(text_texture);
}

// IMPORTANT : Add highlighting to the inventory items
void MenuHighlightBox(SDL_Rect* rect){
    rect->w += 20;
    rect->h += 20;
    rect->x -= 10;
    rect->y -= 10;
}

void MenuDeHighlightBox(SDL_Rect* rect){
    rect->w -= 20;
    rect->h -= 20;
    rect->x += 10;
    rect->y += 10;
}

void MenuItemKeyUp(player_t* player){
    menu_t* menu = player->current_menu;
    if(menu->items_amount <= 1) return;

    int stride = (menu->has_columns) ? 2 : 1;

    if(player->inv_isOpen){
        if(!player->inv->current->prev_item || player->inv->current->prev_item->id == -1) return;
        InventoryMoveBack(player->inv);
        return;
    }

    if(player->selected_menu_itm >= stride && menu->has_rows){
        MenuDeHighlightBox(&menu->menu_items[player->selected_menu_itm]);
        player->selected_menu_itm -= stride;
        MenuHighlightBox(&menu->menu_items[player->selected_menu_itm]);
    }
}

void MenuItemKeyDown(player_t* player){
    menu_t* menu = player->current_menu;
    if(menu->items_amount <= 1) return;

    int stride = (menu->has_columns) ? 2 : 1;

    if(player->inv_isOpen){
        if(!player->inv->current->next_item || player->inv->current->next_item->id == -1) return;
        InventoryMoveForward(player->inv);
        return;
    }

    if(player->selected_menu_itm + stride < menu->items_amount && menu->has_rows) {
        MenuDeHighlightBox(&menu->menu_items[player->selected_menu_itm]);
        player->selected_menu_itm += stride;
        MenuHighlightBox(&menu->menu_items[player->selected_menu_itm]);
    }
}

void MenuItemKeyLeft(player_t* player){
    menu_t* menu = player->current_menu;
    if(menu->items_amount <= 1) return;

    if( menu->has_rows == 0 && player->selected_menu_itm > 0){
        MenuDeHighlightBox(&menu->menu_items[player->selected_menu_itm]);
        player->selected_menu_itm--;
        MenuHighlightBox(&menu->menu_items[player->selected_menu_itm]);

        return;
    }

    if( (player->selected_menu_itm & 1) == 1 && menu->has_rows && menu->has_columns
            && player->selected_menu_itm > 0) {
        MenuDeHighlightBox(&menu->menu_items[player->selected_menu_itm]);
        player->selected_menu_itm--;
        MenuHighlightBox(&menu->menu_items[player->selected_menu_itm]);
    }
}

void MenuItemKeyRight(player_t* player){
    menu_t* menu = player->current_menu;
    if(menu->items_amount <= 1) return;

    if( menu->has_rows == 0 && player->selected_menu_itm < menu->items_amount - 1){
        MenuDeHighlightBox(&menu->menu_items[player->selected_menu_itm]);
        player->selected_menu_itm++;
        MenuHighlightBox(&menu->menu_items[player->selected_menu_itm]);

        return;
    }

    if( (player->selected_menu_itm & 1) == 0 && menu->has_rows && menu->has_columns
            && player->selected_menu_itm < menu->items_amount - 1) {
        MenuDeHighlightBox(&menu->menu_items[player->selected_menu_itm]);
        player->selected_menu_itm++;
        MenuHighlightBox(&menu->menu_items[player->selected_menu_itm]);
        
        return;
    }
}

void MenuSelectCurrentItem(player_t* player){
    // Wrapper function that just runs the current menu's select function
    player->current_menu->select_routine();
}