#ifndef __MOVES__
#define __MOVES__

#include <stdint.h>
#include "items/item.h"
#include "monsters/monster_enums.h"
#include "libraries/cJSON.h"

typedef struct monster_t monster_t;

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

/*
 *  \brief Loads all moves into memory
 * */
void MovesInit();

/*
 *  \brief Creates an array with a random combination of 4 moves a monster with a specific level may have learned.
 *  No data is changed on monster. Caller is responsible for freeing the move_t array.
 *  \param monster The monster whose moves to define
 *  \returns An array of move_t objects or NULL. The array may not be exactly 4 moves.
 * */
int16_t* GetLearnedMovesIdPermutation(monster_t* monster);

/*
 *  \brief Parses JSON data from a move to m
 *  \param entry Pointer to the JSON data
 *  \param m Pointer to where move is loaded
 * */
void MoveParseJSON(cJSON* entry, move_t* m);

/*
 *  \brief Get a move by its id
 *  \param id The id of the monster
 *  \returns A pointer to the monster Template or NULL if it does not exist
 * */
move_t* GetMoveById(int16_t id);

#endif // !__MOVES__
