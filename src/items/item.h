#ifndef __ITEM_H__
#define __ITEM_H__

typedef struct monster_t monster_t;
typedef struct player_t player_t;
// Struct to keep all of the catching device properties
// These will be loaded/created when reading the items.json file
typedef struct catch_device_t{
    int id;
    int type;
    // Multiplier for the catch rate associated with this device.
    int catch_rate_mult;

    // Name of the catching device
    char name[128];

    // Path to the device's sprite file
    char sprite_path[256];
} catch_device_t;

typedef struct restore_item_t{
    int id;
    int type;

    int restore_amount;

    // Name of the catching device
    char name[128];

    // Path to the device's sprite file
    char sprite_path[256];
} restore_item_t;

#endif