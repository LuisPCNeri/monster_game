#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "monsters/monster.h"
#include "menus/menu.h"

#define STARTER_NUM 3

extern SDL_Renderer* rend;

static menu_t* starter_select_menu = NULL;
static monster_t* starter_mons[STARTER_NUM];

void PlayerStarterMenuDraw(menu_t* menu){
    if(!starter_select_menu) return;

    for(int i = 0; i < STARTER_NUM; i++){
        if(starter_mons[i]) MenuRenderItem(starter_mons[i]->monster_name, &starter_select_menu->menu_items[i]);
    }
}

// Then make menu SDL_Rects for each starter and have the player select one
// The selected one will be attributed to the player
// Right now sets the monster to the well uhhh the one available
monster_t* PlayerSetStarter(player_t* player){
    // Create the menu struct
    if(starter_select_menu) MenuDestroy(starter_select_menu);
    starter_select_menu = MenuCreate(3, 0, 1, &PlayerStarterMenuDraw);

    // Get the screen size
    int screen_w, screen_h;
    SDL_GetRendererOutputSize(rend, &screen_w, &screen_h);

    // btn size variables
    int btn_w = 400;
    int btn_h = 100;
    int gap = 50;
    // Calculate where buttons should start appearing at to be centered
    int total_width = (STARTER_NUM * btn_w) + ((STARTER_NUM - 1) * gap);
    // First button coordinates
    int start_x = (screen_w - total_width) / 2;
    int start_y = (screen_h - btn_h) / 2;

    for(int i = 0; i < STARTER_NUM; i++){
        SDL_Rect starter_rect = {start_x + i * (btn_w + gap), start_y, btn_w, btn_h};
        // The first button is selected so make it appear so
        if(i == 0) {
            starter_rect.x -= 10;
            starter_rect.y -= 10;
            starter_rect.w += 20;
            starter_rect.h += 20;
        }
        starter_select_menu->menu_items[i] = starter_rect;
    }

    starter_mons[0] = GetMonsterById(1);
    starter_mons[1] = GetMonsterById(2);
    starter_mons[2] = GetMonsterById(3);

    player->current_menu = starter_select_menu;
    player->selected_menu_itm = 0;
    player->game_state = STATE_IN_MENU;
}