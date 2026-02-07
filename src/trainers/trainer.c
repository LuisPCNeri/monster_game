// TODO : Trainer AI will be worked on with a weight system
// Trainers may have special traits as CAUTIOUS, AGGRESSIVE, ...
// These traits should change the way the moves are weighted for that trainer

// More expirienced trainers ought to have a better weight system as in
// probably switch monsters if their current one is not the best for the current enemy
// availability to use items
// and in general a weight system that would result in a MUCH harder battle
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "trainer.h"
#include "map.h"
#include "monsters/battle/battle.h"
#include "trainer_battle/trainer_battle.h"
#include "monsters/monster.h"

#define TRAINER_FILE "data/trainer.json"
#define TRAINER_SPRITE_SIZE 128
// Max ditance trainer can aggro from in TILES
#define MAX_AGGRO_DIST 10
#define AGGRO_LENIENCE 10

extern SDL_Renderer* rend;

static int world_offset_x, world_offset_y;
// Number of trainers is arbitrary and can me increased or decreased at any time
static trainer_t TRAINERS[100];
static unsigned int current_trainers = 0;

static const char* NOTIF_SOUND_LOC = "resources/sfx/notif_sfx.mp3";
static Mix_Music* notif_sound = NULL;

static char* LoadFileToString(char* file_path){
    FILE* fptr;
    if(!(fptr = fopen(file_path, "r"))){
        perror("Open JSON Monster File: ");
        return NULL;
    }

    // Count file size
    fseek(fptr, 0 , SEEK_END);
    long file_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char* buffer = (char*) malloc((size_t) file_size + 1);
    size_t out = fread(buffer, 1, file_size, fptr);
    if(out == 0) perror("fread");
    buffer[file_size] = '\0';

    fclose(fptr);
    return buffer;
}

static void TrainerPrint(trainer_t* t){
    printf(
        "Name: %s\r\n"
        "Orientation: %d\r\n"
        "x:    %d\r\n"
        "y:    %d\r\n",
        t->name, t->facing_direction, t->x_pos, t->y_pos
    );
}

static FacingDirection GetOrientationFromString(const char* orientation){
    if(     strcmp("FRONT",orientation) == 0) return FRONT;
    else if(strcmp("BACK" ,orientation) == 0) return BACK;
    else if(strcmp("RIGHT",orientation) == 0) return RIGHT;
    else if(strcmp("LEFT" ,orientation) == 0) return LEFT;

    return -1;
}

void TrainersInit(){
    char* file = LoadFileToString(TRAINER_FILE);
    if(!file) exit(EXIT_FAILURE);

    cJSON* trainers = cJSON_Parse(file);
    cJSON* entry = NULL;
    cJSON_ArrayForEach(entry, trainers){
        trainer_t* t = &TRAINERS[current_trainers];
        strcpy(t->name, cJSON_GetObjectItem(entry, "name")->valuestring);
        strcpy(t->sprite_path, cJSON_GetObjectItem(entry, "sprite")->valuestring);
        strcpy(t->intro_msg, cJSON_GetObjectItem(entry, "intro_msg")->valuestring);
        t->type = MonsterGetTypeFromString(cJSON_GetObjectItem(entry, "type")->valuestring);
        t->facing_direction = GetOrientationFromString(cJSON_GetObjectItem(entry, "orientation")->valuestring);
        t->x_pos = cJSON_GetObjectItem(entry, "x")->valueint;
        t->y_pos = cJSON_GetObjectItem(entry, "y")->valueint;

        t->was_defeated = 0;

        cJSON* party = cJSON_GetObjectItem(entry, "party");
        cJSON* monster = NULL;
        int slot = 0;
        
        for(int i = 0; i < PARTY_SIZE; i++) t->party[i].id = -1;

        cJSON_ArrayForEach(monster, party){
            if(slot > PARTY_SIZE) break;
            monster_t* m = &t->party[slot];
            MonsterParseJSON(monster, m);
            slot++;
        };

        current_trainers++;
    };

    printf("LOADED %d TRAINERS\n", current_trainers);
    cJSON_Delete(trainers);
    free(file);
}

int TrainerIsVisible(trainer_t* t, int offset_x, int offset_y){
    int screen_w, screen_h;
    SDL_GetRendererOutputSize(rend , &screen_w, &screen_h);

    int draw_x = t->x_pos + offset_x;
    int draw_y = t->y_pos + offset_y;

    return (draw_x + TRAINER_SPRITE_SIZE > 0 && draw_x < screen_w &&
            draw_y + TRAINER_SPRITE_SIZE > 0 && draw_y < screen_h);
}

void TrainerDraw(int offset_x, int offset_y){
    world_offset_x = offset_x;
    world_offset_y = offset_y;

    for(unsigned int i = 0; i < current_trainers; i++){
        if(!TrainerIsVisible(&TRAINERS[i], offset_x, offset_y)) continue;

        if(TRAINERS[i].texture == NULL){
            SDL_Surface* s = IMG_Load(TRAINERS[i].sprite_path);
            if(s){
                TRAINERS[i].texture = SDL_CreateTextureFromSurface(rend, s);
                SDL_FreeSurface(s);
            }
        }

        if(TRAINERS[i].texture == NULL) continue;

        SDL_Rect src_rect;
        src_rect.w = 256;
        src_rect.h = 256;
        SDL_Rect dst_rect = { TRAINERS[i].x_pos + offset_x, TRAINERS[i].y_pos + offset_y, TRAINER_SPRITE_SIZE, TRAINER_SPRITE_SIZE};

        switch (TRAINERS[i].facing_direction){
        case FRONT:
            src_rect.x = 0;
            src_rect.y = 0;
            break;
        case BACK:
            src_rect.x = 256;
            src_rect.y = 0;
            break;
        case RIGHT:
            src_rect.x = 0;
            src_rect.y = 256;
            break;
        case LEFT:
            src_rect.x = 256;
            src_rect.y = 256;
            break;
        }

        SDL_RenderCopy(rend, TRAINERS[i].texture, &src_rect, &dst_rect);
    }
}

static trainer_t* TrainerGetClosest(player_t* p){
    if(current_trainers < 2) return &TRAINERS[0];

    trainer_t* closest = NULL;
    int closest_dist = INT32_MAX;
    for(unsigned int i = 0; i < current_trainers; i++){
        int dist = sqrt(pow(p->x_pos - TRAINERS[i].x_pos, 2) + pow(p->y_pos - TRAINERS[i].y_pos, 2));
        if(closest_dist > dist){
            closest_dist = dist;
            closest = &TRAINERS[i];
        }
    }

    return closest;
}

static void TrainerAggro(player_t* p, trainer_t* t){
    if(!notif_sound) notif_sound = Mix_LoadMUS(NOTIF_SOUND_LOC);
    Mix_PlayMusic(notif_sound, 1);

    p->aggro_trainer = t;
    p->game_state = STATE_AGGRO;
    p->aggro_timer = PLAYER_AGGRO_TIMER;
}

int TrainerCheckAggro(player_t* player){
    trainer_t* closest = TrainerGetClosest(player);

    if(closest->was_defeated) return 0;
    if(PlayerCheckIsPartyDead(player)) return 0;

    int dist = sqrt(pow(player->x_pos - closest->x_pos, 2) + pow(player->y_pos - closest->y_pos, 2));

    // Position of the center of the closest trainer's sprite
    int trainer_CoM_x = closest->x_pos + (TRAINER_SPRITE_SIZE / 2);
    int trainer_CoM_y = closest->y_pos + (TRAINER_SPRITE_SIZE) / 2;

    if(dist / TILE_SIZE > MAX_AGGRO_DIST) return 0;

    if(closest->facing_direction == FRONT){
        if(trainer_CoM_x + AGGRO_LENIENCE > player->x_pos && trainer_CoM_x - AGGRO_LENIENCE < player->x_pos
        && player->y_pos >= closest->y_pos){
            
            TrainerAggro(player, closest);
            return 1;
        }
    }
    else if(closest->facing_direction == BACK){
        if(trainer_CoM_x + AGGRO_LENIENCE > player->x_pos && trainer_CoM_x - AGGRO_LENIENCE < player->x_pos
        && player->y_pos <= closest->y_pos){

            TrainerAggro(player, closest);
            return 1;
        }
    }
    else if(closest->facing_direction == LEFT){
        if(trainer_CoM_y + AGGRO_LENIENCE > player->y_pos && trainer_CoM_y - AGGRO_LENIENCE < player->y_pos 
        && player->x_pos <= closest->x_pos){

            TrainerAggro(player, closest);
            return 1;
        }
    }
    else if(closest->facing_direction == RIGHT){
        if(trainer_CoM_y + AGGRO_LENIENCE > player->y_pos && trainer_CoM_y - AGGRO_LENIENCE < player->y_pos 
        && player->x_pos >= closest->x_pos){

            TrainerAggro(player, closest);
            return 1;
        }
    }

    return 0;
}

void TrainerUpdateAggro(player_t* player){
    if(player->aggro_timer > 0){
        player->aggro_timer--;
    } else {
        if(notif_sound) Mix_FreeMusic(notif_sound);

        TrainerBattleInit(player, player->aggro_trainer);
        player->aggro_trainer = NULL;
    }
}

int TrainerCheckPartyIsDead(trainer_t* trainer){
    for(int i = 0; i < PARTY_SIZE; i++){
        if(trainer->party[i].current_hp > 0) return 0;
    }

    return 1;
}

void TrainerRestoreParty(trainer_t* trainer){
    for(int i = 0; i < PARTY_SIZE; i++){
        if(trainer->party[i].id == -1) continue;

        monster_t* mon = &trainer->party[i];
        mon->current_hp = mon->max_hp;
        mon->spd_stage = 0;
        mon->atk_stage = 0;
        mon->def_stage = 0;
        mon->current_status_fx = NONE;
        // k < amount of moves per monster
        for(int k = 0; k < 4; k++){
            mon->usable_moves[i].available_uses = mon->usable_moves[i].max_uses;
        }
    }
}

void TrainerRenderNotifBox(trainer_t* t, int offset_x, int offset_y){
    // blink blink fucker
    static int blink_timer = 0;
    blink_timer++;
    if ((blink_timer / BLINK_FRAMES) % 2 != 0) return;

    SDL_Surface* notif_surf = IMG_Load("resources/player_notif.png");
    SDL_Texture* notif_text = SDL_CreateTextureFromSurface(rend, notif_surf);
    SDL_FreeSurface(notif_surf);

    int w, h;
    SDL_QueryTexture(notif_text, NULL, NULL, &w, &h);

    SDL_Rect notif_box = {
        .x = t->x_pos + offset_x + (TRAINER_SPRITE_SIZE/2) - 32/2, 
        .y = t->y_pos + offset_y + (TRAINER_SPRITE_SIZE/2) - 32 - 20, 
        .w = 32, .h = 32};
    SDL_RenderCopy(rend, notif_text, NULL, &notif_box);
    SDL_DestroyTexture(notif_text);
}