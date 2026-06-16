#ifndef __MONSTER_H__
#define __MONSTER_H__

#include <SDL2/SDL.h>
#include "items/item.h"
#include "libraries/cJSON.h"

#define MAX_LEVEL 100
#define LEARNABLE_MOVES_AMOUNT_PER_LEVEL 5

#define USBALE_MOVES_AMOUNT 4

typedef struct map_t map_t;

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

// A move that can be used by a monster
typedef struct move_t{
    char* move_description;
    char* move_name;
    StatType stat_to_modify;
    // Moves can only have one type
    MonsterTypes attack_type;
    // Status effect the move may apply on hit
    StatusEffects status_effect;
    // id to lookup the move
    int16_t id;
    int16_t damage;
    // The level at which a monster can learn this move
    // If it is 0 the monster can always learn it
    int16_t required_level;
    // Max amount of times move can be use => PP
    int8_t max_uses;
    // Amount of times move can still be used
    int8_t available_uses;
    // Percentage of times move will hit enemy
    int8_t acc_percent;
    // Amount of damage enemy will take (can be 0)
    // States if this move's modifier applies to self or enemy
    int8_t is_modify_self;
    int8_t stat_stage_change;
} move_t;

// Monster with all it's data
typedef struct monster_t {
    char* name;
    char* description;

    move_t moves[4];

    MonsterTypes types[2];
    Rarities rarity;

    StatusEffects current_sfx;

    int16_t level_up_table[MAX_LEVEL][LEARNABLE_MOVES_AMOUNT_PER_LEVEL];

    int16_t exp;
    int16_t max_xp;

    int16_t id;
    int16_t sprite_idx;
    
    int16_t max_hp;
    int16_t hp;

    int16_t attack;
    int16_t sp_atk;

    int16_t defense;
    int16_t sp_def;

    int16_t speed;

    /// Generally any given mon has 2 or less evos
    uint8_t evo_table[2][2];

    int8_t level;
    int8_t status_fx_durantion;
    
    int8_t atk_stage;
    int8_t def_stage;
    int8_t spd_stage;
} monster_t;

// Initializes all the monster's data
// Reads from json file with monster's data and add's it to a universal array for the monsters
// This array will contain ALL monsters and their information
void MonstersInit();

// Parses the JSON Entry with the monster data and puts it in the address of mon
uint8_t MonsterParseJSON(cJSON* entry, monster_t* mon);

// Parses the JSON Entry with the move data and puts it in the address of m 
void MoveParseJSON(cJSON* entry, move_t* m);

// Returns 1 if a monster can spawn and 0 if not
// The monster spawning or not depends on the tile_type
int8_t CheckMonsterCanSpawn(int8_t tile_type);

// "Spawns" a monster that immediatly tries to fight the player
// Takes in an int representing the tile_type to choose the monster's type
// Returns a pointer to the monster's data
monster_t SpawnMonster(int8_t tile_type, int8_t avg_player_level);

// Every time the player changes tiles it checks the tile_type
// If monsters can spawn in that tile it tries to spawn one
int8_t TrySpawnMonster(player_t* player, map_t* map);

// Returns the float multiplier for the corresponding effectiveness of attacker's attack type on the defender's type
float MonsterGetTypeEffectiveness(MonsterTypes attacker, MonsterTypes defender);

// Returns MonsterType correspondant to the char type_name
MonsterTypes MonsterGetTypeFromString(const char* type_name);
 
/*
    Uses MOVE move on the enemy monster, only to be used during a batle, attacker uses move on attacked
    \param *attacker Monster who used the attack
    \param *move move used by attacker
    \param *attacked monster who is targeted by the attack
    \param *return_msg pointer to a char that will store the message correspondant to the attack.
*/
int8_t MonsterUseMoveOn(monster_t* attacker, move_t* move, monster_t* attacked, char* return_msg);

// Checks if the monster can move this turn (handles Sleep, Freeze, etc.)
int8_t MonsterCheckCanMove(monster_t* m, char* msg);

// Applies status damage (Poison, Burn) and returns 1 if damage was taken
int8_t MonsterApplyStatusDamage(monster_t* m, char* msg);

// Return char* correspondant to the status effect applied on monster
char* MonsterGetSFXString(monster_t* m);

/*
    Given a monster returns the attack that monster will use in battle. The caller is responsible for the returned move_t*
    \param *enemy Monster whose moves will be considered when choosing what move to use
*/
move_t* MonsterChooseEnemyAttack(monster_t* enemy);

// Prints a monsters data to the terminal
void MonsterPrint(monster_t* monster);

/*
    Returns a pointer to the base template of the move with a given id.
    To make changes to this monster or create a new instance of it create a copy of the struct.
    DO NOT ALTER THE VALUES IN THE STRUCT GIVE BY THE POINTER

    \param id Id of the move to get
*/
move_t* GetMoveById(int16_t id); 

/*
    Returns a pointer to the base template of the monster with a given id.
    To make changes to this monster or create a new instance of it create a copy of the struct.
    DO NOT ALTER THE VALUES IN THE STRUCT GIVE BY THE POINTER

    \param id Id of the monster to get
*/
monster_t* GetMonsterById(int16_t id);

/*
    Increments the monsters' current exp by either the amount provided by enemy_monster or in exp.

    \param *monster Monster that will receive the exp.
    \param *enemy_monster Either a pointer to the defeated monster that will be used for exp calculations or NULL to use the int exp parameter.
    \param exp Amount of exp to add to monster. Will only be used if enemy_monster is NULL.
*/
void MonsterAddExp(monster_t* monster, monster_t* enemy_monster, int32_t exp);

int32_t MonsterGetExpYield(monster_t* defeated_monster, monster_t* player_monster);

// Has the player try to catch a monster.
// Uses the rarity modifiers to calculate the chance.
// If it hits the monster will go to the player's party. If the party is full goes to the storage.
int8_t MonsterTryCatch(player_t* player, monster_t* monster, catch_device_t* device);

// Sets the stats of a monster to have some rng
// Used when a monster spawns and when choosing a starter
// All other stat increments are done with MonsterAddExp
void MonsterSetStats(monster_t* monster);

// Resets the temporary battle stats (stages) to 0
void MonsterResetBattleStats(monster_t* monster);

// Heal a monster through moves, items or idk anything else
// Returns 1 if the monster was healed 0 if not
int8_t MonsterHeal(monster_t* monster, uint16_t heal_amount);

void MonsterUpdateAggro(player_t* player, Uint32 dt);

// Handles the enter key input for the battle menu
void BattleMenuHandleSelect();

/// \brief Gets the SRC rect for a sprite in a sprite sheet
/// \param sprite_sheet A pointer to the sprite sheet texture
/// \param sprite_size Size of the sides of the SQUARE sprite
/// \param vertical_padding The amount of vertical padding between rows in px
/// \param horizontal_padding The amount of horizontal padding between cols in px
SDL_Rect GetFromSpriteSheet(uint16_t sprite_size, uint16_t vertical_padding, uint16_t horizontal_padding, uint8_t col_num, uint16_t idx);

#endif