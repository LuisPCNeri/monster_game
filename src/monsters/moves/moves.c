#include <stdlib.h>

#include "moves.h"
#include "monsters/monster.h"
#include "utils/utils.h"
#include "utils/term_colors.h"

#define MAX_GAME_MOVES 500
static move_t ALL_MOVES[MAX_GAME_MOVES];
static int16_t MoveLibraryCount = 0;

void MovesInit() {
    // LOAD ALL MOVES INTO THE LIBRARY
    char* moveData = LoadFileToString("data/moves.json");
    if(moveData) {
        cJSON* jsonMoves = cJSON_Parse(moveData);
        cJSON* entry = NULL;

        cJSON_ArrayForEach(entry, jsonMoves) {
            if(MoveLibraryCount >= MAX_GAME_MOVES) break;
            
            move_t* m = &ALL_MOVES[MoveLibraryCount];
            MoveParseJSON(entry, m);
            MoveLibraryCount++;
        }
        cJSON_Delete(jsonMoves);
        free(moveData);
        printf(ANSI_COLOR_GREEN"Loaded %d moves.\n"ANSI_COLOR_RESET, MoveLibraryCount);
    }
}

move_t* GetMoveById(int16_t id) {
    for(int16_t i = 0; i < MoveLibraryCount; i++) {
        if(ALL_MOVES[i].id == id) {
            return &ALL_MOVES[i];
        }
    }
    return NULL;
}

int16_t* GetLearnedMovesIdPermutation(monster_t* m) {

    int16_t* moves_array = (int16_t*) calloc(MAX_LEVEL * LEARNABLE_MOVES_AMOUNT_PER_LEVEL, sizeof(int16_t));
    int8_t count = 0;

    for(int i = 0; i <= m->level; i++) {
        for(int k = 0; k < LEARNABLE_MOVES_AMOUNT_PER_LEVEL; k++) {
            if(m->level_up_table[i][k] == -1)
                continue;

            moves_array[count] = m->level_up_table[i][k];
            count++;
        }
    }

    if(count <= 4) return moves_array;


    int16_t* moves = (int16_t*) malloc(2*4);
    int8_t gotten_moves = 0;

    int16_t picked[count];
    memset(picked, 0, count);

    for(int i = 0; i < USBALE_MOVES_AMOUNT; i++) {
        int16_t rnd;

        do { rnd = (rand() % count); } while(picked[rnd]);
        picked[rnd] = 1;

        moves[gotten_moves] = moves_array[rnd];
        gotten_moves++;
    }

    free(moves_array);
    return moves;
}

void MoveParseJSON(cJSON* entry, move_t* m){
    // Load basic data
    m->id = cJSON_GetObjectItem(entry, "id")->valueint;
    m->move_name = SDL_strdup(cJSON_GetObjectItem(entry, "name")->valuestring);
    m->required_level = cJSON_GetObjectItem(entry, "req_level")->valueint;
    m->damage = cJSON_GetObjectItem(entry, "power")->valueint;
    m->max_uses = cJSON_GetObjectItem(entry, "max_pp")->valueint;
    m->status_effect = cJSON_GetObjectItem(entry, "status_fx")->valueint;

    char* type = cJSON_GetObjectItem(entry, "type")->valuestring;
    m->attack_type = MonsterGetTypeFromString(type);

    cJSON* acc_item = cJSON_GetObjectItem(entry, "accuracy");
    m->acc_percent = acc_item ? acc_item->valueint : 100;
            
    // Load Stat Modifiers
    cJSON* stat_item = cJSON_GetObjectItem(entry, "stat_target");
    if(stat_item){
        char* s = stat_item->valuestring;
        if(     strcmp(s, "ATTACK")  == 0) m->stat_to_modify = STAT_ATTACK;
        else if(strcmp(s, "DEFENSE") == 0) m->stat_to_modify = STAT_DEFENSE;
        else if(strcmp(s, "SPEED")   == 0) m->stat_to_modify = STAT_SPEED;
        else m->stat_to_modify = STAT_NONE;
    }
    else {
        m->stat_to_modify = STAT_NONE;
    }

    cJSON* stage_item = cJSON_GetObjectItem(entry, "stage_change");
    m->stat_stage_change = stage_item ? stage_item->valueint : 0;
    cJSON* self_item = cJSON_GetObjectItem(entry, "target_self");
    m->is_modify_self = self_item ? self_item->valueint : 0;

    // Initialize current state
    m->available_uses = m->max_uses; 
}
