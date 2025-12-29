#include <stdio.h>
#include <stdlib.h>

#include "map.h"
#include "monster.h"

void MonstersInit(){

}

// MONSTER SPAWNING LOGIC

int CheckMonsterCanSpawn(int tile_type){
    switch (tile_type)
    {
    case SPAWNABLE_TALL_GRASS:
        return 1;
        break;
    
    default:
        return 0;
        break;
    }
}

monster_t* SpawnMonster(int tile_type){

}

void TrySpawnMonster(void* arg){

}