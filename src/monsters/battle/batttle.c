#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "monsters/monster.h"
#include "player/player.h"
#include "menus/menu.h"
#include "battle.h"

// Renderer created in main
extern SDL_Renderer* rend;
extern TTF_Font* game_font;

static monster_t enemy_mon_data;
static monster_t* enemy_mon = NULL;
static menu_t* battle_menu = NULL;

void BattleInit(player_t* player, monster_t* enemy_monster){
    printf("BATTLE STARTING\n");

    if (battle_menu) MenuDestroy(battle_menu);
    menu_t* battle_menu = MenuCreate(4, 1, 1, &BattleDraw);

    // SWITCH BTN
    SDL_Rect switch_btn = { 50, 950, 400, 100 };
    // ATTACK
    SDL_Rect attack_btn = { 40, 790, 420, 120};
    // INVENTORY BTN
    SDL_Rect inventory_btn = {500, 800, 400, 100};
    // RUN BTN
    SDL_Rect run_btn = {500, 950, 400, 100};

    battle_menu->menu_items[ATTACK] = attack_btn;
    battle_menu->menu_items[SWITCH] = switch_btn;
    battle_menu->menu_items[INVENTORY] = inventory_btn;
    battle_menu->menu_items[RUN] = run_btn;

    // Set player's game state to battle
    player->game_state = STATE_IN_MENU;
    player->selected_menu_itm = ATTACK;
    player->current_menu = battle_menu;

    // Just to hep on BattleDraw function to prevent it running when it is not supposed to
    enemy_mon_data = *enemy_monster;
    enemy_mon = &enemy_mon_data;
}

// Renders the button's text and a rect for them
// TODO Change from the draw rect to a custom texture
static void BattleRenderMenuItem(const char* btn_text, SDL_Rect* dst_rect){
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

static void BattleRenderMonInfo(const char* btn_text, SDL_Rect* dst_rect, int x_offset, int y_offset){
    // New font size for the info
    TTF_Font* info_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", 24);

    // Set color to render the rects
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, dst_rect);

    // Set the text color
    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* text_surface = TTF_RenderText_Solid(info_font, btn_text, text_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surface);

    // Free the surface as it won't be used again
    SDL_FreeSurface(text_surface);

    // The text textures natural height and width
    int text_w, text_h;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);

    SDL_Rect text_rect;
    text_rect.x = dst_rect->x + x_offset;
    text_rect.y = dst_rect->y + y_offset;
    text_rect.w = text_w;
    text_rect.h = text_h;

    // Render the text
    SDL_RenderCopy(rend, text_texture, NULL, &text_rect);
    // Draw the text_rect outline
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderDrawRect(rend, &text_rect);

    TTF_CloseFont(info_font);
    SDL_DestroyTexture(text_texture);
}

void BattleDraw(menu_t* battle_menu){
    if (!enemy_mon || !battle_menu) return;

    // Container for the text (PLACEHOLDER)

    // Render all of the button's textures (mostly text texture)
    BattleRenderMenuItem("SWITCH", &battle_menu->menu_items[SWITCH]);
    BattleRenderMenuItem("ATTACK", &battle_menu->menu_items[ATTACK]);
    BattleRenderMenuItem("INVENTORY", &battle_menu->menu_items[INVENTORY]);
    BattleRenderMenuItem("RUN", &battle_menu->menu_items[RUN]);

    // Enemy moster health bar, name and lvl rect
    SDL_Rect enemy_rect = {1450, 50, 400, 100};
    BattleRenderMonInfo(enemy_mon->monster_name, &enemy_rect, 20, 20);

    // Is freed whenever the function stops running
    char lvl_info[12];
    sprintf(lvl_info, "lvl: %d", enemy_mon->level);
    // Render the enemy monster's level
    BattleRenderMonInfo(lvl_info, &enemy_rect, 300, 20);

    char hp_info[12];
    sprintf(hp_info, "%d/%d", enemy_mon->current_hp, enemy_mon->max_hp);
    // Render the enemy monster's hp info
    BattleRenderMonInfo(hp_info, &enemy_rect, 300, 70);
}

void BattleQuit(void){
    if (battle_menu) {
        MenuDestroy(battle_menu);
        battle_menu = NULL;
    }
}