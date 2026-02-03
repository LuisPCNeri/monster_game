#ifndef __TRAINER_BATTLE_H__
#define __TRAINER_BATTLE_H__

// A wrapper for BattleInit with some small diferences to adhere better to the Trainer battle requirements
void TrainerBattleInit();
void TrainerBattleDraw();
move_t* TrainerBattleChooseMove(monster_t* player_monster, monster_t* enemy);

#endif