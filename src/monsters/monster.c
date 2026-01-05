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
monster_t ALL_MONSTERS[MAX_GAME_MONSTERS];
int monster_count = 0;
move_t ALL_MOVES[MAX_GAME_MOVES];
int MoveLibraryCount = 0;

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
    fread(buffer, 1, file_size, fptr);
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
        "EVO 2: %d\r\n"
        "TYPE 1: %d\r\n"
        "TYPE 2: %d\r\n"
        "HP: %d\r\n"
        "ATTACK: %d\r\n"
        "DEFENSE: %d\r\n"
        "SPEED: %d\r\n"
        "MOVE 1: %s\r\n"
        "MOVE 2: %s\r\n\n\n",
        m->id, m->monster_name, m->monster_description, m->rarity, m->level, m->evo_1_level, m->evo_2_level, m->type_1, m->type_2,
        m->current_hp, m->attack, m->defense, m->speed, m->usable_moves[0].move_name, m->usable_moves[1].move_name
    );
}

void MonstersInit() {

    // Set srand once only
    srand(time(NULL));
    // =========================================================
    // LOAD ALL MOVES INTO THE LIBRARY
    // =========================================================
    char* moveData = LoadFileToString("src/monsters/data/moves.json");
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
            m->attack_type = cJSON_GetObjectItem(entry, "type")->valueint; // Assumes int in JSON
            
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
    char* monData = LoadFileToString("src/monsters/data/monster.json");
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

            mon->type_1 = cJSON_GetObjectItem(entry, "type_1")->valueint;
            mon->type_2 = cJSON_GetObjectItem(entry, "type_2")->valueint;

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
    // Has no evolutions so level should be made according
    if(monster->evo_1_level == 0){
        // Level will be between lvl 45 and 100 (very unfair know)
        monster->level = rand() % (100 + 1 - 45) + 45;
    }

    // Generate a random level based on evolution level and base level
    monster->level = rand() % (monster->evo_1_level - monster->level) + monster->level;
    if(monster->level == 0) monster->level = 5;

    // All base stats will fluctuate by 10 values from base
    monster->max_hp += rand() % (10 + 1);
    monster->current_hp = monster->max_hp;

    monster->attack += rand() % (10 + 1);
    monster->defense += rand() % (10 + 1);
    monster->speed += rand() % (10 + 1);

    // TODO Set stats according to level
}

monster_t SpawnMonster(int tile_type){
    printf("MONSTER SPAWNED\n");
    
    char* tiles_file_data = LoadFileToString("src/monsters/data/tile_data.json");
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
    if(spawned_rarity < 50){
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
                MonsterSetStats(&return_mons);

                return return_mons;
            }
        }
    }
    else if(spawned_rarity < 85){
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
                MonsterSetStats(&return_mons);

                return return_mons;
            }
        }
    }
    else{
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

void* TrySpawnMonster(void* arg){
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
                monster_t spawned_mons = SpawnMonster(new_tile_type);
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
    return NULL;
}

// Updates the monster's stats to reflect the level up
static void MonsterLevelUpStats(monster_t* monster){
    // Stats will increment higher in lower levels and start getting deminishing returns after
    // Increment will be by a random amount between 5-10 with the level affecting it maybe

    monster->max_hp = monster->max_hp + (rand() % (10 + 1 - 5) + 5) - monster->level / 15;
    // At level up monster's hp will be refilled
    monster->current_hp = monster->max_hp;

    monster->attack = monster->attack + (rand() % (10 + 1 - 5) + 5) - monster->level / 15;
    monster->defense = monster->defense + (rand() % (10 + 1 - 5) + 5) - monster->level / 15;
    monster->speed = monster->speed + (rand() % (10 + 1 - 5) + 5) - monster->level / 15;
}

// TODO EVOLVE
// Called when the monster's level coincides with it's evo1 level
// Changes the monster to a new copy of it's evolution whilst increasing it's stats significantly
// Keeps it's moves as well
// Returns the new monster struct
static monster_t* MonsterEvolve(monster_t* monster){
    return NULL;
}

void MonsterAddExp(monster_t* monster, int exp_amount){
    // Update monster's current exp
    monster->current_exp += exp_amount;

    // While loop to keep leveling up whilst current_exp > exp to next level
    while(monster->current_exp >= monster->exp_to_next_level){
        // Updates the current exp and updates the monster's level
        monster->current_exp -= monster->exp_to_next_level;
        monster->level++;

        // Update the monster's stats to reflect the level up
        MonsterLevelUpStats(monster);

        // Clear the status effects
        monster->current_status_fx = NONE;

        //TODO EVOLVE
        if(monster->level >= monster->evo_1_level) MonsterEvolve(monster);

        // FuckAss algorithm to set the exp for next level
        monster->exp_to_next_level = (1 / monster->level) + 250 * (10 / monster->level);
    }
}

monster_t* MonsterTryCatch(monster_t* monster, catch_device_t* device){

    // Initially set the catch_rate to the default
    float catch_rate = BASE_CATCH_RATE;

    float level_debuff = catch_rate * monster->level/100;
    float remaining_hp_buff = catch_rate * (monster->max_hp - monster->current_exp)/100;

    // Catch chance will be affected by the monster's level and remaining hp
    // Base chance will be 25%
    // First the level debuff and remaining hp buff

    catch_rate = catch_rate - level_debuff + remaining_hp_buff;
    catch_rate *= device->catch_rate_mult;

    // Guaranteed catch
    if(catch_rate > 100) return monster;

    int rnd_catch = rand() % (100 + 1);
    // Random number was inside the catch rate so it was caught
    if(rnd_catch <= catch_rate) return monster;

    // Monster was not caught
    return NULL;
}