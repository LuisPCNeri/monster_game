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

typedef enum BattleState{
    MAIN_MENU,
    MOVES_MENU,
    SWITCH_MENU,
    EXECUTING_TURN,
    INV_OPEN,
    MESSAGE_DISPLAYED,
    MONSTER_CAUGHT,
    TRAINER_SWITCH,
    LEARN_MOVE,
    LEARN_MOVE_MENU,
    BATTLE_END_MSG
} BattleState;

// This function puts the player in the battle state
// Renders a new texture on top of the existing one (the map)
// Locks the player position and let's the player change selected buttons with arrow keys
// The player monster is not an arg as it will always be the first one in the party
void BattleInit(player_t* player, monster_t* enemy_monster, trainer_t* trainer);

// Renders the battle interface. Should be called every frame while in battle state.
void BattleDraw(Uint32 dt);

void BattleMenuBack();

/*
    Somewhat of a helper function to be called from the MonsterEvolve function. It changes the battle state to LEARN_MOVE and sets the monster to
    learn a move to the monster that just leveled up.

    \param monster A pointer to the monster that just leveld up.
*/
void BattleSetupLearnMove(monster_t* monster, move_t* move_to_learn);

// Cleans up resources used by the battle system
void BattleQuit(void);

#endif