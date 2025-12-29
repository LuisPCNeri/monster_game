#ifndef __MONSTER_H__
#define __MONSTER_H__

// MONSTER RARITIES
// These affect only the monster's spawning rate
typedef enum{
    COMMON,
    UNCOMMON,
    RARE,
    LEGENDARY
} Rarities;

// STATUS EFFECTS
typedef enum {
    NONE,
    // Takes damage every turn
    BURN,
    // Takes damage every turn
    POISON,
    // Has a chance to be unnable to move every turn
    PARALYZED,
    // Cannot move
    SLEEP,
    // Cannot move
    FROZEN,
    // Has chance to hit itself with move used
    CONFUSED
} StatusEffects;

// Monster with all it's data
typedef struct 
{   
    char monster_name[256];
    char monster_description[4096];

    // Monster's rarity affects only it's spawning chance
    // EXISTS ONLY IN THE NUMBERS DEFINED AS COMMON, UNCOMMON, RARE, LEGENDARY
    int rarity;

    // A monster can have only 2 types

    // Monster's main type
    int type_1;
    // Monster's secondary type
    int type_2;
    
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

    // The current status (debuff) applied to the monster
    // AVAILABLE VALUES DEFINED HERE AS BURN, POISON, ...
    // To specify no status applied, set current_status_fx to 0
    int current_status_fx;

    // Just here to keep track of what moves the monster can use in battle
    // When a move is learned one of these 4 moves will be changed
    // Moves the monster can currently use in battle
    move_t usable_moves[4];

    // Moves the monster can learn
    learnable_moves_list_t learnable_moves;

} monster_t;

// A move that can be used by a monster
typedef struct{
    // The level at which a monster can learn this move
    // If it is 0 the monster can always learn it
    int required_level;

    char move_name[256];
    char move_description[4096];

    // Moves can only have one type
    int attack_type;

    // Max amount of times move can be use => PP
    int max_PP;
    // Amount of times move can still be used
    int current_PP;

    // Percentage of times move will hit enemy
    int acc_percent;
    // Amount of damage enemy will take (can be 0)
    int damage;

    // The status effect IS a function that apllies x effect to the pokemon it was used on
    void* status_effect;
} move_t;

// A list of all moves a single monster can learn
typedef struct{

} learnable_moves_list_t;

// Initializes all the monster's data
// Reads from files with monster's data and add's it to a universal array for the monsters
// This array will contain ALL monsters and their information
void MonstersInit();

// Returns 1 if a monster can spawn and 0 if not
// The monster spawning or not depends on the tile_type
int CheckMonsterCanSpawn(int tile_type);

// "Spawns" a monster that immediatly tries to fight the player
// Takes in an int representing the tile_type to choose the monster's type
// Returns a pointer to the monster's data
monster_t* SpawnMonster(int tile_type);

// To be used in a separate thread, this function is always running
// Every time the player changes tiles it checks the tile_type
// If monsters can spawn in that tile it tries to spawn one
// void* arg is passed when creating the thread that will run this code and should be a pointer to the player "object"
void TrySpawnMonster(void* arg);

// Uses MOVE move on the enemy monster
// Only to be used during a batle
void UseMoveOn(move_t move, monster_t enemy_monster);

#endif