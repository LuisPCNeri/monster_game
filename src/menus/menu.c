#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "player/player.h"
#include "menu.h"

extern SDL_Renderer* rend;
extern TTF_Font* game_font;

menu_t* MenuCreate(int item_num, int has_rows, int has_columns, void* draw_func){
    // Make space for the menu
    menu_t* menu = (menu_t*) malloc(sizeof(menu_t));

    // Make space in the menu_items array for all items
    menu->menu_items = (SDL_Rect*) malloc(sizeof(SDL_Rect)*item_num);
    menu->has_rows = has_rows;
    menu->has_columns = has_columns;
    menu->items_amount = item_num;
    menu->draw = draw_func;

    return menu;
}

void MenuDestroy(menu_t* menu){
    if(!menu) return;
    free(menu->menu_items);
    free(menu);
}

// Renders the button's text and a rect for them
void MenuRenderItem(const char* btn_text, SDL_Rect* dst_rect){
    // Set color to render the rects
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, dst_rect);

    // Set the text color
    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* text_surface = TTF_RenderText_Solid(game_font, btn_text, text_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surface);

    // Free the surface as it won't be used again
    SDL_FreeSurface(text_surface);

    // The text textures natural height and width
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

void MenuItemKeyUp(player_t* player){
    menu_t* menu = player->current_menu;
    if(player->selected_menu_itm >= 2 && menu->has_rows){
        menu->menu_items[player->selected_menu_itm].w -= 20;
        menu->menu_items[player->selected_menu_itm].h -= 20;
        menu->menu_items[player->selected_menu_itm].x += 10;
        menu->menu_items[player->selected_menu_itm].y += 10;

        player->selected_menu_itm -= 2;

        menu->menu_items[player->selected_menu_itm].w += 20;
        menu->menu_items[player->selected_menu_itm].h += 20;
        menu->menu_items[player->selected_menu_itm].x -= 10;
        menu->menu_items[player->selected_menu_itm].y -= 10;
    }

    printf("SELECTED ITEM: %d\n", player->selected_menu_itm);
}

void MenuItemKeyDown(player_t* player){
    menu_t* menu = player->current_menu;
    if(player->selected_menu_itm <= 1 && menu->has_rows) {
        menu->menu_items[player->selected_menu_itm].w -= 20;
        menu->menu_items[player->selected_menu_itm].h -= 20;
        menu->menu_items[player->selected_menu_itm].x += 10;
        menu->menu_items[player->selected_menu_itm].y += 10;

        player->selected_menu_itm += 2;

        menu->menu_items[player->selected_menu_itm].w += 20;
        menu->menu_items[player->selected_menu_itm].h += 20;
        menu->menu_items[player->selected_menu_itm].x -= 10;
        menu->menu_items[player->selected_menu_itm].y -= 10;
    }

    printf("SELECTED ITEM: %d\n", player->selected_menu_itm);
}

void MenuItemKeyLeft(player_t* player){
    menu_t* menu = player->current_menu;
    if( menu->has_rows == 0 && player->selected_menu_itm > 0){
        menu->menu_items[player->selected_menu_itm].w -= 20;
        menu->menu_items[player->selected_menu_itm].h -= 20;
        menu->menu_items[player->selected_menu_itm].x += 10;
        menu->menu_items[player->selected_menu_itm].y += 10;

        player->selected_menu_itm--;

        menu->menu_items[player->selected_menu_itm].w += 20;
        menu->menu_items[player->selected_menu_itm].h += 20;
        menu->menu_items[player->selected_menu_itm].x -= 10;
        menu->menu_items[player->selected_menu_itm].y -= 10;

        printf("SELECTED ITEM: %d\n", player->selected_menu_itm);
        return;
    }

    if(player->selected_menu_itm % 2 != 0 && menu->has_rows) {
        menu->menu_items[player->selected_menu_itm].w -= 20;
        menu->menu_items[player->selected_menu_itm].h -= 20;
        menu->menu_items[player->selected_menu_itm].x += 10;
        menu->menu_items[player->selected_menu_itm].y += 10;

        player->selected_menu_itm--;

        menu->menu_items[player->selected_menu_itm].w += 20;
        menu->menu_items[player->selected_menu_itm].h += 20;
        menu->menu_items[player->selected_menu_itm].x -= 10;
        menu->menu_items[player->selected_menu_itm].y -= 10;
    }
}

void MenuItemKeyRight(player_t* player){
    menu_t* menu = player->current_menu;

    if( menu->has_rows == 0 && player->selected_menu_itm < menu->items_amount - 1){
        menu->menu_items[player->selected_menu_itm].w -= 20;
        menu->menu_items[player->selected_menu_itm].h -= 20;
        menu->menu_items[player->selected_menu_itm].x += 10;
        menu->menu_items[player->selected_menu_itm].y += 10;

        player->selected_menu_itm++;

        menu->menu_items[player->selected_menu_itm].w += 20;
        menu->menu_items[player->selected_menu_itm].h += 20;
        menu->menu_items[player->selected_menu_itm].x -= 10;
        menu->menu_items[player->selected_menu_itm].y -= 10;

        printf("SELECTED ITEM: %d\n", player->selected_menu_itm);
        return;
    }

    if(player->selected_menu_itm % 2 == 0 && menu->has_rows) {
        menu->menu_items[player->selected_menu_itm].w -= 20;
        menu->menu_items[player->selected_menu_itm].h -= 20;
        menu->menu_items[player->selected_menu_itm].x += 10;
        menu->menu_items[player->selected_menu_itm].y += 10;

        player->selected_menu_itm++;

        menu->menu_items[player->selected_menu_itm].w += 20;
        menu->menu_items[player->selected_menu_itm].h += 20;
        menu->menu_items[player->selected_menu_itm].x -= 10;
        menu->menu_items[player->selected_menu_itm].y -= 10;
        return;
    }
}

//TODO void MenuSelectCurrentItem(menu_t* menu)