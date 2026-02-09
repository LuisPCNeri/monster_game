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

static move_t* enemy_last_move = NULL;

#define NOTCH_SIZE 16

void TrainerBattleDraw(Uint32 dt){
    BattleDraw(dt);

    // This next code block just draws the symbols indicating the trainer's party
    // as in a symbol for a slot with a monster, a greyed ou slot for a dead monster and an empty slot for well an empty space
    SDL_SetRenderDrawColor(rend, 82, 79, 79, 255);
    SDL_RenderDrawLine(rend, 1450, 40, 1850, 40);

    static SDL_Texture* empty_notch = NULL;
    static SDL_Texture* full_notch = NULL;
    static SDL_Texture* dead_notch = NULL;

    if(!empty_notch){
        SDL_Surface* s = IMG_Load("resources/battle_misc/empty_notch.png");
        empty_notch = SDL_CreateTextureFromSurface(rend, s);
        SDL_FreeSurface(s);
    }
    if(!full_notch){
        SDL_Surface* s = IMG_Load("resources/battle_misc/full_notch.png");
        full_notch = SDL_CreateTextureFromSurface(rend, s);
        SDL_FreeSurface(s);
    }
    if(!dead_notch){
        SDL_Surface* s = IMG_Load("resources/battle_misc/dead_notch.png");
        dead_notch = SDL_CreateTextureFromSurface(rend, s);
        SDL_FreeSurface(s);
    }

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
}

static SDL_Texture* intro_msg_tex = NULL;
static SDL_Texture* intro_name_tex = NULL;
static SDL_Texture* intro_sprite_tex = NULL;

void TrainerBattleInitMessageDraw(){
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, &player->current_menu->menu_items[0]);

    // Message from the trainer when starting the battle
    if(intro_msg_tex){
        int w, h;
        SDL_QueryTexture(intro_msg_tex, NULL, NULL, &w, &h);
        SDL_Rect text_rect = {.x = 50 + 20, .y = player->current_menu->menu_items[0].y + 20, .w = w, .h = h};
        SDL_RenderCopy(rend, intro_msg_tex, NULL, &text_rect);
    }

    // This just renders the trainer's name to the box where usually the enemy monster's info is
    SDL_Rect trainer_info_rect = {.x = 1450, .y = 50, .w = 400, .h = 100};
    SDL_RenderDrawRect(rend, &trainer_info_rect);

    if(intro_name_tex){
        int w, h;
        SDL_QueryTexture(intro_name_tex, NULL, NULL, &w, &h);
        SDL_Rect trainer_name_rect = {
            .x = trainer_info_rect.x + (trainer_info_rect.w - w) / 2,
            .y = trainer_info_rect.y + (trainer_info_rect.h - h) / 2,
            .w = w, .h = h
        };
        SDL_RenderCopy(rend, intro_name_tex, NULL, &trainer_name_rect);
    }

    // Finally this renders the trainer's sprite to the screen
    // TODO : Change the sprite to a full body more detailed sprite
    if(intro_sprite_tex){
        SDL_Rect src_rect = {.x = 0,    .y = 0,   .w = 256, .h = 256};
        SDL_Rect dst_rect = {.x = 1500, .y = 100, .w = 300, .h = 300};
        SDL_RenderCopy(rend, intro_sprite_tex, &src_rect, &dst_rect);
    }

    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}

void HandleEnterDownInitMessage(){
    if(intro_msg_tex){ 
        SDL_DestroyTexture(intro_msg_tex);    
        intro_msg_tex = NULL; 
    }
    if(intro_name_tex){ 
        SDL_DestroyTexture(intro_name_tex);   
        intro_name_tex = NULL; 
    }
    if(intro_sprite_tex){ 
        SDL_DestroyTexture(intro_sprite_tex); 
        intro_sprite_tex = NULL; 
    }

    menu_t* intro_menu = player->current_menu;
    BattleInit(player, &trainer->party[0], trainer);
    if(intro_menu) MenuDestroy(intro_menu);
    
    player->current_menu->draw = TrainerBattleDraw;
}

void TrainerBattleInit(player_t* active_player, trainer_t* active_trainer){
    printf("TRAINER BATTLE STARTING\n");

    player = active_player;
    trainer = active_trainer;

    if(!info_font) info_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", 16);

    int w, h;
    SDL_GetRendererOutputSize(rend, &w, &h);

    SDL_Color white = {255, 255, 255, 255};
    
    SDL_Surface* s = TTF_RenderText_Solid(game_font, trainer->intro_msg, white);
    intro_msg_tex = SDL_CreateTextureFromSurface(rend, s);
    SDL_FreeSurface(s);

    s = TTF_RenderText_Solid(game_font, trainer->name, white);
    intro_name_tex = SDL_CreateTextureFromSurface(rend, s);
    SDL_FreeSurface(s);

    s = IMG_Load(trainer->sprite_path);
    intro_sprite_tex = SDL_CreateTextureFromSurface(rend, s);
    SDL_FreeSurface(s);

    menu_t* m = MenuCreate(1, 1, 1, TrainerBattleInitMessageDraw, HandleEnterDownInitMessage);
    m->menu_items[0]. x = 50;
    m->menu_items[0]. y = h - 450;
    m->menu_items[0]. w = w - 100;
    m->menu_items[0]. h = 400;

    player->current_menu = m;
    player->game_state = STATE_IN_MENU;

    printf("TRAINER BATTLE STARTED\n");
}

move_t* TrainerBattleChooseMove(monster_t* player_monster, monster_t* enemy){
    
    move_t* highest_dmg = NULL;
    for(unsigned int i = 0; i < 4; i++){
        if(enemy->usable_moves[i].id == -1) continue;
        if(!highest_dmg) highest_dmg = &enemy->usable_moves[i];
        
        // Check if last move was inneficient
        if(enemy_last_move && enemy_last_move->id == enemy->usable_moves[i].id){
            float type_mult = MonsterGetTypeEffectiveness(enemy->usable_moves[i].attack_type, player_monster->type_1);

            if(player_monster->type_2 != NONE_TYPE){
                if(MonsterGetTypeEffectiveness(enemy->usable_moves[i].attack_type, player_monster->type_2) > type_mult)
                    type_mult = MonsterGetTypeEffectiveness(enemy->usable_moves[i].attack_type, player_monster->type_2);
            }
            
            // Do not let this move be used next
            if(type_mult < 1.0f) continue;
        }

        if(enemy->usable_moves[i].damage > highest_dmg->damage) 
            highest_dmg = &enemy->usable_moves[i];
    }
    
    return highest_dmg;
}