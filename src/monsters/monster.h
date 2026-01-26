#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "items/item.h"

// MONSTER RARITIES
// These affect only the monster's spawning rate
typedef enum Rarities{
    COMMON,
    UNCOMMON,
    RARE,
    VERY_RARE,
    LEGENDARY
} Rarities;

// STATUS EFFECTS
typedef enum StatusEffects{
    NONE,
    // Takes damage every turn
    SCORCHED,
    // Takes damage every turn
    POISON,
    // Has a chance to be unnable to move every turn
    STUNNED,
    // Cannot move
    ASLEEP,
    // Cannot move
    FROZEN,
    // Speed Debuff, damage debuff
    // Chance to happen when water interacts with metal
    CORRODED
} StatusEffects;

typedef enum StatType{
    STAT_NONE,
    STAT_ATTACK,
    STAT_DEFENSE,
    STAT_SPEED
} StatType;

typedef enum MonsterTypes{
    NONE_TYPE,
    FIRE_TYPE,
    WATER_TYPE,
    GRASS_TYPE,
    ROCK_TYPE,
    POISON_TYPE,
    ELECTRIC_TYPE,
    NORMAL_TYPE,
    DRAGON_TYPE,
    METAL_TYPE,
    DARK_TYPE,
    FLYING_TYPE,
    FIGHTING_TYPE,
    BUG_TYPE,
    ICE_TYPE,
    TYPE_COUNT
} MonsterTypes;

// A list of all moves a single monster can learn
typedef struct{

} learnable_moves_list_t;

// A move that can be used by a monster
typedef struct move_t{
    // States if this move's modifier applies to self or enemy
    int is_modify_self;
    // id to lookup the move
    int id;
    // The level at which a monster can learn this move
    // If it is 0 the monster can always learn it
    int required_level;
    // Max amount of times move can be use => PP
    int max_uses;
    // Amount of times move can still be used
    int available_uses;

    // Percentage of times move will hit enemy
    int acc_percent;
    // Amount of damage enemy will take (can be 0)
    int damage;
    
    StatType stat_to_modify;
    int stat_stage_change;

    // Moves can only have one type
    MonsterTypes attack_type;
    // Status effect the move may apply on hit
    StatusEffects status_effect;

    char move_name[256];
    char move_description[4096];
} move_t;

// Monster with all it's data
typedef struct monster_t {      
    // id to lookup the monster
    int id;

    // Current monster level
    int level;

    int status_fx_durantion;

    // Level at which monster will evolve for the first time
    // If monster has no evolution set to -1
    int evo_1_level;
    // Level at which monster will evolve for the second time
    // If monster has no second evolution set to -1
    int evo_2_level;

    // Max amount of hitpoints for monster
    int max_hp;
    // Current amount of hitpoints monster has
    int current_hp;

    // Amount of attack points monster has
    int attack;
    // Amount of defense points monster has
    int defense;
    // Amount of speed points monster has
    int speed;
    
    int atk_stage;
    int def_stage;
    int spd_stage;

    int current_exp;
    int exp_to_next_level;

    // Monster's main type
    MonsterTypes type_1;
    // Monster's secondary type
    MonsterTypes type_2;
    // Monster's rarity affects only it's spawning chance
    // EXISTS ONLY IN THE NUMBERS DEFINED AS COMMON, UNCOMMON, RARE, VERY_RARE, LEGENDARY
    Rarities rarity;
    // The current status (debuff) applied to the monster
    // AVAILABLE VALUES DEFINED HERE AS SCORCHED, POISON, ...
    // To specify no status applied, set current_status_fx to NONE
    StatusEffects current_status_fx;

    // Just here to keep track of what moves the monster can use in battle
    // When a move is learned one of these 4 moves will be changed
    // Moves the monster can currently use in battle
    move_t usable_moves[4];

    // Moves the monster can learn
    learnable_moves_list_t learnable_moves;

    char monster_name[256];
    // Path to the monter's sprite
    char sprite_path[256];
    char monster_description[4096];
} monster_t;

// Initializes all the monster's data
// Reads from json file with monster's data and add's it to a universal array for the monsters
// This array will contain ALL monsters and their information
void MonstersInit();

// Returns 1 if a monster can spawn and 0 if not
// The monster spawning or not depends on the tile_type
int CheckMonsterCanSpawn(int tile_type);

// "Spawns" a monster that immediatly tries to fight the player
// Takes in an int representing the tile_type to choose the monster's type
// Returns a pointer to the monster's data
monster_t SpawnMonster(int tile_type, int avg_player_level);

// To be used in a separate thread, this function is always running
// Every time the player changes tiles it checks the tile_type
// If monsters can spawn in that tile it tries to spawn one
// void* arg is passed when creating the thread that will run this code and should be a pointer to the player "object"
int TrySpawnMonster(void* arg);

// Returns the float multiplier for the corresponding effectiveness of attacker's attack type on the defender's type
float MonsterGetTypeEffectiveness(MonsterTypes attacker, MonsterTypes defender);

// Uses MOVE move on the enemy monster
// Only to be used during a batle
// attacker uses move on attacked 
int MonsterUseMoveOn(monster_t* attacker, move_t* move, monster_t* attacked, char* return_msg);

// Checks if the monster can move this turn (handles Sleep, Freeze, etc.)
int MonsterCheckCanMove(monster_t* m, char* msg);

// Applies status damage (Poison, Burn) and returns 1 if damage was taken
int MonsterApplyStatusDamage(monster_t* m, char* msg);

// Return char* correspondant to the status effect applied on monster
char* MonsterGetSFXString(monster_t* m);

// Has the enemy monster choose a move to use on the player monster and then attack the player's monster
move_t* MonsterChooseEnemyAttack(monster_t* enemy);

// Prints a monsters data to the terminal
void MonsterPrint(monster_t* monster);

// With a given id returns a pointer to the move on the global moves array
// To use this please create a local copy of the struct
move_t* GetMoveById(int id); 

// With a given id returns a pointer to the monster on the global monsters array
// To use this please create a local copy of the struct
monster_t* GetMonsterById(int id);

// Increments monster_t monster's exp by an amount calculated in another helper function
// This amount has a linear realtion to the enemy_monster's level
void MonsterAddExp(monster_t* monster, monster_t* enemy_monster);

// Has the player try to catch a monster.
// Uses the rarity modifiers to calculate the chance.
// If it hits the monster will go to the player's party. If the party is full goes to the storage.
int MonsterTryCatch(player_t* player, monster_t* monster, catch_device_t* device);

// Sets the stats of a monster to have some rng
// Used when a monster spawns and when choosing a starter
// All other stat increments are done with MonsterAddExp
void MonsterSetStats(monster_t* monster);

// Resets the temporary battle stats (stages) to 0
void MonsterResetBattleStats(monster_t* monster);

// Heal a monster through moves, items or idk anything else
// Returns 1 if the monster was healed 0 if not
int MonsterHeal(monster_t* monster, unsigned int heal_amount);

// Handles the enter key input for the battle menu
void BattleMenuHandleSelect();

#endif