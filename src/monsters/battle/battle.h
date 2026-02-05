#ifndef __BATTLE_H__
#define __BATTLE_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "player/player.h"
#include "trainers/trainer.h"
typedef enum BattleMenuButtons{
    ATTACK,
    INVENTORY,
    SWITCH,
    RUN
} BattleMenuButtons;

// When defeating a trainer's monster the exp to add gets saved with the monster that defeated that monster
typedef struct BattleQueueItem{
    int exp_amount;
    monster_t* monster;
} BattleQueueItem;

// This function puts the player in the battle state
// Renders a new texture on top of the existing one (the map)
// Locks the player position and let's the player change selected buttons with arrow keys
// The player monster is not an arg as it will always be the first one in the party
void BattleInit(player_t* player, monster_t* enemy_monster, trainer_t* trainer);

// Renders the battle interface. Should be called every frame while in battle state.
void BattleDraw();

void BattleMenuBack();

// Cleans up resources used by the battle system
void BattleQuit(void);

#endif