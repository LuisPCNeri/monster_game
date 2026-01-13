// TODO : Trainer AI will be worked on with a weight system
// Trainers may have special traits as CAUTIOUS, AGGRESSIVE, ...
// These traits should change the way the moves are weighted for that trainer

// More expirienced trainers ought to have a better weight system as in
// probably switch monsters if their current one is not the best for the current enemy
// availability to use items
// and in general a weight system that would result in a MUCH harder battle
#include <stdio.h>

#include "monsters/monster.h"

static move_t* enemy_last_move = NULL;

move_t* TrainerUseMove(monster_t* player_monster, monster_t* enemy){
    // Wild monsters have no notion of type advantages
    // What I think is more plausible for them is to choose the attack that does the highest damage
    // If they attacked once and the other monster was immune THEY LEARN
    // and so they will not use that attack again agains that monster
    
    move_t* highest_dmg = NULL;

    for(unsigned int i = 0; i < 4; i++){
        // Check if last move was inneficient
        if(enemy_last_move && enemy_last_move->id == enemy->usable_moves[i].id){
            float type_mult = MonsterGetTypeEffectiveness(enemy->usable_moves[i].attack_type, player_monster->type_1);

            if(player_monster->type_2 != NONE_TYPE){
                if(MonsterGetTypeEffectiveness(enemy->usable_moves[i].attack_type, player_monster->type_2) > type_mult)
                    type_mult = MonsterGetTypeEffectiveness(enemy->usable_moves[i].attack_type, player_monster->type_2);
            }
            
            // Do not let this move be used next
            if(type_mult < 1.0f) continue;
        }

        if(!highest_dmg) highest_dmg = &enemy->usable_moves[i];

        if(enemy->usable_moves[i].damage > highest_dmg->damage) 
            highest_dmg = &enemy->usable_moves[i];
    }
    
    return highest_dmg;
}