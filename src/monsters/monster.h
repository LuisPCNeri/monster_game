#ifndef __MONSTER_H__
#define __MONSTER_H__

#include <SDL2/SDL.h>
#include "items/item.h"
#include "libraries/cJSON.h"
#include "monsters/monster_enums.h"
#include "monsters/moves/moves.h"

#define MAX_LEVEL 100
#define LEARNABLE_MOVES_AMOUNT_PER_LEVEL 5

#define USBALE_MOVES_AMOUNT 4

typedef struct chunked_map_t chunked_map_t; 

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

/*
    \brief Checks if a monster can spawn in the players current tile, if so the monster is spawned.
    \param player The current player.
    \param map The current chunked_map_t obj.
*/
void MonsterTrySpawn(player_t* player, chunked_map_t* map);

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
    \param msg Pointer to set the message saying the monster learned a move if necessary.
*/
void MonsterAddExp(monster_t* monster, monster_t* enemy_monster, int32_t exp, char* msg);

int32_t MonsterGetExpYield(monster_t* defeated_monster, monster_t* player_monster);

/*
 *  \brief Based on rarity, level difference and remaining hp calculates if a mon was caught or not.
 *  On success immediatly adds the mon to the current active player party, if no space it adds to the players PC.
 *  \param monster The monster the current player is trying to catch.
 *  \param device The catching device used for trying to capture the mon.
 *  \returns 1 on successful atempt and 0 on failed atempt.
 * */
int8_t MonsterTryCatch(monster_t* monster, catch_device_t* device);

/*
 *  \brief Sets the moves of a monster randomly based on the moves it has learned up to its level.
 *  \param m The monster whose moves will be set
 * */
void MonsterSetMoves(monster_t* m);

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
