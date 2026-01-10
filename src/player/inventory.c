#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "items/item.h"
#include "inventory.h"

// IMPORTANT : Add function logic here

// Helper struct to safely check item ID and Type
// Assumes all item structs in the game start with 'int id' and 'int type'
typedef struct {
    int id;
    int type;
} item_header_t;

inventory_t* InventoryCreateEmpty(int size){
    
    inventory_t* inv = (inventory_t*) malloc(sizeof(inventory_t));
    inv->item_count = size;
    inv->items = (inventory_item_t*) malloc(sizeof(inventory_item_t) * size);

    for(int i = 0; i < size; i++){
        inv->items[i].item = NULL;
        inv->items[i].id = -1;
        inv->items[i].type = -1;
        inv->items[i].count = 0;
        if(i > 0) inv->items[i].prev_item = &inv->items[i - 1];
        if(i < size - 1) inv->items[i].next_item = &inv->items[i + 1];
    }

    return inv;
}

inventory_item_t* InventorySearch(inventory_t* inv, void* item){
    if (!inv || !item) return NULL;
    
    // Cast to header to get ID safely (assuming common memory layout)
    int search_id = ((item_header_t*)item)->id;

    for(int i = 0; i < inv->item_count; i++){
        // Now we can check the ID directly without dereferencing the void* item
        if(inv->items[i].id == search_id && inv->items[i].count > 0) return &inv->items[i];
    } 
    return NULL;
}

void InventoryAddItem(inventory_t* inv, void* item){
    if (!inv || !item) return;

    item_header_t* header = (item_header_t*)item;
    
    // Check if item already exists to stack it
    inventory_item_t* existing = InventorySearch(inv, item);
    if(existing){
        existing->count++;
        return;
    }

    // Find first empty slot
    for(int i = 0; i < inv->item_count; i++){
        if(inv->items[i].count == 0){
            inv->items[i].item = item;
            inv->items[i].id = header->id;
            inv->items[i].type = header->type;
            inv->items[i].count = 1;
            return;
        }
    }
    printf("Inventory is full!\n");
}

void InventoryRemoveItem(inventory_t* inv, void* item){
    if (!inv || !item) return;

    item_header_t* header = (item_header_t*)item;
    
    // Check if item already exists to stack it
    inventory_item_t* existing = InventorySearch(inv, item);
    if(existing){
        existing->count--;
        return;
    }

    // The item does not exist in the inventory
    return;
}

void InventoryDestroy(inventory_t* inv){
    
}

void InventoryDraw(){

}