#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "items/item.h"
#include "menus/menu.h"
#include "inventory.h"

#define MAX_ITEM_TYPE_AMOUNT 15

// IMPORTANT : Add function logic here

// Helper struct to safely check item ID and Type
// Assumes all item structs in the game start with 'int id' and 'int type'
typedef struct {
    int id;
    int type;
} item_header_t;

extern SDL_Renderer* rend;
extern TTF_Font* game_font;
static TTF_Font* info_font;

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

    info_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", 24);
    inv->menu = MenuCreate(MAX_ITEM_TYPE_AMOUNT, 1, 0, InventoryDraw, NULL);

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

void InventoryAddItem(inventory_t* inv, void* item, int count){
    if (!inv || !item) return;

    item_header_t* header = (item_header_t*)item;
    
    // Check if item already exists to stack it
    inventory_item_t* existing = InventorySearch(inv, item);
    if(existing){
        existing->count += count;
        return;
    }

    // Find first empty slot
    for(int i = 0; i < inv->item_count; i++){
        if(inv->items[i].count == 0){
            inv->items[i].item = item;
            inv->items[i].id = header->id;
            inv->items[i].type = header->type;
            inv->items[i].count = count;
            return;
        }
    }
    printf("Inventory is full!\n");
}

void InventoryRemoveItem(inventory_t* inv, void* item, int count){
    if (!inv || !item) return;

    item_header_t* header = (item_header_t*)item;
    
    // Check if item already exists to stack it
    inventory_item_t* existing = InventorySearch(inv, item);
    if(existing){
        if( existing->count <= count ){
            // Erase the item
            inventory_item_t* prev = existing->prev_item;

            prev->next_item = existing->next_item;
            existing->next_item->prev_item = prev;

            free(existing);
            return;
        }

        existing->count -= count;
        return;
    }

    // The item does not exist in the inventory
    printf("Somehow someway this item does not exist.\n");
    return;
}

void InventoryDestroy(inventory_t* inv){
    MenuDestroy(inv->menu);
    free(inv->items);
    free(inv);
}

void InventoryDraw(inventory_t* inv){
    SDL_Rect inv_box = {1450, 200, 400, 800};
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderDrawRect(rend, &inv_box);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    inv_box.x++;
    inv_box.y++;
    inv_box.h -= 2;
    inv_box.w -= 2;
    SDL_RenderFillRect(rend, &inv_box);

    // Set the color to white for the item boxes
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

    for(int i = 0; i < inv->item_count; i++){
        // Don't render items that do not exist
        if(inv->items[i].id == -1 || inv->items[i].count == 0) continue;

        SDL_Rect item_box = {1450, 200 + (i*50), 400, 50};
        
        
        char* item_name = "Unknown";
        // Type 0 would be catch_device_t type so cast current item to it
        if(inv->items[i].type == 0) {
            catch_device_t* device = (catch_device_t*) inv->items[i].item;
            item_name = device->name;
        }

        SDL_Color text_color = {255, 255, 255, 255};
        SDL_Surface* text_surface = TTF_RenderText_Solid(info_font, item_name, text_color);
        if(!text_surface) continue;
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surface);
        SDL_FreeSurface(text_surface);

        char item_count[3];
        sprintf(item_count, "%d", inv->items[i].count);

        SDL_Surface* count_surface = TTF_RenderText_Solid(info_font, item_count, text_color);
        if(!count_surface) continue;
        SDL_Texture* count_texture = SDL_CreateTextureFromSurface(rend, count_surface);
        SDL_FreeSurface(count_surface);

        
        int w, h;
        SDL_QueryTexture(text_texture, NULL, NULL, &w, &h);
        SDL_Rect text_rect = {item_box.x + 10, item_box.y + 18, w, h};

        int count_w, count_h;
        SDL_QueryTexture(count_texture, NULL, NULL, &count_w, &count_h);
        SDL_Rect count_rect = {item_box.x + 370, item_box.y + 18, count_w, count_h};
        
        SDL_RenderCopy(rend, text_texture,  NULL, &text_rect);
        SDL_RenderCopy(rend, count_texture, NULL, &count_rect);
        SDL_RenderDrawRect(rend, &item_box);
        SDL_DestroyTexture(text_texture);
        SDL_DestroyTexture(count_texture);
    }

    // Set Render color back to black PLEASE FFS DO NOT LET THIS BE CHANGED ME :pray:
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}