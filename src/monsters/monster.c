#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "libraries/cJSON.h"

#include "map.h"
#include "player/player.h"
#include "items/item.h"
#include "monster.h"
#include "battle/battle.h"

#define MAX_GAME_MOVES 500
#define MAX_GAME_MONSTERS 150

#define BASE_CATCH_RATE 25

// GLOBAL array for this file that has IN MEMORY all the monsters (indexed by their order)
static monster_t ALL_MONSTERS[MAX_GAME_MONSTERS];
static int monster_count = 0;

static move_t ALL_MOVES[MAX_GAME_MOVES];
static int MoveLibraryCount = 0;

static float TypeChart[TYPE_COUNT][TYPE_COUNT];

move_t* GetMoveById(int id) {
    for(int i = 0; i < MoveLibraryCount; i++) {
        if(ALL_MOVES[i].id == id) {
            return &ALL_MOVES[i];
        }
    }
    return NULL;
}

monster_t* GetMonsterById(int id){
    for(int i=0; i < monster_count; i++){
        if(ALL_MONSTERS[i].id == id) return &ALL_MONSTERS[i];
    }
    return NULL;
}

// Takes in the path to a file and reads it to a string
// Returns a pointer to the string or NULL if it fails
// The caller is responsible for freeing the string
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

void MonsterPrint(monster_t* m){
    printf(
        "ID: %d\r\n"
        "SPRITE PATH: %s\r\n"
        "NAME: %s\r\n"
        "DESCRIPTION: %s\r\n"
        "RARITY: %d\r\n"
        "LEVEL: %d\r\n"
        "EVO 1: %d\r\n"
        "EVO 2: %d\r\n"
        "TYPE 1: %d\r\n"
        "TYPE 2: %d\r\n"
        "HP: %d\r\n"
        "ATTACK: %d\r\n"
        "DEFENSE: %d\r\n"
        "SPEED: %d\r\n"
        "MOVE 1: %s\r\n"
        "MOVE 2: %s\r\n\n\n",
        m->id, m->sprite_path, m->monster_name, m->monster_description, m->rarity, m->level, m->evo_1_level, m->evo_2_level, m->type_1, m->type_2,
        m->current_hp, m->attack, m->defense, m->speed, m->usable_moves[0].move_name, m->usable_moves[1].move_name
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

void MonstersInit() {

    // Set srand once only
    srand(time(NULL));

    // =========================================================
    // LOAD ALL TYPE ADVANTAGES INTO THE LIBRARY
    // =========================================================

    // Initialize all values to 1 as the default mult
    for(unsigned int i = 0; i < TYPE_COUNT; i++){
        for(unsigned int k = 0; k < TYPE_COUNT; k++){
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

    // =========================================================
    // LOAD ALL MOVES INTO THE LIBRARY
    // =========================================================
    char* moveData = LoadFileToString("data/moves.json");
    if(moveData) {
        cJSON* jsonMoves = cJSON_Parse(moveData);
        cJSON* entry = NULL;

        cJSON_ArrayForEach(entry, jsonMoves) {
            if(MoveLibraryCount >= MAX_GAME_MOVES) break;
            
            move_t* m = &ALL_MOVES[MoveLibraryCount];
            
            // Load basic data
            m->id = cJSON_GetObjectItem(entry, "id")->valueint;
            strcpy(m->move_name, cJSON_GetObjectItem(entry, "name")->valuestring);
            m->required_level = cJSON_GetObjectItem(entry, "req_level")->valueint;
            m->damage = cJSON_GetObjectItem(entry, "power")->valueint;
            m->max_uses = cJSON_GetObjectItem(entry, "max_pp")->valueint;

            char* type = cJSON_GetObjectItem(entry, "type")->valuestring;
            m->attack_type = MonsterGetTypeFromString(type);

            cJSON* acc_item = cJSON_GetObjectItem(entry, "accuracy");
            m->acc_percent = acc_item ? acc_item->valueint : 100;
            
            // Initialize current state
            m->available_uses = m->max_uses; 

            MoveLibraryCount++;
        }
        cJSON_Delete(jsonMoves);
        free(moveData);
        printf("Loaded %d moves.\n", MoveLibraryCount);
    }

    // =========================================================
    // LOAD MONSTERS AND LINK MOVES
    // =========================================================
    char* monData = LoadFileToString("data/monster.json");
    if(monData) {
        cJSON* jsonMons = cJSON_Parse(monData);
        cJSON* entry = NULL;

        cJSON_ArrayForEach(entry, jsonMons) {
            if(monster_count >= MAX_GAME_MONSTERS) break;

            // Monsters are to be indexed by ther id
            // Makes the attributing them much easier and as the monster number is fixed it is relativelly safe
            monster_t* mon = &ALL_MONSTERS[monster_count];

            // Load Basic Info
            mon->id = cJSON_GetObjectItem(entry, "id")->valueint;
            strcpy(mon->sprite_path, cJSON_GetObjectItem(entry, "sprite")->valuestring);
            strcpy(mon->monster_name, cJSON_GetObjectItem(entry, "name")->valuestring);
            strcpy(mon->monster_description, cJSON_GetObjectItem(entry, "description")->valuestring);

            // Load Stats
            cJSON* stats = cJSON_GetObjectItem(entry, "stats");
            mon->max_hp = cJSON_GetObjectItem(stats, "hp")->valueint;
            mon->current_hp = mon->max_hp;

            mon->rarity = cJSON_GetObjectItem(entry, "rarity")->valueint;

            mon->level = cJSON_GetObjectItem(entry, "level")->valueint;
            mon->evo_1_level = cJSON_GetObjectItem(entry, "evo_1")->valueint;
            mon->evo_2_level = cJSON_GetObjectItem(entry, "evo_2")->valueint;

            char* type_1 = cJSON_GetObjectItem(entry, "type_1")->valuestring;
            mon->type_1 = MonsterGetTypeFromString(type_1);
            char* type_2 = cJSON_GetObjectItem(entry, "type_2")->valuestring;;
            mon->type_2 = MonsterGetTypeFromString(type_2);

            mon->attack = cJSON_GetObjectItem(stats, "atk")->valueint;
            mon->defense = cJSON_GetObjectItem(stats, "def")->valueint;
            mon->speed = cJSON_GetObjectItem(stats, "spd")->valueint;

            mon->current_exp = 0;
            
            // Load the USABLE MOVES for this monster
            cJSON* movesArray = cJSON_GetObjectItem(entry, "starting_moves");
            cJSON* moveIdVal = NULL;
            int slot = 0;

            // Initialize slots to empty values first
            for(int k=0; k<4; k++) {
                // -1 FOR EMPTY SLOT
                mon->usable_moves[k].id = -1;
                mon->usable_moves[k].damage = 0;
            }

            // Loop through the JSON array
            cJSON_ArrayForEach(moveIdVal, movesArray) {
                if(slot >= 4) break; // Cannot have more than 4 moves

                int idToFind = moveIdVal->valueint;
                
                // Find the move definition in our library
                move_t* foundMove = GetMoveById(idToFind);

                if(foundMove != NULL) {
                    // COPY the struct into the monster's slot
                    mon->usable_moves[slot] = *foundMove; 
                    
                    // Reset dynamic variables for this specific monster
                    mon->usable_moves[slot].available_uses = foundMove->max_uses;
                    
                    slot++;
                } else {
                    printf("Warning: Monster %s tries to use unknown Move ID %d\n", mon->monster_name, idToFind);
                }
            }

            monster_count++;
        }
        cJSON_Delete(jsonMons);
        free(monData);
        printf("Loaded %d monsters.\n", monster_count);
    }
}

// MONSTER SPAWNING LOGIC

int CheckMonsterCanSpawn(int tile_type){
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

void MonsterSetStats(monster_t* monster){
    if(monster->level == 0) monster->level = 5;

    // Retrieve base stats to calculate initial stats based on level
    monster_t* base = GetMonsterById(monster->id);
    if (!base) return;

    // Formula: Base * ((Level + 10) / 30) + Random Variance (IV simulation)
    float level_mult = ((float)monster->level + 10.0f) / 30.0f;

    monster->max_hp = (int)(base->max_hp * level_mult) + 10 + (rand() % 3);
    monster->current_hp = monster->max_hp;

    monster->attack = (int)(base->attack * level_mult) + (rand() % 2);
    monster->defense = (int)(base->defense * level_mult) + (rand() % 2);
    monster->speed = (int)(base->speed * level_mult) + (rand() % 2);

    // Quadratic growth: 10 * Level^2 + 50 * Level
    monster->exp_to_next_level = (monster->level * monster->level * 10) + (monster->level * 50);
}

// Takes in a template monster and sets the spawned monster similar to the player's average level
static int MonsterSetSpawnLevel(int avg_player_level){
    int spawn_level = avg_player_level;
    int rng = rand() % 100;

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

monster_t SpawnMonster(int tile_type, int avg_player_level){
    printf("MONSTER SPAWNED\n");
    
    char* tiles_file_data = LoadFileToString("data/tile_data.json");
    if (!tiles_file_data) return ALL_MONSTERS[0];

    // Array to store the id of all the spawnable monsters on this tile
    int spawnable_mons_ids[150];
    int count = 0;

    cJSON* tiles_json = cJSON_Parse(tiles_file_data);
    if (!tiles_json) {
        free(tiles_file_data);
        return ALL_MONSTERS[0];
    }
    cJSON* tile_data = NULL; 

    cJSON_ArrayForEach(tile_data, tiles_json){

        cJSON* t_item = cJSON_GetObjectItem(tile_data, "tile_type");
        if (!t_item) continue;

        int tile_type_json = t_item->valueint;
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
    int spawned_rarity = rand() % (100 + 1);
    printf("RARITY: %d\n", spawned_rarity);
    if(spawned_rarity < 70){
        // COMMON SPAWN
        // Infinite loop to try to keep spawning
        while(1){
            int random_id = rand() % (count + 1);
            int rndm_m_id = spawnable_mons_ids[random_id];

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
            int random_id = rand() % (count + 1);
            int rndm_m_id = spawnable_mons_ids[random_id];

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
            int random_id = rand() % (count + 1);
            int rndm_m_id = spawnable_mons_ids[random_id];

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
            int random_id = rand() % (count + 1);
            int rndm_m_id = spawnable_mons_ids[random_id];

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

    // LEGENDARY SPAWN
    // Infinite loop to try to keep spawning
    // Legendaries will not spawn naturaly
    // so nothing here

    return ALL_MONSTERS[0];
}

int TrySpawnMonster(void* arg){
    // Convert the argument to a player pointer
    player_t* player = (player_t*) arg;
    
    // Player's current tile basically
    int old_x = player->x_pos / 32;
    int old_y = player->y_pos / 32;

    while(player->running){
        
        int current_x = player->x_pos / 32;
        int current_y = player->y_pos / 32;
        // Get the new tile
        int new_tile_type = GetCurrentTileType(current_x, current_y);

        if( (old_x != current_x || old_y != current_y) && CheckMonsterCanSpawn(new_tile_type)){
            printf("NEW TILE TYPE: %d\n", new_tile_type);

            // Every time player changes tile 40% chance to spawn
            int spawn_num = rand() % (100 + 1);

            if(spawn_num <= 20){
                // The chance hit spawn monster now
                int total_level = 0;
                int count = 0;
                for(int i = 0; i < 5; i++){
                    if(player->monster_party[i]){
                        total_level += player->monster_party[i]->level;
                        count++;
                    }
                }
                int avg_level = (count > 0) ? total_level / count : 5;

                printf("AVG LEVEL DONE\n");

                monster_t spawned_mons = SpawnMonster(new_tile_type, avg_level);
                MonsterPrint(&spawned_mons);

                // Monster has spawned so iniciate a battle
                BattleInit(player ,&spawned_mons);
            }
        }

        // Set the current_tile value to the new one
        old_x = current_x;
        old_y = current_y; 
        SDL_Delay(50);
    }
    return -1;
}

// Updates the monster's stats to reflect the level up
static void MonsterLevelUpStats(monster_t* monster){
    // Retrieve base stats to calculate growth
    monster_t* base = GetMonsterById(monster->id);
    if (!base) return;

    // Growth formula: (Base Stat / 50) + 1 + Random Variance
    // This ensures monsters with higher base stats grow faster

    // TODO : Learn new moves based on level
    
    int hp_gain = (base->max_hp / 50) + 1 + (rand() % 3);
    monster->max_hp += hp_gain;
    monster->current_hp = monster->max_hp;

    int atk_gain = (base->attack / 50) + 1 + (rand() % 2);
    monster->attack += atk_gain;

    int def_gain = (base->defense / 50) + 1 + (rand() % 2);
    monster->defense += def_gain;

    int spd_gain = (base->speed / 50) + 1 + (rand() % 2);
    monster->speed += spd_gain;

    printf("%s leveled up to %d! (+%d HP, +%d Atk, +%d Def, +%d Spd)\n", 
           monster->monster_name, monster->level, hp_gain, atk_gain, def_gain, spd_gain);
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

    strcpy(monster->monster_name, evo_monster->monster_name);
    strcpy(monster->monster_description, evo_monster->monster_description);
    strcpy(monster->sprite_path, evo_monster->sprite_path);

    monster->rarity = evo_monster->rarity;
    monster->type_1 = evo_monster->type_1;
    monster->type_2 = evo_monster->type_2;

    monster->evo_1_level = evo_monster->evo_1_level;
    monster->evo_2_level = evo_monster->evo_2_level;

    // TODO : Check for evolution unlocked moves

    return monster;
}

static int MonsterGetExpYield(monster_t* monster, float multiplier){
    if(multiplier <= 0) multiplier = 1.0;
    // Quadratic formula to scale with the XP requirement curve
    // (Level^2 / 2) + 10*Level + 10
    int base_yield = (monster->level * monster->level / 2) + (monster->level * 10) + 100;
    
    // Bonus for rarity (Common=0, Uncommon=1, etc.)
    // Adds 20% per rarity tier
    int rarity_bonus = (base_yield * monster->rarity) / 5;
    
    return (int)((base_yield + rarity_bonus) * multiplier);
}

static void MonsterRestoreMoves(monster_t* monster){
    for(unsigned int i = 0; i < 4; i++){
        monster->usable_moves[i].available_uses = monster->usable_moves[i].max_uses;
    }
}


void MonsterAddExp(monster_t* monster, monster_t* defeated_monster){
    float xp_mult = 1.0f;

    // Underdog bonus: 25% extra XP per level difference if enemy is higher level
    if(defeated_monster->level > monster->level){
        xp_mult += (defeated_monster->level - monster->level) * 0.25f;
    }

    // Update monster's current exp
    monster->current_exp += MonsterGetExpYield(defeated_monster, xp_mult);

    // While loop to keep leveling up whilst current_exp > exp to next level
    while(monster->current_exp >= monster->exp_to_next_level){
        if(monster->level >= 100){
            monster->current_exp = 0;
            break;
        }

        // Updates the current exp and updates the monster's level
        monster->current_exp -= monster->exp_to_next_level;
        monster->level++;

        // Update the monster's stats to reflect the level up
        MonsterLevelUpStats(monster);
        MonsterRestoreMoves(monster);

        // Clear the status effects
        monster->current_status_fx = NONE;

        if(monster->level >= monster->evo_1_level) monster = MonsterEvolve(monster);

        // Quadratic growth: 10 * Level^2 + 50 * Level
        monster->exp_to_next_level = (monster->level * monster->level * 10) + (monster->level * 50);
    }
}

int MonsterTryCatch(player_t* player, monster_t* monster, catch_device_t* device){

    // Initially set the catch_rate to the default
    float catch_rate = BASE_CATCH_RATE;

    float level_debuff = catch_rate * monster->level/25;
    float remaining_hp_buff = catch_rate * (monster->max_hp - monster->current_hp)/100;

    // Catch chance will be affected by the monster's level and remaining hp
    // Base chance will be 25%
    // First the level debuff and remaining hp buff

    catch_rate = catch_rate - level_debuff + remaining_hp_buff;
    catch_rate *= device->catch_rate_mult;

    int rnd_catch = rand() % (100 + 1);
    
    if(rnd_catch <= catch_rate || catch_rate >= 100){
        PlayerAddMonsterToParty(monster);
        printf("Caught %s!\n", monster->monster_name);
        return 1;
    }

    // Monster was not caught
    return 0;
}

float MonsterGetTypeEffectiveness(MonsterTypes attacker, MonsterTypes defender){
    if(attacker >= TYPE_COUNT || defender >= TYPE_COUNT) return 1.0f;
    return TypeChart[attacker][defender];
}

// For now it will just stay a simple move power/damage * monster attack / 100 (if it was 100 it'd be way too much damage)

void MonsterUseMoveOn(monster_t* attacker, move_t* move, monster_t* attacked){
    // Decrement available uses
    move->available_uses--;

    // Check Accuracy
    int move_hit = rand() % 100;
    if(move->acc_percent != 100 && move_hit >= move->acc_percent){
        printf("%s used %s but missed!\n", attacker->monster_name, move->move_name);
        return;
    }

    if(move->damage == 0){
        // TODO : Damage is 0 So most likely has some debuff/buff associated
        return;
    }

    // Calculate Damage
    // Formula: ((((2 * Level / 5 + 2) * Power * Attack / Defense) / 20) + 2) * Modifier
    float level = (float)attacker->level;
    float power = (float)move->damage;
    float attack = (float)attacker->attack;
    float defense = (float)attacked->defense;
    
    // Prevent division by zero
    if (defense == 0) defense = 1.0f;

    float base_damage = ((((2.0f * level / 5.0f + 2.0f) * power * (attack / defense)) / 50.0f) + 2.0f);

    // Modifiers
    float modifier = 1.0f;

    // Get type effectiveness mult
    float type_effectiveness = MonsterGetTypeEffectiveness(move->attack_type, attacked->type_1);
    if (attacked->type_2 != NONE_TYPE) {
        type_effectiveness *= MonsterGetTypeEffectiveness(move->attack_type, attacked->type_2);
    }
    modifier *= type_effectiveness;

    // If attack and monster are of the same type apply a bonus
    if (move->attack_type == attacker->type_1 || move->attack_type == attacker->type_2) {
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

    int final_damage = (int)(base_damage * modifier);
    if (final_damage < 1) final_damage = 1;

    // Apply Damage
    attacked->current_hp -= final_damage;
        if(attacked->current_hp < 0) attacked->current_hp = 0;

    printf("%s used %s! Damage: %d (Eff: %.2fx)\n", attacker->monster_name, move->move_name, final_damage, type_effectiveness);

    // TODO : take into account the status effect move can inflict
}

int MonsterHeal(monster_t* monster, unsigned int heal_amount){
    // Has fainted so does not heal
    if(monster->current_hp <= 0) return 0;

    monster->current_hp += heal_amount;
    if(monster->current_hp > monster->max_hp) monster->current_hp = monster->max_hp;
    return 1;
}

move_t* MonsterChooseEnemyAttack(monster_t* enemy){
    int rnd = rand() % 4;
    return &enemy->usable_moves[rnd];
}