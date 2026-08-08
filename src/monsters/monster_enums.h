#ifndef __MONSTER_ENUMS__
#define __MONSTER_ENUMS__

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

#endif // !__MONSTER_ENUMS__
