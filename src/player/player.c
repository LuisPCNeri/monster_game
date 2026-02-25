#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "monsters/monster.h"
#include "menus/menu.h"
#include "player/inventory.h"

#define STARTER_NUM 3

extern SDL_Renderer* rend;

static menu_t* starter_select_menu = NULL;
static monster_t* starter_mons[STARTER_NUM];

static player_t* active_player = NULL;

static int starting_sprite_stage = 0;
static int max_sprite_stage = 0;

player_t* PlayerInit(){
    player_t* player = (player_t*) calloc(1, sizeof(player_t));

    SDL_Surface* player_sprite_sheet_surf = IMG_Load("resources/player_sheet_overworld.png");
    if(!player_sprite_sheet_surf) printf("Error loading image: %s\n", IMG_GetError());
    SDL_Texture* player_sprite_sheet = SDL_CreateTextureFromSurface(rend, player_sprite_sheet_surf);
    SDL_FreeSurface(player_sprite_sheet_surf);

    player->x_pos = 0;
    player->y_pos = 0;
    player->inv_isOpen = 0;
    player->inv = InventoryCreateEmpty();
    player->sprite_sheet = player_sprite_sheet;
    player->sprite_stage = 1;

    player->sprite_rect.x = 0;
    player->sprite_rect.y = 0;
    player->sprite_rect.w = PLAYER_SPRITE_SIZE;
    player->sprite_rect.h = PLAYER_SPRITE_SIZE;

    return player;
}

void PlayerStarterMenuDraw(Uint32 dt){
    (void)dt;
    if(!starter_select_menu) return;

    for(int i = 0; i < STARTER_NUM; i++){
        if(starter_mons[i]) MenuRenderItem(starter_mons[i]->monster_name, &starter_select_menu->menu_items[i]);
    }
}

// Then make menu SDL_Rects for each starter and have the player select one
// The selected one will be attributed to the player
// Right now sets the monster to the well uhhh the one available
void PlayerSetStarters(player_t* player){
    active_player = player;
    // Create the menu struct
    if(starter_select_menu) MenuDestroy(starter_select_menu);
    starter_select_menu = MenuCreate(3, 0, 1, &PlayerStarterMenuDraw, &PlayerMenuHandleSelect);

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
    starter_mons[1] = GetMonsterById(4);
    starter_mons[2] = GetMonsterById(7);

    player->current_menu = starter_select_menu;
    player->selected_menu_itm = 0;
    player->game_state = STATE_IN_MENU;
    active_player = player;
}

void PlayerMenuHandleSelect(){
    for(int i = 0; i < PARTY_SIZE; i++){
        // INITIALIZEALL PLAYER PARTY VALUES TO NULL
        active_player->monster_party[i] = NULL;
    }

    // Makes a copy of the generic monster in the arry
    // This copy corresponds to the monster the player chose
    monster_t* selected_mon = (monster_t*) malloc(sizeof(monster_t));
    *selected_mon = *(starter_mons[active_player->selected_menu_itm]);
    selected_mon->level = 5;

    MonsterSetStats(selected_mon);

    // Adds the chosen starter to the player's party at the first position
    active_player->monster_party[0] = selected_mon;

    active_player->game_state = STATE_EXPLORING;
    
    MenuDestroy(starter_select_menu);
    active_player->current_menu = NULL;
}

int PlayerAddMonsterToParty(monster_t* monster){
    int has_space = 0;
    
    for(int i = 0; i < PARTY_SIZE; i++){
        if(!active_player->monster_party[i]){
            active_player->monster_party[i] = (monster_t*) malloc(sizeof(monster_t));

            *active_player->monster_party[i] = *monster;
            has_space = 1;
            break;
        }
    }

    // TODO : PC saving monster stuff
    if(!has_space){}

    return has_space;
}

int PlayerCheckIsPartyDead(player_t* player){
    for(int i = 0; i < PARTY_SIZE; i++){
        if(player->monster_party[i]){
            if(player->monster_party[i]->current_hp > 0) return 0;
        }
    }

    return 1;
}

void PlayerRenderNotifBox(player_t* player, int offset_x, int offset_y, Uint32 dt){
    static SDL_Texture* notif_text = NULL;
    if(!notif_text) {
        SDL_Surface* notif_surf = IMG_Load("resources/player_notif.png");
        if(notif_surf) {
            notif_text = SDL_CreateTextureFromSurface(rend, notif_surf);
            SDL_FreeSurface(notif_surf);
        }
    }

    // blink blink fucker
    static int blink_timer = 0;
    blink_timer += dt;
    if ((blink_timer / BLINK_FRAMES) % 2 != 0) return;

    SDL_Rect notif_box = {
        .x = player->x_pos + offset_x - (16/2), 
        .y = player->y_pos + offset_y - (PLAYER_SPRITE_SIZE / 2) - 16 - 4, 
        .w = 16, .h = 16
    };
    if(notif_text) SDL_RenderCopy(rend, notif_text, NULL, &notif_box);
}

SDL_Rect PlayerGetSheetWindow(player_t* p){
    SDL_Rect window = {
        // There is one pixel between every 16x16 px sprite in the sprite sheet
        // The + sprite_stage takes that into account
        p->sprite_stage * 16 + p->sprite_stage, 
        0,
        16, 16
    };

    return window;
}

static void UpdateBaseSpriteStages(player_t* player){
    switch (player->facing_direction){
        case NORTH:
        starting_sprite_stage = 3;
        max_sprite_stage = 5;
        break;
        case SOUTH:
        starting_sprite_stage = 0;
        max_sprite_stage = 2;
        break;
        case WEST:
        starting_sprite_stage = 6;
        max_sprite_stage = 7;
        break;
        case EAST:
        starting_sprite_stage = 8;
        max_sprite_stage = 9;
    }
}

void PlayerMove(player_t* player, Orientation direction, int* world_pos){

    if(player->facing_direction == direction){
        int range = max_sprite_stage - starting_sprite_stage + 1;
        if (range > 0) {
            player->sprite_stage = starting_sprite_stage + (SDL_GetTicks64() / 200) % range;
        }
    }else{
        player->facing_direction = direction;
        UpdateBaseSpriteStages(player);
        player->sprite_stage = starting_sprite_stage;
    }

    switch (player->facing_direction){
        case NORTH:
        *world_pos -= CHARACTER_SPEED / 30;
        player->y_pos = *world_pos + (player->sprite_rect.h / 2);
        break;
        case SOUTH:
        *world_pos += CHARACTER_SPEED / 30;
        player->y_pos = *world_pos + (player->sprite_rect.h / 2);
        break;
        case WEST:
        *world_pos -= CHARACTER_SPEED / 30;
        player->x_pos = *world_pos + (player->sprite_rect.h / 2);
        break;
        case EAST:
        *world_pos += CHARACTER_SPEED / 30;
        player->x_pos = *world_pos + (player->sprite_rect.h / 2);
        break;
    }
}

void PlayerDestroy(player_t* p){
    for(unsigned int i = 0; i < PARTY_SIZE; i++){
        if(p->monster_party[i]) free(p->monster_party[i]);
    }

    if(p->sprite_sheet) SDL_DestroyTexture(p->sprite_sheet);
    if(p->current_menu) MenuDestroy(p->current_menu);
    InventoryDestroy(p->inv);
    free(p);
}