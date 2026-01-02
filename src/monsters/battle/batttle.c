#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "monsters/monster.h"
#include "player/player.h"
#include "battle.h"

// Renderer created in main
extern SDL_Renderer* rend;
extern TTF_Font* game_font;

static monster_t* enemy_mon = NULL;

typedef enum MenuItems{
    ATTACK,
    SWITCH,
    INVENTORY,
    RUN
} MenuItems;

void BattleInit(player_t* player, monster_t* enemy_monster){
    printf("BATTLE STARTING\n");

    // Set player's game state to battle
    player->game_state = STATE_BATTLE;

    // Just to hep on BattleDraw function to prevent it running when it is not supposed to
    enemy_mon = enemy_monster;
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

void BattleDraw(void){
    if (!enemy_mon) return;

    // Container for the text (PLACEHOLDER)
    // SWITCH BTN
    SDL_Rect switch_btn = { 50, 950, 400, 100 };
    // ATTACK
    SDL_Rect attack_btn = { 50, 800, 400, 100};
    // INVENTORY BTN
    SDL_Rect inventory_btn = {500, 800, 400, 100};
    // RUN BTN
    SDL_Rect run_btn = {500, 950, 400, 100};

    // Render all of the button's textures (mostly text texture)
    BattleRenderMenuItem("SWITCH", &switch_btn);
    BattleRenderMenuItem("ATTACK", &attack_btn);
    BattleRenderMenuItem("INVENTORY", &inventory_btn);
    BattleRenderMenuItem("RUN", &run_btn);

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