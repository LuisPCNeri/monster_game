#ifndef __INVENTORY_H__
#define __INVENTORY_H__

// A struct to make the inventory a linked list
typedef struct inventory_item_t{
    int id;
    int type;

    // As the item can be a capture device, healing item, or any other item type
    // it shall be considered a void*
    void* item;
    unsigned int count;
    // Function associated to the use of the item
    void (*item_use)();

    struct inventory_item_t* next_item;
    struct inventory_item_t* prev_item;

} inventory_item_t;

// Struct to keep track of the items the player has
// It is a Doubly Linked List that CANNOT have repeated items
// So a Doubly Linked Set or something like that
typedef struct{
    int item_count;

    struct menu_t* menu;
    // Array for all the items the player has
    inventory_item_t* items;

} inventory_t;

// Creates an empty Doubly Linked Inventory of item_count = size
// All elements of items are initialized to NULL
inventory_t* InventoryCreateEmpty(int size);
// Adds an item to the inventory
// If the item already exists in the inventory increases it's count
void InventoryAddItem(inventory_t* inv, void* item, int count);
// Removes an item of the inventory
// If the item already exists decreases it's count
void InventoryRemoveItem(inventory_t* inv, void* item, int count);

// Searches the inventory struct for an item that matches void* item
// Returns a pointer to the found item or NULL if it finds nothing
inventory_item_t* InventorySearch(inventory_t* inv, void* item);

void InventoryDestroy(inventory_t* inv);

void InventoryDraw();

#endif