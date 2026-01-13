#ifndef __ITEM_H__
#define __ITEM_H__

// Struct to keep all of the catching device properties
// These will be loaded/created when reading the items.json file
typedef struct catch_device_t{
    int id;
    int type;
    // Name of the catching device
    char name[128];

    // Path to the device's sprite file
    char sprite_path[256];

    // Multiplier for the catch rate associated with this device.
    int catch_rate_mult;
} catch_device_t;

typedef struct restore_item_t{
    int id;
    int type;
    // Name of the catching device
    char name[128];

    // Path to the device's sprite file
    char sprite_path[256];

    int restore_amount;
} restore_item_t;

#endif