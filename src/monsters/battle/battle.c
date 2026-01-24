#include <stdio.h>
#include <math.h>

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

static SDL_Rect status_rect = {50, 850, 800, 200};
// Enemy moster health bar, name and lvl rect
static SDL_Rect enemy_rect = {1450, 50, 400, 100};
static SDL_Rect player_rect = {50, 550, 400, 100};
static SDL_Rect enemy_mon_sprite = {1500, 150, 300, 300};
static SDL_Rect player_mon_sprite = {50, 250, 300, 300};
static SDL_Rect player_hp_rect = {70, 602, 250, 10};
static SDL_Rect enemy_hp_rect = {1470, 102, 250, 10};

static monster_t* enemy_mon = NULL;
static SDL_Texture* enemy_mon_tex = NULL;

static menu_t* battle_menu = NULL;
static menu_t* switch_menu = NULL;

static player_t* active_player = NULL;
static SDL_Texture* player_mon_tex = NULL;

static int turn_stage = 0;
static monster_t* first_attacker = NULL;
static monster_t* second_attacker = NULL;

static move_t* player_move_ptr = NULL;
static move_t* fst_atck_move = NULL;
static move_t* scnd_atck_move = NULL;

static float player_displayed_hp = 0.0f;
static float enemy_displayed_hp = 0.0f;

static SDL_Rect switch_hp_bars[PARTY_SIZE];

static char message[520];

typedef enum BattleState{
    MAIN_MENU,
    MOVES_MENU,
    SWITCH_MENU,
    EXECUTING_TURN,
    INV_OPEN,
    MESSAGE_DISPLAYED,
    MONSTER_CAUGHT
} BattleState;

static BattleState battle_state = MAIN_MENU;

void BattleInit(player_t* player, monster_t* enemy_monster){
    printf("BATTLE STARTING\n");
    active_player = player;
    battle_state = MAIN_MENU;

    if (battle_menu) MenuDestroy(battle_menu);
    if (switch_menu) MenuDestroy(switch_menu);
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

    int w, h;
    SDL_GetRendererOutputSize(rend, &w, &h);
    switch_menu = MenuCreate(5, 1, 1, BattleDraw, BattleMenuHandleSelect);
    for(int i = 0; i < switch_menu->items_amount; i++){
        if( !(i & 1) ) switch_menu->menu_items[i].x = (w/2)-420;
        else           switch_menu->menu_items[i].x = (w/2)+20;
        switch_menu->menu_items[i].y = 350 + i*100;
        switch_menu->menu_items[i].w = 400;
        switch_menu->menu_items[i].h = 100;
    }
    switch_menu->back = BattleMenuBack;
    // Just to hep on BattleDraw function to prevent it running when it is not supposed to
    enemy_mon = enemy_monster;

    // Set player's active monster to the first one in the party
    player->active_mon_index = 0;

    player_displayed_hp = (float)player->monster_party[0]->current_hp;
    enemy_displayed_hp = (float)enemy_mon->current_hp;

    // Set player's game state to battle LAST to prevent race conditions
    player->selected_menu_itm = ATTACK;
    player->current_menu = battle_menu;
    player->game_state = STATE_IN_MENU;
}

// Checks if battle is over (enemy monster is dead)
// If so calls other functions such as increase exp
// Returns >1 if battle is over and 0 if not
static int BattleCheckIsOver(){
    int has_mon_left = 0;
    for(int i = 0; i < PARTY_SIZE; i++){
        if(active_player->monster_party[i]){
            if(active_player->monster_party[i]->current_hp > 0) has_mon_left = 1;
        }
    }

    if(enemy_mon->current_hp <= 0){
        MonsterAddExp(active_player->monster_party[active_player->active_mon_index], enemy_mon);
        printf("Battle Won! Current Exp: %d/%d\n", 
            active_player->monster_party[active_player->active_mon_index]->current_exp,
            active_player->monster_party[active_player->active_mon_index]->exp_to_next_level);
        return 1;
    }
    else if(active_player->monster_party[active_player->active_mon_index]->current_hp <= 0 && has_mon_left){
        MenuDeHighlightBox(&active_player->current_menu->menu_items[active_player->selected_menu_itm]);
        active_player->selected_menu_itm = SWITCH;
        MenuHighlightBox(&active_player->current_menu->menu_items[SWITCH]);

        battle_state = MAIN_MENU;
        BattleMenuHandleSelect();
    }
    else if(active_player->monster_party[active_player->active_mon_index]->current_hp <= 0 && !has_mon_left){
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

static void BattleRenderInfo(const char* btn_text, SDL_Rect* dst_rect, int x_offset, int y_offset, int is_right_aligned){
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
    if(is_right_aligned){
        text_rect.x = dst_rect->x + dst_rect->w - text_w - x_offset;
        text_rect.y = dst_rect->y + y_offset;
        text_rect.w = text_w;
        text_rect.h = text_h;
    }
    else{
        text_rect.x = dst_rect->x + x_offset;
        text_rect.y = dst_rect->y + y_offset;
        text_rect.w = text_w;
        text_rect.h = text_h;
    }

    // Render the text
    SDL_RenderCopy(rend, text_texture, NULL, &text_rect);
    // Draw the text_rect outline
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderDrawRect(rend, &text_rect);

    SDL_DestroyTexture(text_texture);
}

static void RenderHpBar(int current_hp, int max_hp, SDL_Rect hp_bar){
    float hp_percent = (float)current_hp / (float)max_hp;
    int max_size = hp_bar.w;
    hp_bar.w = (int)(hp_bar.w * hp_percent);

    if(current_hp < max_hp){
        SDL_Rect rem_hp = {hp_bar.x + hp_bar.w, hp_bar.y, max_size - hp_bar.w, hp_bar.h};// Render almost balck opaque
        SDL_SetRenderDrawColor(rend, 22, 22, 22, 255);
        SDL_RenderFillRect(rend, &rem_hp);
    }

    // Render color to green
    SDL_SetRenderDrawColor(rend, 0, 255, 0, 255);
    SDL_RenderFillRect(rend, &hp_bar);
    // Reset render color
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}

static void RenderMonInfo(monster_t* active_mon){
    BattleRenderInfo(enemy_mon->monster_name, &enemy_rect, 20, 20, 0);
    BattleRenderInfo(active_mon->monster_name, &player_rect, 20, 20, 0);

    // Is freed whenever the function stops running
    char enemy_lvl_info[12];
    sprintf(enemy_lvl_info, "lvl: %d", enemy_mon->level);

    char player_lvl_info[12];
    sprintf(player_lvl_info, "lvl: %d", active_mon->level);
    // Render the enemy monster's level
    BattleRenderInfo(enemy_lvl_info, &enemy_rect, 20, 20, 1);
    BattleRenderInfo(player_lvl_info, &player_rect, 20, 20, 1);

    char enemy_hp_info[12];
    sprintf(enemy_hp_info, "%d/%d", enemy_mon->current_hp, enemy_mon->max_hp);

    char player_hp_info[12];
    sprintf(player_hp_info, "%d/%d", active_mon->current_hp, active_mon->max_hp);
    // Render the enemy monster's hp info
    BattleRenderInfo(enemy_hp_info, &enemy_rect, 20, 50, 1);
    BattleRenderInfo(player_hp_info, &player_rect, 20, 50, 1);

    RenderHpBar((int)player_displayed_hp, active_mon->max_hp, player_hp_rect);
    RenderHpBar((int)enemy_displayed_hp, enemy_mon->max_hp, enemy_hp_rect);
}

static void MovesMenuDraw(monster_t* active_mon){
    for(int i = 0; i < 4; i++){
        if(active_mon->usable_moves[i].id != -1){
            BattleRenderMenuItem(active_mon->usable_moves[i].move_name, &battle_menu->menu_items[i]);

            char move_uses[12];
            sprintf(move_uses, "%d/%d", active_mon->usable_moves[i].available_uses,
                active_mon->usable_moves[i].max_uses);
            BattleRenderInfo(move_uses, &battle_menu->menu_items[i], 
                battle_menu->menu_items[i].w - 85, battle_menu->menu_items[i].h - 30, 0);

            char move_power[6];
            sprintf(move_power, "%dP", active_mon->usable_moves[i].damage);
            BattleRenderInfo(move_power, &battle_menu->menu_items[i], 
                20, battle_menu->menu_items[i].h - 30, 0);
        }
        else{
            BattleRenderMenuItem("-", &battle_menu->menu_items[i]);
        }
    }
}

static void SwitchMenuDraw(){
    for(int i = 0; i < PARTY_SIZE; i++){
        SDL_Rect cItem = switch_menu->menu_items[i];
        switch_hp_bars[i].x = cItem.x + 20; 
        switch_hp_bars[i].y = cItem.y + 52; 
        switch_hp_bars[i].w = 250; 
        switch_hp_bars[i].h = 10;
    }
    
    for(int i = 0; i < PARTY_SIZE; i++){
        if(active_player->monster_party[i]){
            BattleRenderInfo(active_player->monster_party[i]->monster_name, &switch_menu->menu_items[i], 20, 20, 0);

            char lvl_info[12];
            sprintf(lvl_info, "lvl: %d", active_player->monster_party[i]->level);
            BattleRenderInfo(lvl_info, &switch_menu->menu_items[i], 20, 20, 1);

            char hp_info[12];
            sprintf(hp_info, "%d/%d", active_player->monster_party[i]->current_hp,
                active_player->monster_party[i]->max_hp);
            BattleRenderInfo(hp_info, &switch_menu->menu_items[i], 20, 50, 1);

            RenderHpBar(active_player->monster_party[i]->current_hp, active_player->monster_party[i]->max_hp, switch_hp_bars[i]);
        }
        else
            BattleRenderMenuItem("-", &switch_menu->menu_items[i]);
    }
}

static void BattleExecuteTurns(monster_t* player_mon){
    static char msg[1024];
    if(turn_stage == 0){
        msg[0] = '\0';
        // Execute First Attack
        if(fst_atck_move){
            monster_t* target = (first_attacker == player_mon) ? enemy_mon : player_mon;
            MonsterUseMoveOn(first_attacker, fst_atck_move, target, msg);
        }
        turn_stage = 1;
    }
    else if(turn_stage == 1){
        if(msg[0] != '\0'){
            BattleRenderInfo(msg, &status_rect, 20, 20 ,0);
            BattleRenderInfo("->", &status_rect, 750, 150, 0);
        }
        else{
            // Check if second attacker has fainted to display it
            if(fst_atck_move){
                if(second_attacker->current_hp <= 0){
                    char info[269];
                    sprintf(info, "%s has fainted!", second_attacker->monster_name);
                    BattleRenderInfo(info, &status_rect, 20, 20, 0);
                }
                else{
                    // Print normal attack info
                    char atck_info[520];
                    snprintf(atck_info, 520, "%s USED %s", first_attacker->monster_name, fst_atck_move->move_name);
                    BattleRenderInfo(atck_info, &status_rect, 20, 20, 0);
                }
                BattleRenderInfo("->", &status_rect, 750, 150, 0);
            }
            else {
                turn_stage = 2;
            }
        }
        
    }
    else if(turn_stage == 2){
        msg[0] = '\0';
        // Execute Second Attack
        if(scnd_atck_move){
            monster_t* target = (second_attacker == player_mon) ? enemy_mon : player_mon;
            MonsterUseMoveOn(second_attacker, scnd_atck_move, target, msg);
        }
        turn_stage = 3;
    }
    else if(turn_stage == 3){
        if(msg[0] != '\0'){
            BattleRenderInfo(msg, &status_rect, 20, 20 ,0);
            BattleRenderInfo("->", &status_rect, 750, 150, 0);
        }
        else{
            // Check if first attacker has fainted to display it
            if(scnd_atck_move){
                if(first_attacker->current_hp <= 0){
                    char info[269];
                    sprintf(info, "%s has fainted!", first_attacker->monster_name);
                    BattleRenderInfo(info, &status_rect, 20, 20, 0);
                }
                else{
                    // Print normal attack info
                    char atck_info[520];
                    snprintf(atck_info, 520, "%s USED %s", second_attacker->monster_name, scnd_atck_move->move_name);
                    BattleRenderInfo(atck_info, &status_rect, 20, 20, 0);
                }
                // Render "Press Enter" indicator
                BattleRenderInfo("->", &status_rect, 750, 150, 0);
            }      
            else{
                battle_state = MAIN_MENU;
                active_player->is_player_turn = 1;
                active_player->current_menu = battle_menu;
            }
        }
    }
}

void BattleDraw(){
    if (!enemy_mon || !battle_menu || !active_player) return;
    if(!info_font) info_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", 24);
    // Safety check for player monster
    if(!active_player->monster_party[active_player->active_mon_index]) return;

    monster_t* active_mon = active_player->monster_party[active_player->active_mon_index];

    // Animate HP bars
    float player_target = (float)active_mon->current_hp;
    player_displayed_hp += (player_target - player_displayed_hp) * 0.1f;
    if(fabs(player_target - player_displayed_hp) < 0.5f) player_displayed_hp = player_target;

    float enemy_target = (float)enemy_mon->current_hp;
    enemy_displayed_hp += (enemy_target - enemy_displayed_hp) * 0.1f;
    if(fabs(enemy_target - enemy_displayed_hp) < 0.5f) enemy_displayed_hp = enemy_target;

    if(battle_state == MAIN_MENU || battle_state == INV_OPEN){
        // Render normal menu items
        BattleRenderMenuItem("SWITCH", &battle_menu->menu_items[SWITCH]);
        BattleRenderMenuItem("ATTACK", &battle_menu->menu_items[ATTACK]);
        BattleRenderMenuItem("INVENTORY", &battle_menu->menu_items[INVENTORY]);
        BattleRenderMenuItem("RUN", &battle_menu->menu_items[RUN]);
    }
    else if(battle_state == MOVES_MENU)     MovesMenuDraw(active_mon);
    else if(battle_state == EXECUTING_TURN) BattleExecuteTurns(active_mon);
    else if(battle_state == MESSAGE_DISPLAYED || battle_state == MONSTER_CAUGHT){
        BattleRenderInfo(message, &status_rect, 20, 20, 0);
        BattleRenderInfo("->", &status_rect, 750, 150, 0);
    }

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
    RenderMonInfo(active_mon);
    
    if(battle_state == SWITCH_MENU) SwitchMenuDraw();
    if(battle_state == INV_OPEN) InventoryDraw(active_player->inv);
}

static void HandleMovesMenuSelect(monster_t* active_mon){
    // Make it so it is not the player's turn anymore
    active_player->is_player_turn = 0;

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

    MenuDeHighlightBox(&active_player->current_menu->menu_items[active_player->selected_menu_itm]);
    active_player->selected_menu_itm = 0;
    MenuHighlightBox(&active_player->current_menu->menu_items[ATTACK]);
}

static void HandleInvOpenSelect(monster_t* active_mon){
    inventory_item_t* item = InventoryGetCurrent(active_player->inv);
    switch(item->type){
    case 0:
        if(item->count <= 0 || item->id == -1) return;
        printf("Used item with id: %d\n", item->id);
                
        InventoryRemoveItem(active_player->inv, item->item, 1);
        catch_device_t* device = (catch_device_t*) item->item;
        int has_caught = MonsterTryCatch(active_player, enemy_mon, device);
        if(has_caught){
            sprintf(message,"You caught a(n) %s!", enemy_mon->monster_name);

            fflush(stdout);
                    
            active_player->inv_isOpen = 0;
            battle_state = MONSTER_CAUGHT;
        }
        else{
            sprintf(message,"%s broke free!", enemy_mon->monster_name);
            active_player->inv_isOpen = 0;
            battle_state = MESSAGE_DISPLAYED;
        }

        break;
    case 1:
        if(item->count <= 0 || item->id == -1) return;
        printf("Used healing item with id: %d\n", item->id);

        restore_item_t* pot = (restore_item_t*) item->item;
        int has_healed = MonsterHeal(active_mon, pot->restore_amount);
        if(has_healed){
            InventoryRemoveItem(active_player->inv, item->item, 1);
            sprintf(message, "Used a %s on %s", pot->name, active_mon->monster_name);
                    
            battle_state = MESSAGE_DISPLAYED;
        }

        active_player->inv_isOpen = 0;

        break;
    default:
        return;
    }
}

static int HandleSwitchMenuSelect(monster_t* active_mon){
    if(!active_player->monster_party[active_player->selected_menu_itm]) return 0;
    if(active_player->monster_party[active_player->selected_menu_itm]->current_hp <= 0) return 0;
    if(active_player->monster_party[active_player->selected_menu_itm] == active_player->monster_party[active_player->active_mon_index]) return 0;

    player_mon_tex = NULL;
    active_player->active_mon_index = active_player->selected_menu_itm;
    active_mon = active_player->monster_party[active_player->active_mon_index];
    
    player_displayed_hp = (float)active_mon->current_hp;
    sprintf(message, "Sent out %s!", active_mon->monster_name);

    battle_state = MESSAGE_DISPLAYED;
    return 1;
}

void BattleMenuHandleSelect(){
    monster_t* active_mon = active_player->monster_party[active_player->active_mon_index];
    if(battle_state == EXECUTING_TURN) {
        if (turn_stage == 1) {
             if(BattleCheckIsOver()){
                active_player->game_state = STATE_EXPLORING;
             } else if(battle_state == EXECUTING_TURN) {
                turn_stage = 2;
             }
        } else if (turn_stage == 3) {
             if(BattleCheckIsOver()){
                active_player->game_state = STATE_EXPLORING;
             } else if(battle_state == EXECUTING_TURN) {
                battle_state = MAIN_MENU;
                active_player->is_player_turn = 1;
                active_player->current_menu = battle_menu;
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
            active_player->inv_isOpen = 1;
            active_player->current_menu = active_player->inv->menu;
            active_player->current_menu->select_routine = BattleMenuHandleSelect;
            active_player->current_menu->draw = BattleDraw;
            active_player->current_menu->back = BattleMenuBack;
        }
        else if(selected_btn == SWITCH){
            battle_state = SWITCH_MENU;
            MenuHighlightBox(&switch_menu->menu_items[0]);
            active_player->current_menu = switch_menu;
            active_player->current_menu->draw = BattleDraw;
            active_player->selected_menu_itm = 0;
        }
    }
    else if(battle_state == MOVES_MENU)  HandleMovesMenuSelect(active_mon);
    else if(battle_state == INV_OPEN)    HandleInvOpenSelect(active_mon);
    else if(battle_state == SWITCH_MENU){
        int has_switched = HandleSwitchMenuSelect(active_mon);
        
        if(has_switched){
            MenuDeHighlightBox(&active_player->current_menu->menu_items[active_player->selected_menu_itm]);
            active_player->selected_menu_itm = SWITCH;
        }
    }
    else if(battle_state == MONSTER_CAUGHT){
        active_player->game_state = STATE_EXPLORING;
    }
    else if(battle_state == MESSAGE_DISPLAYED){
        // Make it so it is not the player's turn anymore
        active_player->is_player_turn = 0;
        // Enemy must take an action after the player
        if(active_mon->speed >= enemy_mon->speed){
            first_attacker = active_mon;
            second_attacker = enemy_mon;

            fst_atck_move = NULL;
            scnd_atck_move = MonsterChooseEnemyAttack(enemy_mon);
        } else {
            first_attacker = enemy_mon;
            second_attacker = active_mon;

            fst_atck_move = MonsterChooseEnemyAttack(enemy_mon);
            scnd_atck_move = NULL;
        }
        printf("EXECUTING TURN\n");
        battle_state = EXECUTING_TURN;
        turn_stage = 0;
    }
}

void BattleMenuBack(){
    if(battle_state == MAIN_MENU) return;
    if(battle_state == EXECUTING_TURN) return;
    
    
    if(battle_state == MOVES_MENU){
        MenuDeHighlightBox(&active_player->current_menu->menu_items[active_player->selected_menu_itm]);
        active_player->selected_menu_itm = ATTACK;
        MenuHighlightBox(&battle_menu->menu_items[ATTACK]);
    }
    if(battle_state == INV_OPEN){
        active_player->inv_isOpen = 0;
        active_player->selected_menu_itm = INVENTORY;
    }
    if(battle_state == SWITCH_MENU){
        MenuDeHighlightBox(&active_player->current_menu->menu_items[active_player->selected_menu_itm]);
        active_player->current_menu = battle_menu;
        active_player->selected_menu_itm = SWITCH;
    }

    battle_state = MAIN_MENU;
    active_player->current_menu = battle_menu;
}

void BattleQuit(void){
    if(battle_menu){
        // Prevents double frees
        if(active_player) active_player->current_menu = NULL;
        
        MenuDestroy(battle_menu);
        battle_menu = NULL;
    }
    if(switch_menu){
        if(active_player) active_player->current_menu = NULL;
        MenuDestroy(switch_menu);
        switch_menu = NULL;
    }

    if(info_font){
        TTF_CloseFont(info_font);
        info_font = NULL;
    }

    if(player_mon_tex){
        SDL_DestroyTexture(player_mon_tex);
        player_mon_tex = NULL;
    }
    if(enemy_mon_tex){
        SDL_DestroyTexture(enemy_mon_tex);
        enemy_mon_tex = NULL;
    }

    if(active_player) active_player->game_state = STATE_EXPLORING;
}