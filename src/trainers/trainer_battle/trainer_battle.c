#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "menus/menu.h"
#include "player/player.h"
#include "trainers/trainer.h"
#include "monsters/battle/battle.h"

extern SDL_Renderer* rend;
extern TTF_Font* game_font;
static TTF_Font* info_font = NULL;

static player_t* player = NULL;
static trainer_t* trainer = NULL;

#define NOTCH_SIZE 16

void TrainerBattleDraw(){
    BattleDraw();

    // This next code block just draws the symbols indicating the trainer's party
    // as in a symbol for a slot with a monster, a greyed ou slot for a dead monster and an empty slot for well an empty space
    SDL_SetRenderDrawColor(rend, 82, 79, 79, 255);
    SDL_RenderDrawLine(rend, 1450, 40, 1850, 40);

    SDL_Surface* empty_notch_surf = IMG_Load("resources/battle_misc/empty_notch.png");
    SDL_Texture* empty_notch = SDL_CreateTextureFromSurface(rend, empty_notch_surf);
    SDL_FreeSurface(empty_notch_surf);

    SDL_Surface* full_notch_surf = IMG_Load("resources/battle_misc/full_notch.png");
    SDL_Texture* full_notch = SDL_CreateTextureFromSurface(rend, full_notch_surf);
    SDL_FreeSurface(full_notch_surf);

    SDL_Surface* dead_notch_surf = IMG_Load("resources/battle_misc/dead_notch.png");
    SDL_Texture* dead_notch = SDL_CreateTextureFromSurface(rend, dead_notch_surf);
    SDL_FreeSurface(dead_notch_surf);

    int w_empty, h_empty;
    int w_full, h_full;

    int start = 1450;
    int step = 400 / PARTY_SIZE;
    for(int i = 0; i < PARTY_SIZE; i++){
        if(trainer->party[i].id == -1){
            SDL_QueryTexture(empty_notch, NULL, NULL, &w_empty, &h_empty);

            SDL_Rect notch = {
                .x = start + step*i + step/2 - NOTCH_SIZE,
                .y = 40 - 8 - NOTCH_SIZE,
                .w = NOTCH_SIZE,
                .h = NOTCH_SIZE
            };

            SDL_RenderCopy(rend, empty_notch, NULL, &notch);
        }
        else{
            SDL_QueryTexture(full_notch, NULL, NULL, &w_full, &h_full);

            SDL_Rect notch = {
                .x = start + step*i + step/2 - NOTCH_SIZE,
                .y = 40 - 8 - NOTCH_SIZE,
                .w = NOTCH_SIZE,
                .h = NOTCH_SIZE
            };

            if(trainer->party[i].current_hp <= 0){
                SDL_RenderCopy(rend, dead_notch, NULL, &notch);
                continue;
            }
            SDL_RenderCopy(rend, full_notch, NULL, &notch);
        }
    }


    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_DestroyTexture(empty_notch);
    SDL_DestroyTexture(full_notch);
    SDL_DestroyTexture(dead_notch);
}

void TrainerBattleInitMessageDraw(){
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, &player->current_menu->menu_items[0]);

    // Message from the trainer when starting the battle
    SDL_Color white = {.r = 255, .g = 255, .b = 255, .a = 255};
    SDL_Surface* intro_text_surf = TTF_RenderText_Solid(game_font, trainer->intro_msg, white);
    SDL_Texture* intro_text = SDL_CreateTextureFromSurface(rend, intro_text_surf);
    SDL_FreeSurface(intro_text_surf);

    int w, h;
    SDL_QueryTexture(intro_text, NULL, NULL, &w, &h);
    SDL_Rect text_rect = {.x = 50 + 20, .y = player->current_menu->menu_items[0].y + 20, .w = w, .h = h};

    SDL_RenderCopy(rend, intro_text, NULL, &text_rect);
    SDL_DestroyTexture(intro_text);

    // This just renders the trainer's name to the box where usually the enemy monster's info is
    SDL_Rect trainer_info_rect = {.x = 1450, .y = 50, .w = 400, .h = 100};
    SDL_Surface* trainer_name_surf = TTF_RenderText_Solid(game_font, trainer->name, white);
    SDL_Texture* trainer_name = SDL_CreateTextureFromSurface(rend, trainer_name_surf);
    SDL_FreeSurface(trainer_name_surf);

    SDL_QueryTexture(trainer_name, NULL, NULL, &w, &h);
    SDL_Rect trainer_name_rect = {
        .x = trainer_info_rect.x + (trainer_info_rect.w - w) / 2,
        .y = trainer_info_rect.y + (trainer_info_rect.h - h) / 2,
        .w = w, .h = h
    };

    SDL_RenderCopy(rend, trainer_name, NULL, &trainer_name_rect);
    SDL_RenderDrawRect(rend, &trainer_info_rect);
    SDL_DestroyTexture(trainer_name);

    // Finally this renders the trainer's sprite to the screen
    // TODO : Change the sprite to a full body more detailed sprite
    SDL_Rect src_rect = {.x = 0,    .y = 0,   .w = 256, .h = 256};
    SDL_Rect dst_rect = {.x = 1500, .y = 100, .w = 300, .h = 300};

    SDL_Surface* trainer_surf = IMG_Load(trainer->sprite_path);
    SDL_Texture* trainer_tex = SDL_CreateTextureFromSurface(rend, trainer_surf);
    SDL_FreeSurface(trainer_surf);

    SDL_RenderCopy(rend, trainer_tex, &src_rect, &dst_rect);
    SDL_DestroyTexture(trainer_tex);

    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}

void HandleEnterDownInitMessage(){
    BattleInit(player, &trainer->party[0]);
    player->current_menu->draw = TrainerBattleDraw;
}

void TrainerBattleInit(player_t* active_player, trainer_t* active_trainer){
    player = active_player;
    trainer = active_trainer;

    if(!info_font) info_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", 16);

    int w, h;
    SDL_GetRendererOutputSize(rend, &w, &h);

    menu_t* m = MenuCreate(1, 0, 1, TrainerBattleInitMessageDraw, HandleEnterDownInitMessage);
    m->menu_items[0]. x = 50;
    m->menu_items[0]. y = h - 450;
    m->menu_items[0]. w = w - 100;
    m->menu_items[0]. h = 400;

    player->current_menu = m;
    player->game_state = STATE_IN_MENU;
}