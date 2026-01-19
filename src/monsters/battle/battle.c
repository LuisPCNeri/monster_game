#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "monsters/monster.h"
#include "player/player.h"
#include "menus/menu.h"
#include "battle.h"

#define TURN_LENGTH 1000

// Renderer created in main
extern SDL_Renderer* rend;
extern TTF_Font* game_font;
static TTF_Font* info_font = NULL;

static monster_t* enemy_mon = NULL;
static SDL_Texture* enemy_mon_tex = NULL;

static menu_t* battle_menu = NULL;

static player_t* active_player = NULL;
static SDL_Texture* player_mon_tex = NULL;

static int turn_stage = 0;
static monster_t* first_attacker = NULL;
static monster_t* second_attacker = NULL;

static move_t* player_move_ptr = NULL;
static move_t* fst_atck_move = NULL;
static move_t* scnd_atck_move = NULL;

typedef enum BattleState{
    MAIN_MENU,
    MOVES_MENU,
    EXECUTING_TURN,
    INV_OPEN,
    MESSAGE_DISPLAYED
} BattleState;

static BattleState battle_state = MAIN_MENU;

void BattleInit(player_t* player, monster_t* enemy_monster){
    printf("BATTLE STARTING\n");
    active_player = player;
    battle_state = MAIN_MENU;

    if (battle_menu) MenuDestroy(battle_menu);
    if (enemy_mon_tex) {
        SDL_DestroyTexture(enemy_mon_tex);
        enemy_mon_tex = NULL;
    }
    battle_menu = MenuCreate(4, 1, 1, &BattleDraw, &BattleMenuHandleSelect);
    battle_menu->back = BattleMenuBack;

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

    // Just to hep on BattleDraw function to prevent it running when it is not supposed to
    enemy_mon = enemy_monster;

    // Set player's active monster to the first one in the party
    player->active_mon_index = 0;

    // Set player's game state to battle LAST to prevent race conditions
    player->selected_menu_itm = ATTACK;
    player->current_menu = battle_menu;
    player->game_state = STATE_IN_MENU;

    MonsterPrint(active_player->monster_party[active_player->active_mon_index]);
}

// Checks if battle is over (enemy monster is dead)
// If so calls other functions such as increase exp
// Returns >1 if battle is over and 0 if not
static int BattleCheckIsOver(){
    if(enemy_mon->current_hp <= 0){
        MonsterAddExp(active_player->monster_party[active_player->active_mon_index], enemy_mon);
        printf("Battle Won! Current Exp: %d/%d\n", 
            active_player->monster_party[active_player->active_mon_index]->current_exp,
            active_player->monster_party[active_player->active_mon_index]->exp_to_next_level);
        return 1;
    }
    else if(active_player->monster_party[active_player->active_mon_index]->current_hp <= 0){
        return 1;
    }

    return 0;
}

// Renders the button's text and a rect for them
// TODO : Change from the draw rect to a custom texture
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

    // TODO : Indicate player victory with a rect (in the middle of the screen showing the exp gained or something and if monster leveled up (WITH SOUNDS *GASP*)
}

static void BattleRenderInfo(const char* btn_text, SDL_Rect* dst_rect, int x_offset, int y_offset){
    if(!info_font) return;

    // Set color to render the rects
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, dst_rect);

    // Set the text color
    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* text_surface = TTF_RenderText_Solid(info_font, btn_text, text_color);
    if(!text_surface) return;
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

    SDL_DestroyTexture(text_texture);
}

void BattleDraw(){
    if (!enemy_mon || !battle_menu || !active_player) return;

    if(!info_font) info_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", 24);

    // Safety check for player monster
    if(!active_player->monster_party[active_player->active_mon_index]) return;

    // Logic to decide which menu to render
    if(battle_state == MAIN_MENU || battle_state == INV_OPEN){
        // Render normal menu items
        BattleRenderMenuItem("SWITCH", &battle_menu->menu_items[SWITCH]);
        BattleRenderMenuItem("ATTACK", &battle_menu->menu_items[ATTACK]);
        BattleRenderMenuItem("INVENTORY", &battle_menu->menu_items[INVENTORY]);
        BattleRenderMenuItem("RUN", &battle_menu->menu_items[RUN]);
    }
    else if(battle_state == MOVES_MENU){
        monster_t* active_mon = active_player->monster_party[active_player->active_mon_index];
        for(int i = 0; i < 4; i++){
            if(active_mon->usable_moves[i].id != -1){
                BattleRenderMenuItem(active_mon->usable_moves[i].move_name, &battle_menu->menu_items[i]);

                char move_uses[12];
                sprintf(move_uses, "%d/%d", active_mon->usable_moves[i].available_uses,
                    active_mon->usable_moves[i].max_uses);
                BattleRenderInfo(move_uses, &battle_menu->menu_items[i], 
                    battle_menu->menu_items[i].w - 85, battle_menu->menu_items[i].h - 30);

                char move_power[6];
                sprintf(move_power, "%dP", active_mon->usable_moves[i].damage);
                BattleRenderInfo(move_power, &battle_menu->menu_items[i], 
                    20, battle_menu->menu_items[i].h - 30);
            }
            else{
                BattleRenderMenuItem("-", &battle_menu->menu_items[i]);
            }
        }
    } else if(battle_state == EXECUTING_TURN){
        monster_t* player_mon = active_player->monster_party[active_player->active_mon_index];
        if(turn_stage == 0){
            // Execute First Attack
            monster_t* target = (first_attacker == player_mon) ? enemy_mon : player_mon;
            MonsterUseMoveOn(first_attacker, fst_atck_move, target);

            turn_stage = 1;
        }
        else if(turn_stage == 1){
            // Wait
            SDL_Rect status_rect = {50, 850, 800, 200};

            // Check if second attacker has fainted to display it
            if(second_attacker->current_hp <= 0){
                char info[269];
                sprintf(info, "%s has fainted!", second_attacker->monster_name);
                BattleRenderInfo(info, &status_rect, 20, 20);
            }else{
                // Print normal attack info
                char atck_info[520];
                snprintf(atck_info, 520, "%s USED %s", first_attacker->monster_name, fst_atck_move->move_name);
                BattleRenderInfo(atck_info, &status_rect, 20, 20);
            }
            
            // Render "Press Enter" indicator
            BattleRenderInfo("->", &status_rect, 750, 150);
        }
        else if(turn_stage == 2){
            // Execute Second Attack
            monster_t* target = (second_attacker == player_mon) ? enemy_mon : player_mon;
            MonsterUseMoveOn(second_attacker, scnd_atck_move, target);

            turn_stage = 3;
        }
        else if(turn_stage == 3){
            // Wait
            SDL_Rect status_rect = {50, 850, 800, 200};

            // Check if first attacker has fainted to display it
            if(first_attacker->current_hp <= 0){
                char info[269];
                sprintf(info, "%s has fainted!", first_attacker->monster_name);
                BattleRenderInfo(info, &status_rect, 20, 20);
            }else{
                // Print normal attack info
                char atck_info[520];
                snprintf(atck_info, 520, "%s USED %s", second_attacker->monster_name, scnd_atck_move->move_name);
                BattleRenderInfo(atck_info, &status_rect, 20, 20);
            }

            // Render "Press Enter" indicator
            BattleRenderInfo("->", &status_rect, 750, 150);
        }
    }
    else if(battle_state == MESSAGE_DISPLAYED){
        SDL_Rect status_rect = {50, 850, 800, 200};

        char message[520];
        sprintf(message,"You caught a(n) %s!", enemy_mon->monster_name);
        BattleRenderInfo(message, &status_rect, 20, 20);

        // Render "Press Enter" indicator
        BattleRenderInfo("->", &status_rect, 750, 150);
    }

    // Enemy moster health bar, name and lvl rect
    SDL_Rect enemy_rect = {1450, 50, 400, 100};
    SDL_Rect player_rect = {50, 550, 400, 100};
    SDL_Rect enemy_mon_sprite = {1500, 150, 300, 300};
    SDL_Rect player_mon_sprite = {50, 250, 300, 300};

    if (!enemy_mon_tex) {
        SDL_Surface* enemy_mon_surf = IMG_Load(enemy_mon->sprite_path);
        enemy_mon_tex = SDL_CreateTextureFromSurface(rend, enemy_mon_surf);
        SDL_FreeSurface(enemy_mon_surf);
    }
    if (!player_mon_tex) {
        char tex_name[512];
        sprintf(tex_name, "resources/monster_sprites/PLAYER_%s.png",
            active_player->monster_party[active_player->active_mon_index]->monster_name);

        SDL_Surface* player_mon_surf = IMG_Load(tex_name);
        player_mon_tex = SDL_CreateTextureFromSurface(rend, player_mon_surf);
        SDL_FreeSurface(player_mon_surf);
    }
    SDL_RenderCopy(rend, player_mon_tex, NULL, &player_mon_sprite);
    SDL_RenderCopy(rend, enemy_mon_tex, NULL, &enemy_mon_sprite);

    BattleRenderInfo(enemy_mon->monster_name, &enemy_rect, 20, 20);
    BattleRenderInfo(active_player->monster_party[active_player->active_mon_index]->monster_name, &player_rect, 20, 20);

    // Is freed whenever the function stops running
    char enemy_lvl_info[12];
    sprintf(enemy_lvl_info, "lvl: %d", enemy_mon->level);

    char player_lvl_info[12];
    sprintf(player_lvl_info, "lvl: %d", active_player->monster_party[active_player->active_mon_index]->level);
    // Render the enemy monster's level
    BattleRenderInfo(enemy_lvl_info, &enemy_rect, 300, 20);
    BattleRenderInfo(player_lvl_info, &player_rect, 300, 20);

    char enemy_hp_info[12];
    sprintf(enemy_hp_info, "%d/%d", enemy_mon->current_hp, enemy_mon->max_hp);

    char player_hp_info[12];
    sprintf(player_hp_info, "%d/%d", active_player->monster_party[active_player->active_mon_index]->current_hp,
        active_player->monster_party[active_player->active_mon_index]->max_hp);
    // Render the enemy monster's hp info
    BattleRenderInfo(enemy_hp_info, &enemy_rect, 300, 70);
    BattleRenderInfo(player_hp_info, &player_rect, 300, 70);

    if(battle_state == INV_OPEN) InventoryDraw(active_player->inv);
}

void BattleMenuHandleSelect(){
    if(battle_state == EXECUTING_TURN) {
        if (turn_stage == 1) {
             if(BattleCheckIsOver()){
                active_player->game_state = STATE_EXPLORING;
             } else {
                turn_stage = 2;
             }
        } else if (turn_stage == 3) {
             if(BattleCheckIsOver()){
                active_player->game_state = STATE_EXPLORING;
             } else {
                battle_state = MAIN_MENU;
                active_player->is_player_turn = 1;
             }
        }
        return;
    }

    if(battle_state == MAIN_MENU){
        BattleMenuButtons selected_btn = active_player->selected_menu_itm;
        if(selected_btn == ATTACK){
            battle_state = MOVES_MENU;
            active_player->selected_menu_itm = 0;
        }
        else if(selected_btn == RUN) BattleQuit();
        else if(selected_btn == INVENTORY){
            battle_state = INV_OPEN;
            active_player->current_menu = active_player->inv->menu;
            active_player->current_menu->select_routine = BattleMenuHandleSelect;
            active_player->current_menu->draw = BattleDraw;
            active_player->current_menu->back = BattleMenuBack;
        }
    }
    else if(battle_state == MOVES_MENU){
        // Make it so it is not the player's turn anymore
        active_player->is_player_turn = 0;

        monster_t* active_mon = active_player->monster_party[active_player->active_mon_index];
        player_move_ptr = &active_mon->usable_moves[active_player->selected_menu_itm];

        if(player_move_ptr->available_uses > 0){
            if(active_mon->speed >= enemy_mon->speed){
                first_attacker = active_mon;
                second_attacker = enemy_mon;

                fst_atck_move = player_move_ptr;
                scnd_atck_move = MonsterChooseEnemyAttack(enemy_mon);
            } else {
                first_attacker = enemy_mon;
                second_attacker = active_mon;

                fst_atck_move = MonsterChooseEnemyAttack(enemy_mon);
                scnd_atck_move = player_move_ptr;
            }
            battle_state = EXECUTING_TURN;
            turn_stage = 0;
        }
    }
    else if(battle_state == INV_OPEN){
        // IMPORTANT : Handle Inventory to test catching
        inventory_item_t* item = InventoryGetCurrent(active_player->inv);

        switch(item->type){
            case 0:
                if(item->count <= 0 || item->id == -1) return;
                printf("Used item with id: %d\n", item->id);
                
                InventoryRemoveItem(active_player->inv, item->item, 1);
                catch_device_t* device = (catch_device_t*) item->item;
                int has_caught = MonsterTryCatch(active_player, enemy_mon, device);
                if(has_caught){
                    // TODO : Display a caught message
                    fflush(stdout);
                    battle_state = MESSAGE_DISPLAYED;
                    return;
                }

                break;
            default:
                return;
        }
    }
    else if(battle_state == MESSAGE_DISPLAYED){
        active_player->game_state = STATE_EXPLORING;
    }
}

void BattleMenuBack(){
    if(battle_state == MAIN_MENU) return;
    if(battle_state == EXECUTING_TURN) return;
    
    MenuDeHighlightBox(&active_player->current_menu->menu_items[active_player->selected_menu_itm]);
    if(battle_state == MOVES_MENU){
        active_player->selected_menu_itm = ATTACK;
        MenuHighlightBox(&battle_menu->menu_items[ATTACK]);
    }
    if(battle_state == INV_OPEN) active_player->selected_menu_itm = INVENTORY;

    battle_state = MAIN_MENU;
    active_player->current_menu = battle_menu;
}

void BattleQuit(void){
    if (battle_menu){
        // Prevents double frees
        if(active_player) active_player->current_menu = NULL;
        
        MenuDestroy(battle_menu);
        battle_menu = NULL;
    }
    if (info_font){
        TTF_CloseFont(info_font);
        info_font = NULL;
    }

    if(player_mon_tex){
        SDL_DestroyTexture(player_mon_tex);
        player_mon_tex = NULL;
    }
    if (enemy_mon_tex){
        SDL_DestroyTexture(enemy_mon_tex);
        enemy_mon_tex = NULL;
    }

    if(active_player) active_player->game_state = STATE_EXPLORING;
}