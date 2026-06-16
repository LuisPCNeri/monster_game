#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "libraries/cJSON.h"

#include "map.h"
#include "player/player.h"
#include "items/item.h"
#include "monster.h"
#include "battle/battle.h"
#include "utils/glbl_asset_manager.h"

#define MAX_GAME_MOVES 500
#define MAX_GAME_MONSTERS 160

#define BASE_CATCH_RATE 25

#define FRONT_MON_SHEET "resources/MON_FRONT_SHEET.png"
#define BACK_MON_SHEET "resources/MON_BACK_SHEET.png"

// GLOBAL array for this file that has IN MEMORY all the monsters (indexed by their order)
static monster_t ALL_MONSTERS[MAX_GAME_MONSTERS];

static int16_t monster_count = 0;

static move_t ALL_MOVES[MAX_GAME_MOVES];
static int16_t MoveLibraryCount = 0;

static float TypeChart[TYPE_COUNT][TYPE_COUNT];

static int32_t old_x, old_y = 0;

static const char* NOTIF_SOUND_LOC = "resources/sfx/notif_sfx.mp3";
static Mix_Music* notif_sound = NULL;

extern SDL_Renderer* rend;
extern glbl_asset_manager* asset_manager;

move_t* GetMoveById(int16_t id) {
    for(int16_t i = 0; i < MoveLibraryCount; i++) {
        if(ALL_MOVES[i].id == id) {
            return &ALL_MOVES[i];
        }
    }
    return NULL;
}

monster_t* GetMonsterById(int16_t id){
    for(int16_t i=0; i < monster_count; i++){
        if(ALL_MONSTERS[i].id == id) return &ALL_MONSTERS[i];
    }
    return NULL;
}

// Takes in the path to a file and reads it to a string
// Returns a pointer to the string or NULL if it fails
// The caller is responsible for freeing the string
static char* LoadFileToString(const char* file_path){
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

void MonsterPrint(monster_t* m){
    printf(
        "ID: %d\r\n"
        "NAME: %s\r\n"
        "DESCRIPTION: %s\r\n"
        "RARITY: %d\r\n"
        "LEVEL: %d\r\n"
        "EVO 1: %d\r\n"
        "TYPE 1: %d\r\n"
        "TYPE 2: %d\r\n"
        "HP: %d\r\n"
        "ATTACK: %d\r\n"
        "DEFENSE: %d\r\n"
        "SPEED: %d\r\n"
        "MOVE 1: %s\r\n"
        "MOVE 2: %s\r\n\n\n",
        m->id, m->name, m->description, m->rarity, m->level, m->evo_table[0][1], m->types[0], m->types[1],
        m->hp, m->attack, m->defense, m->speed, m->moves[0].move_name, m->moves[1].move_name
    );
}

MonsterTypes MonsterGetTypeFromString(const char* type_name){
    if      (strcmp(type_name, "FIRE") == 0)     return FIRE_TYPE;
    else if (strcmp(type_name, "WATER") == 0)    return WATER_TYPE;
    else if (strcmp(type_name, "GRASS") == 0)    return GRASS_TYPE;
    else if (strcmp(type_name, "ROCK") == 0)     return ROCK_TYPE;
    else if (strcmp(type_name, "POISON") == 0)   return POISON_TYPE;
    else if (strcmp(type_name, "ELECTRIC") == 0) return ELECTRIC_TYPE;
    else if (strcmp(type_name, "NORMAL") == 0)   return NORMAL_TYPE;
    else if (strcmp(type_name, "DRAGON") == 0)   return DRAGON_TYPE;
    else if (strcmp(type_name, "METAL") == 0)    return METAL_TYPE;
    else if (strcmp(type_name, "DARK") == 0)     return DARK_TYPE;
    else if (strcmp(type_name, "FLYING") == 0)   return FLYING_TYPE;
    else if (strcmp(type_name, "FIGHTING") == 0) return FIGHTING_TYPE;
    else if (strcmp(type_name, "BUG") == 0)      return BUG_TYPE;
    return NONE_TYPE;
}

SDL_Rect GetFromSpriteSheet(uint16_t sprite_size, uint16_t vertical_padding, uint16_t horizontal_padding, uint8_t col_num, uint16_t idx) {
    
    uint32_t col = idx % col_num;
    uint32_t row = idx / col_num;

    uint32_t x = col * (sprite_size + horizontal_padding);
    uint32_t y = row * (sprite_size + vertical_padding);

    SDL_Rect src_rect = {.x = x, .y = y, .w = sprite_size, .h = sprite_size};
    return src_rect;
}


/// \brief Loads the JSON entry to the correct place in the Mons array
/// \param entry The JSON entry
/// \param mon The pointer to the mon in the array
uint8_t MonsterParseJSON(cJSON* entry, monster_t* mon) {
        if (!entry || !mon) return 0;
    
    // Load Basic Info
    cJSON *id_item = cJSON_GetObjectItem(entry, "id");
    if (id_item && id_item->type == cJSON_Number) mon->id = id_item->valueint;
    else return 0;

    cJSON* sprite_idx_item = cJSON_GetObjectItem(entry, "sprite_idx");
    if(sprite_idx_item && sprite_idx_item->type == cJSON_Number) mon->sprite_idx = sprite_idx_item->valueint;
    
    cJSON *name_item = cJSON_GetObjectItem(entry, "name");
    if (name_item && name_item->type == cJSON_String) mon->name = SDL_strdup(name_item->valuestring);
    else return 0;
    
    cJSON *desc_item = cJSON_GetObjectItem(entry, "description");
    if (desc_item && desc_item->type == cJSON_String) mon->description = SDL_strdup(desc_item->valuestring);
    else return 0;

    cJSON* rarity = cJSON_GetObjectItem(entry, "rarity");
    if(rarity && rarity->type == cJSON_Number) mon->rarity = rarity->valueint;

    cJSON* level_item = cJSON_GetObjectItem(entry, "level");
    if(level_item && level_item->type == cJSON_Number) mon->level = level_item->valueint;
 
    // Load Stats
    cJSON* base_stats = cJSON_GetObjectItem(entry, "base");
    if (base_stats && base_stats->type == cJSON_Object) {
        cJSON *hp_item = cJSON_GetObjectItem(base_stats, "HP");
        if (hp_item) {
            mon->max_hp = hp_item->valueint;
            mon->hp = mon->max_hp;
        }
 
        cJSON *atk_item = cJSON_GetObjectItem(base_stats, "Attack");
        if (atk_item) mon->attack = atk_item->valueint;
 
        cJSON *sp_atk_item = cJSON_GetObjectItem(base_stats, "Sp. Attack");
        if (sp_atk_item) mon->sp_atk = sp_atk_item->valueint;
 
        cJSON *def_item = cJSON_GetObjectItem(base_stats, "Defense");
        if (def_item) mon->defense = def_item->valueint;
 
        cJSON *sp_def_item = cJSON_GetObjectItem(base_stats, "Sp. Defense");
        if (sp_def_item) mon->sp_def = sp_def_item->valueint;
 
        cJSON *spd_item = cJSON_GetObjectItem(base_stats, "Speed");
        if (spd_item) mon->speed = spd_item->valueint;
    }
 
    // Parse types array
    cJSON *types_array = cJSON_GetObjectItem(entry, "type");
    if (types_array && types_array->type == cJSON_Array) {
        int type_count = cJSON_GetArraySize(types_array);
        for (int i = 0; i < 2 && i < type_count; i++) {
            cJSON *type_item = cJSON_GetArrayItem(types_array, i);
            if (type_item && type_item->type == cJSON_String) {
                if (i == 0) {
                    mon->types[0] = MonsterGetTypeFromString(type_item->valuestring);
                } else {
                    mon->types[1] = MonsterGetTypeFromString(type_item->valuestring);
                }
            }
        }
    }
 
    // Parse evolution data
    cJSON *evolution = cJSON_GetObjectItem(entry, "evolution");
    if (evolution && evolution->type == cJSON_Object) {
        int evo_idx = 0;
        
        // Check for "next" evolution (evolves into)
        cJSON *next_evo = cJSON_GetObjectItem(evolution, "next");
        if (next_evo && next_evo->type == cJSON_Array && evo_idx < 2) {
            cJSON *evo_id = cJSON_GetArrayItem(next_evo, 0);
            cJSON *evo_method = cJSON_GetArrayItem(next_evo, 1);
            
            if (evo_id && evo_id->type == cJSON_Number) {
                mon->evo_table[evo_idx][0] = evo_id->valueint;
            }
            
            if (evo_method && evo_method->type == cJSON_String) {
                int level = 0;
                int matched = sscanf(evo_method->valuestring, "Level %d", &level);
                if (matched == 1) {
                    mon->evo_table[evo_idx][1] = level;
                } else {
                    mon->evo_table[evo_idx][1] = 255; // Marker for item-based evolution
                }
            }
            evo_idx++;
        }
        
        // Check for "prev" evolution (evolved from)
        cJSON *prev_evo = cJSON_GetObjectItem(evolution, "prev");
        if (prev_evo && prev_evo->type == cJSON_Array && evo_idx < 2) {
            cJSON *evo_id = cJSON_GetArrayItem(prev_evo, 0);
            cJSON *evo_method = cJSON_GetArrayItem(prev_evo, 1);
            
            if (evo_id && evo_id->type == cJSON_Number) {
                mon->evo_table[evo_idx][0] = evo_id->valueint;
            }
            
            if (evo_method && evo_method->type == cJSON_String) {
                int level = 0;
                int matched = sscanf(evo_method->valuestring, "Level %d", &level);
                if (matched == 1) {
                    mon->evo_table[evo_idx][1] = level;
                } else {
                    mon->evo_table[evo_idx][1] = 255;
                }
            }
            evo_idx++;
        }
    }
            
    // Load the USABLE MOVES for this monster
    cJSON* movesArray = cJSON_GetObjectItem(entry, "starting_moves");
    if(!movesArray) {
        printf("NO MOVES ARRAY\n");
        return 0;
    }

    cJSON* moveIdVal = NULL;
    int8_t slot = 0;

    // Initialize slots to empty values first
    for(int8_t k=0; k<4; k++) {
        // -1 FOR EMPTY SLOT
        mon->moves[k].id = -1;
        mon->moves[k].damage = 0;
    }

    // Loop through the JSON array
    cJSON_ArrayForEach(moveIdVal, movesArray) {
        if(slot >= 4) break;
        int16_t idToFind = moveIdVal->valueint;
                
        move_t* foundMove = GetMoveById(idToFind);
        if(foundMove != NULL) {
            mon->moves[slot] = *foundMove;         
            mon->moves[slot].available_uses = foundMove->max_uses;
                    
            slot++;
        } 
        else {
            printf("Warning: Monster %s tries to use unknown Move ID %d\n", mon->name, idToFind);
        }
    }

    for(int8_t i = 0; i < MAX_LEVEL; i++){
        for(int8_t k = 0; k < LEARNABLE_MOVES_AMOUNT_PER_LEVEL; k++){
            mon->level_up_table[i][k] = -1;
        }
    }

    cJSON* lvl_up_table = cJSON_GetObjectItem(entry, "level_up_table");
    if(!lvl_up_table) {
        printf("NO LVL UP TABLE\n");
        return 0;
    }

    slot = 0;
    for(int8_t i = 0; i < MAX_LEVEL; i++){
        char lvl[8];
        sprintf(lvl, "%d", i);

        cJSON* move_lvl = cJSON_GetObjectItem(lvl_up_table, lvl);
        cJSON* move_id = NULL;
        if(move_lvl){
            int8_t k = 0;
            cJSON_ArrayForEach(move_id, move_lvl){
                mon->level_up_table[i][k] = move_id->valueint;
                k++;
            };
        }
    }

    return 1;
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

/// \brief Loads the Unova Dex into and array in memory. Mons are indexed by ther id - 1.
/// \return 0 On failure, 1 on success
static uint8_t MonstersLoad() {
    const char* fpath = "data/unova_pokedex_indexed.json";

    char* monster_data = LoadFileToString(fpath);
    if(!monster_data) return 0;

    cJSON* jsonMons = cJSON_Parse(monster_data);
        cJSON* entry = NULL;

        cJSON_ArrayForEach(entry, jsonMons) {
        if(monster_count >= MAX_GAME_MONSTERS) break;

        // Monsters are to be indexed by ther id
        // Makes the attributing them much easier and as the monster number is fixed it is relativelly safe
        monster_t* mon = &ALL_MONSTERS[monster_count];
        MonsterParseJSON(entry, mon);
        monster_count++;
    }
    cJSON_Delete(jsonMons);
    free(monster_data);
    printf("Loaded %d monsters.\n", monster_count);

    return 1;
}

void MonstersInit() {
    srand(time(NULL));

    SDL_Surface* surf = IMG_Load(FRONT_MON_SHEET);
    if(!surf) {
        printf("monster.c, MonstersInit: IMG_LOAD ERROR: %s\n", IMG_GetError());
    }
    if ( !(asset_manager->mon_front_sheet = SDL_CreateTextureFromSurface(rend, surf)) ) {
        printf("monster.c, MonstersInit: SDL_CreateTextureFromSurface ERROR: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(surf);

    SDL_Surface* surf2 = IMG_Load(BACK_MON_SHEET);
    if(!surf2) {
        printf("monster.c, MonstersInit: IMG_LOAD ERROR: %s\n", IMG_GetError());
    }
    if ( !(asset_manager->mon_back_sheet = SDL_CreateTextureFromSurface(rend, surf2)) ) {
        printf("monster.c, MonstersInit: SDL_CreateTextureFromSurface ERROR: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(surf2);

    printf("MON Front Sheet Loaded!\n");

    // LOAD ALL TYPE ADVANTAGES INTO THE LIBRARY
    // Initialize all values to 1 as the default mult
    for(uint8_t i = 0; i < TYPE_COUNT; i++){
        for(uint8_t k = 0; k < TYPE_COUNT; k++){
            TypeChart[i][k] = 1.0f;
        }
    }

    char* type_data = LoadFileToString("data/type_chart.json");
    if(type_data) {
        cJSON* json_types = cJSON_Parse(type_data);
        cJSON* entry = NULL;

        cJSON_ArrayForEach(entry, json_types){
            char* atck = cJSON_GetObjectItem(entry, "attacker")->valuestring;
            char* def  = cJSON_GetObjectItem(entry, "defender")->valuestring;
            double mult = cJSON_GetObjectItem(entry, "mult")->valuedouble;
            
            MonsterTypes atck_type = MonsterGetTypeFromString(atck);
            MonsterTypes def_type  = MonsterGetTypeFromString(def);

            if(atck_type != NONE_TYPE && def_type != NONE_TYPE) 
                TypeChart[atck_type][def_type] = (float) mult;
        }

        cJSON_Delete(json_types);
        free(type_data);
    }

    printf("Loaded TYPE CHART\n");

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
        printf("Loaded %d moves.\n", MoveLibraryCount);
    }

    printf("\n\n ------ LOADING FROM UNOVA DEX ------ \n\n");
    MonstersLoad();
}

// MONSTER SPAWNING LOGIC

int8_t CheckMonsterCanSpawn(int8_t tile_type){
    switch (tile_type)
    {
    case SPAWNABLE_TALL_GRASS:
        return 1;
        break;

    case SPAWNABLE_ROCK_GROUND:
        return 1;
        break;
    
    default:
        return 0;
        break;
    }
}

void MonsterResetBattleStats(monster_t* monster){
    if(!monster) return;
    monster->atk_stage = 0;
    monster->def_stage = 0;
    monster->spd_stage = 0;
}

void MonsterSetStats(monster_t* monster){
    if(monster->level == 0) monster->level = 5;

    monster_t* base = GetMonsterById(monster->id);
    if (!base) return;

    // Formula: Base * ((Level + 10) / 30) + Random Variance (IV simulation)
    float level_mult = ((float)monster->level + 10.0f) / 30.0f;

    monster->max_hp = (int)(base->max_hp * level_mult) + 10 + (rand() % 3);
    monster->hp = monster->max_hp;

    monster->attack = (int)(base->attack * level_mult) + (rand() % 2);
    monster->defense = (int)(base->defense * level_mult) + (rand() % 2);
    monster->speed = (int)(base->speed * level_mult) + (rand() % 2);

    // Quadratic growth: 10 * Level^2 + 50 * Level
    monster->max_xp = (monster->level * monster->level * 10) + (monster->level * 50);

    MonsterResetBattleStats(monster);
}

// Takes in a template monster and sets the spawned monster similar to the player's average level
static int8_t MonsterSetSpawnLevel(int8_t avg_player_level){
    int8_t spawn_level = avg_player_level;
    int8_t rng = rand() % 100;

    if(avg_player_level < 10){
        if(rng < 80){
            spawn_level += (rand() % 3) - 1; // -1 to +1 levels
        } else {
            spawn_level += (rand() % 3) + 1; // +1 to +3 levels
        }
    } else {
        if(rng < 60){
            spawn_level += (rand() % 5) - 2; // -2 to +2 levels
        } else if(rng < 90){
            spawn_level += (rand() % 4) + 2; // +2 to +5 levels
        } else {
            spawn_level += (rand() % 6) + 5; // +5 to +10 levels
        }
    }
    if(spawn_level < 1) spawn_level = 1;
    if(spawn_level > 100) spawn_level = 100;

    return spawn_level;
}

monster_t SpawnMonster(int8_t tile_type, int8_t avg_player_level){
    printf("MONSTER SPAWNED\n");
    
    char* tiles_file_data = LoadFileToString("data/tile_data.json");
    if (!tiles_file_data) return ALL_MONSTERS[0];

    // Array to store the id of all the spawnable monsters on this tile
    int16_t spawnable_mons_ids[150];
    int16_t count = 0;

    cJSON* tiles_json = cJSON_Parse(tiles_file_data);
    if (!tiles_json) {
        free(tiles_file_data);
        return ALL_MONSTERS[0];
    }
    cJSON* tile_data = NULL; 

    cJSON_ArrayForEach(tile_data, tiles_json){

        cJSON* t_item = cJSON_GetObjectItem(tile_data, "tile_type");
        if (!t_item) continue;

        int8_t tile_type_json = t_item->valueint;
        if(tile_type_json == tile_type){
            // Load the spawnable monsters to the array
            cJSON* spawnable_mons = cJSON_GetObjectItem(tile_data, "spawnable_monsters");
            cJSON* mon_id = NULL;

            // Iterate through all items from JSON array
            cJSON_ArrayForEach(mon_id, spawnable_mons){
                if (count < 150) {
                    spawnable_mons_ids[count] = mon_id->valueint;
                    count++;
                }
            }
        }

    }

    if (count == 0) {
        cJSON_Delete(tiles_json);
        free(tiles_file_data);
        return ALL_MONSTERS[0];
    }

    //=================================================================
    //= ALL SPAWN POOLS MUST HAVE AT LEAST ONE MONSTER OF EACH RARITY =
    //=================================================================

    // This number will represent the rarity of the spawned monster
    int8_t spawned_rarity = rand() % (100 + 1);
    printf("RARITY: %d\n", spawned_rarity);
    if(spawned_rarity < 70){
        // COMMON SPAWN
        // Infinite loop to try to keep spawning
        while(1){
            int16_t random_id = rand() % (count + 1);
            int16_t rndm_m_id = spawnable_mons_ids[random_id];

            monster_t* monster = GetMonsterById(rndm_m_id);
            if(monster && monster->rarity == COMMON){
                count = 0;
                free(tiles_file_data);
                cJSON_Delete(tiles_json);

                monster_t return_mons = *monster;
                
                return_mons.level = MonsterSetSpawnLevel(avg_player_level);
                MonsterSetStats(&return_mons);

                return return_mons;
            }
        }
    }
    else if(spawned_rarity < 90){
        // UNCOMMON SPAWN
        // Infinite loop to try to keep spawning
        while(1){
            int16_t random_id = rand() % (count + 1);
            int16_t rndm_m_id = spawnable_mons_ids[random_id];

            monster_t* monster = GetMonsterById(rndm_m_id);
            if(monster && monster->rarity == UNCOMMON){
                count = 0;
                free(tiles_file_data);
                cJSON_Delete(tiles_json);

                monster_t return_mons = *monster;
                
                return_mons.level = MonsterSetSpawnLevel(avg_player_level);
                MonsterSetStats(&return_mons);

                return return_mons;
            }
        }
    }
    else if (spawned_rarity < 98){
        // RARE SPAWN
        // Infinite loop to try to keep spawning
        while(1){
            int16_t random_id = rand() % (count + 1);
            int16_t rndm_m_id = spawnable_mons_ids[random_id];

            monster_t* monster = GetMonsterById(rndm_m_id);
            if(monster && monster->rarity == RARE){
                count = 0;
                free(tiles_file_data);
                cJSON_Delete(tiles_json);

                monster_t return_mons = *monster;
                
                return_mons.level = MonsterSetSpawnLevel(avg_player_level);
                MonsterSetStats(&return_mons);
                return return_mons;
            }
        }
    }
    else {
        // VERY RARE SPAWN (Mostly evolutions of starters or starters themselves)
        // Infinite loop to try to keep spawning
        while(1){
            int16_t random_id = rand() % (count + 1);
            int16_t rndm_m_id = spawnable_mons_ids[random_id];

            monster_t* monster = GetMonsterById(rndm_m_id);
            if(monster && monster->rarity == VERY_RARE){
                count = 0;
                free(tiles_file_data);
                cJSON_Delete(tiles_json);

                monster_t return_mons = *monster;
                
                return_mons.level = MonsterSetSpawnLevel(avg_player_level);
                MonsterSetStats(&return_mons);
                return return_mons;
            }
        }
    }

    return ALL_MONSTERS[0];
}

void MonsterUpdateAggro(player_t* player, Uint32 dt){
    if(player->aggro_timer > 0){
        player->aggro_timer -= dt;
    } else {
        if(notif_sound) {
            Mix_FreeMusic(notif_sound);
            notif_sound = NULL;
        }
        
        BattleInit(player, player->aggro_monster, NULL);
        free(player->aggro_monster);
        player->aggro_monster = NULL;
    }
}

int8_t TrySpawnMonster(player_t* player, map_t* map){            
    int32_t current_x = player->x_pos / 32;
    int32_t current_y = player->y_pos / 32;

    int8_t new_tile_type = GetCurrentTileType(current_x, current_y, map);

    if( (old_x != current_x || old_y != current_y) && CheckMonsterCanSpawn(new_tile_type)){
        printf("NEW TILE TYPE: %d\n", new_tile_type);

        // Every time player changes tile 20% chance to spawn
        int8_t spawn_num = rand() % (100 + 1);

        if(spawn_num <= 20){
            int16_t total_level = 0;
            int8_t count = 0;
            for(int8_t i = 0; i < 5; i++){
                if(player->monster_party[i]){
                    total_level += player->monster_party[i]->level;
                    count++;
                }
            }
            int8_t avg_level = (count > 0) ? total_level / count : 5;

            printf("AVG LEVEL DONE\n");

            monster_t* spawned_mons = (monster_t*) malloc(sizeof(monster_t));

            if(!notif_sound) notif_sound = Mix_LoadMUS(NOTIF_SOUND_LOC);
            Mix_PlayMusic(notif_sound, 1);

            *spawned_mons = SpawnMonster(new_tile_type, avg_level);
            player->aggro_timer = PLAYER_AGGRO_TIMER;
            player->aggro_monster = spawned_mons;
            player->game_state = STATE_AGGRO;
            return 1;
        }
    }

    // Set the current_tile value to the new one
    old_x = current_x;
    old_y = current_y; 
    return -1;
}

// Updates the monster's stats to reflect the level up
static void MonsterLevelUpStats(monster_t* monster){
    monster_t* base = GetMonsterById(monster->id);
    if (!base) return;

    // Growth formula: (Base Stat / 50) + 1 + Random Variance
    // This ensures monsters with higher base stats grow faster

    // TODO : Learn new moves based on level
    
    int8_t hp_gain = (base->max_hp / 50) + 1 + (rand() % 3);
    monster->max_hp += hp_gain;
    monster->hp = monster->max_hp;

    int8_t atk_gain = (base->attack / 50) + 1 + (rand() % 2);
    monster->attack += atk_gain;

    int8_t def_gain = (base->defense / 50) + 1 + (rand() % 2);
    monster->defense += def_gain;

    int8_t spd_gain = (base->speed / 50) + 1 + (rand() % 2);
    monster->speed += spd_gain;

    printf("%s leveled up to %d! (+%d HP, +%d Atk, +%d Def, +%d Spd)\n", 
           monster->name, monster->level, hp_gain, atk_gain, def_gain, spd_gain);
}

// Called when the monster's level coincides with it's evo1 level
// Changes the monster to a new copy of it's evolution whilst increasing it's stats significantly
// Keeps it's moves as well
// Returns the new monster struct
static monster_t* MonsterEvolve(monster_t* monster){
    
    // The next monster in the evolution line should be the one with monster->id + 1
    monster_t* evo_monster = GetMonsterById(monster->id + 1);

    // monster's applicable stats will be replaced by the evo monster's
    monster->id = evo_monster->id;

    strcpy(monster->name, evo_monster->name);
    strcpy(monster->description, evo_monster->description);

    monster->rarity = evo_monster->rarity;
    monster->types[0] = evo_monster->types[0];
    monster->types[1] = evo_monster->types[1];

    monster->evo_table[0][0] = evo_monster->evo_table[0][0];
    monster->evo_table[0][1] = evo_monster->evo_table[0][1];

    // TODO : Check for evolution unlocked moves
    for(int8_t i = 0; i < LEARNABLE_MOVES_AMOUNT_PER_LEVEL; i++){
        printf("%d", monster->level_up_table[monster->level][i]);
        if(monster->level_up_table[monster->level][i] == -1) break;

        printf("STARTING LEARN MOVE...\n");
        move_t* move_to_learn = GetMoveById(monster->level_up_table[monster->level][i]);
        BattleSetupLearnMove(monster, move_to_learn);
    }

    return monster;
}

int32_t MonsterGetExpYield(monster_t* defeated_monster, monster_t* player_monster){
    float multiplier = 1.0f;

    if(defeated_monster->level > player_monster->level){
        multiplier += (defeated_monster->level - player_monster->level) * 0.25f;
    }

    // Quadratic formula to scale with the XP requirement curve
    // (Level^2 / 2) + 10*Level + 10
    int32_t base_yield = (defeated_monster->level * defeated_monster->level / 2) + (defeated_monster->level * 10) + 100;
    
    // Bonus for rarity (Common=0, Uncommon=1, etc.)
    // Adds 20% per rarity tier
    int32_t rarity_bonus = (base_yield * player_monster->rarity) / 5;
    
    return (int)((base_yield + rarity_bonus) * multiplier);
}

static void MonsterRestoreMoves(monster_t* monster){
    for(uint8_t i = 0; i < 4; i++){
        monster->moves[i].available_uses = monster->moves[i].max_uses;
    }
}


void MonsterAddExp(monster_t* monster, monster_t* defeated_monster, int32_t exp_amount){
    if(defeated_monster) monster->exp += MonsterGetExpYield(defeated_monster, monster);
    else monster->exp += exp_amount;

    while(monster->exp >= monster->max_xp){
        if(monster->level >= 100){
            monster->exp = 0;
            break;
        }

        monster->exp -= monster->max_xp;
        monster->level++;

        // Update the monster's stats to reflect the level up
        MonsterLevelUpStats(monster);
        MonsterRestoreMoves(monster);
        monster->current_sfx = NONE;

        if(monster->level >= monster->evo_table[0][1]) monster = MonsterEvolve(monster);

        // Quadratic growth: 10 * Level^2 + 50 * Level
        monster->max_xp = (monster->level * monster->level * 10) + (monster->level * 50);

        // Check for new moves upon leveling up ig
        for(int8_t i = 0; i < LEARNABLE_MOVES_AMOUNT_PER_LEVEL; i++){
            if(monster->level_up_table[monster->level][i] == -1) break;

            printf("STARTING LEARN MOVE...\n");
            move_t* move_to_learn = GetMoveById(monster->level_up_table[monster->level][i]);
            BattleSetupLearnMove(monster, move_to_learn);
        }
    }
}

int8_t MonsterTryCatch(player_t* player, monster_t* monster, catch_device_t* device){

    // Initially set the catch_rate to the default
    float catch_rate = BASE_CATCH_RATE;

    float level_debuff = catch_rate * monster->level/25;
    float remaining_hp_buff = catch_rate * (monster->max_hp - monster->hp)/100;

    // Catch chance will be affected by the monster's level and remaining hp
    // Base chance will be 25%
    // First the level debuff and remaining hp buff

    catch_rate = catch_rate - level_debuff + remaining_hp_buff;
    catch_rate *= device->catch_rate_mult;

    int8_t rnd_catch = rand() % (100 + 1);
    
    if(rnd_catch <= catch_rate || catch_rate >= 100){
        PlayerAddMonsterToParty(monster);
        printf("Caught %s!\n", monster->name);
        return 1;
    }

    // Monster was not caught
    return 0;
}

float MonsterGetTypeEffectiveness(MonsterTypes attacker, MonsterTypes defender){
    if(attacker >= TYPE_COUNT || defender >= TYPE_COUNT) return 1.0f;
    return TypeChart[attacker][defender];
}

static void MonsterAddStatusFx(monster_t* m, StatusEffects status_fx){
    m->current_sfx = status_fx;
    m->status_fx_durantion = 3;
}

void MonsterRemoveStatusFx(monster_t* m){
    m->current_sfx = NONE;
}

char* MonsterGetSFXString(monster_t* m){
    switch(m->current_sfx){
    case SCORCHED:
        return "SCORCHED";
        break;
    case POISON:
        return "POISON";
        break;
    case STUNNED:
        return "STUNNED";
        break;
    case ASLEEP:
        return "ASLEEP";
        break;
    case FROZEN:
        return "FROZEN";
        break;
    case CORRODED:
        return "CORRODED";
        break;
    default:
        return "NONE";
    }
}

int8_t MonsterCheckCanMove(monster_t* m, char* msg){
    if(m->current_sfx == NONE) return 1;
    
    m->status_fx_durantion--;
    if(m->status_fx_durantion <= 0){
        m->current_sfx = NONE;
        sprintf(msg, "%s recovered.", m->name);
        return 1;
    }

    switch(m->current_sfx){
        case STUNNED:
            sprintf(msg, "%s is stunned!", m->name);
            return 0;
        case ASLEEP:
            sprintf(msg, "%s is asleep!", m->name);
            return 0;
        case FROZEN:
            sprintf(msg, "%s is frozen!", m->name);
            return 0;
        default:
            return 1;
    }
}

int8_t MonsterApplyStatusDamage(monster_t* m, char* msg){
    if(m->current_sfx == NONE) return 0;
    
    float dmg = 0.0f;
    switch(m->current_sfx){
        case SCORCHED:
            dmg = (float) m->max_hp * 0.0625;
            m->hp -= (int) dmg;
            if(m->hp < 0) m->hp = 0;
            sprintf(msg, "%s is hurt by the burn!", m->name);
            return 1;
        case POISON:
            dmg = (float) m->max_hp * 0.125;
            m->hp -= (int) dmg;
            if(m->hp < 0) m->hp = 0;
            sprintf(msg, "%s is hurt by poison!", m->name);
            return 1;
        case CORRODED:
            float debuff = (float) m->speed * 0.0625;
            m->speed -= (int) debuff;
            sprintf(msg, "%s is corroded!", m->name);
            return 1;
        default:
            return 0;
    }
}

int8_t MonsterUseMoveOn(monster_t* attacker, move_t* move, monster_t* attacked, char* return_msg){
    move->available_uses--;
    sprintf(return_msg, "%s used %s!", attacker->name, move->move_name);

    int8_t move_hit = rand() % 100;
    if(move->acc_percent != 100 && move_hit >= move->acc_percent){
        strcat(return_msg, " But missed!");
        printf("%s\n", return_msg);
        return 0;
    }


    // Calculate Damage
    // Formula: ((((2 * Level / 5 + 2) * Power * Attack / Defense) / 20) + 2) * Modifier
    float level = (float)attacker->level;
    float power = (float)move->damage;
    float attack = (float)attacker->attack;
    float defense = (float)attacked->defense;

    // Apply Stages
    float atk_mult = 1.0f;
    if(attacker->atk_stage > 0) 
        atk_mult = (2.0f + attacker->atk_stage) / 2.0f;
    else if(attacker->atk_stage < 0) 
        atk_mult = 2.0f / (2.0f - attacker->atk_stage);
    attack *= atk_mult;

    float def_mult = 1.0f;
    if(attacked->def_stage > 0) 
        def_mult = (2.0f + attacked->def_stage) / 2.0f;
    else if(attacked->def_stage < 0) 
        def_mult = 2.0f / (2.0f - attacked->def_stage);
    defense *= def_mult;
    
    if (defense == 0) defense = 1.0f;

    float base_damage = ((((2.0f * level / 5.0f + 2.0f) * power * (attack / defense)) / 50.0f) + 2.0f);
    float modifier = 1.0f;

    // Get type effectiveness mult
    float type_effectiveness = MonsterGetTypeEffectiveness(move->attack_type, attacked->types[0]);
    if (attacked->types[1] != NONE_TYPE) {
        printf("Effective against type 2\n");
        type_effectiveness *= MonsterGetTypeEffectiveness(move->attack_type, attacked->types[1]);
    }
    modifier *= type_effectiveness;

    // If attack and monster are of the same type apply a bonus
    if (move->attack_type == attacker->types[0] || move->attack_type == attacker->types[1]) {
        modifier *= 1.5f;
    }

    // RNG on attack as to not have just static attack values
    float random_var = (float)((rand() % 16) + 85) / 100.0f;
    modifier *= random_var;

    // Chance to crit 
    if ((rand() % 16) == 0) {
        modifier *= 2.0f;
        printf("Critical Hit!\n");
    }

    if(move->damage > 0){
        int16_t final_damage = (int)(base_damage * modifier);
        if (final_damage < 1) final_damage = 1;

        // Apply Damage
        attacked->hp -= final_damage;
        if(attacked->hp < 0) attacked->hp = 0;
        printf("%s used %s! Damage: %d (Eff: %.2fx)\n", attacker->name, move->move_name, final_damage, type_effectiveness);

        if(type_effectiveness > 1.0f){
            strcat(return_msg, " It's super effective!");
        }
        else if(type_effectiveness < 1.0f){
            strcat(return_msg, " It's not very effective!");
        }
    }


    if(move->status_effect > 0){
        int8_t rng = rand() % 3 + 1;
        if(rng == 1){
            MonsterAddStatusFx(attacked, move->status_effect);
            printf("STATUS FX APPLIED: %d\n", move->status_effect);
        }
    }

    // Apply Stat Changes
    if(move->stat_to_modify != STAT_NONE){
        monster_t* target = move->is_modify_self ? attacker : attacked;
        int8_t* stage = NULL;
        char* stat_name = NULL;

        switch(move->stat_to_modify){
            case STAT_ATTACK:
                stage = &target->atk_stage;
                stat_name = "Attack";
                break;
            case STAT_DEFENSE:
                stage = &target->def_stage;
                stat_name = "Defense";
                break;
            case STAT_SPEED:
                stage = &target->spd_stage;
                stat_name = "Speed";
                break;
            default: break;
        }

        if(stage){
            int8_t change = move->stat_stage_change;
            char stat_msg[512];
            // Initialize to empty string to prevent gibberish if change is 0
            stat_msg[0] = '\0';

            if(change > 0){
                if(*stage < 3){
                    *stage += change;
                    if(*stage > 3) *stage = 3;
                    sprintf(stat_msg, " %s's %s rose!", target->name, stat_name);
                } else {
                    sprintf(stat_msg, " %s's %s won't go higher!", target->name, stat_name);
                }
            } else if(change < 0){
                if(*stage > -3){
                    *stage += change;
                    if(*stage < -3) *stage = -3;
                    sprintf(stat_msg, " %s's %s fell!", target->name, stat_name);
                } else {
                    sprintf(stat_msg, " %s's %s won't go lower!", target->name, stat_name);
                }
            }
            strcat(return_msg, stat_msg);
        }
    }

    return 1;
}

int8_t MonsterHeal(monster_t* monster, uint16_t heal_amount){
    // Has fainted so does not heal
    if(monster->hp <= 0) return 0;

    monster->hp += heal_amount;
    if(monster->hp > monster->max_hp) monster->hp = monster->max_hp;
    return 1;
}

move_t* MonsterChooseEnemyAttack(monster_t* enemy){
    int32_t rnd = rand() % 4;
    return &enemy->moves[rnd];
}