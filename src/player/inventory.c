#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "items/item.h"
#include "menus/menu.h"
#include "inventory.h"

#define MAX_ITEM_TYPE_AMOUNT 15

// Helper struct to safely check item ID and Type
// Assumes all item structs in the game start with 'int id' and 'int type'
typedef struct {
    int id;
    int type;
} item_header_t;

extern SDL_Renderer* rend;
extern TTF_Font* game_font;
extern int screen_w;
extern int screen_h;

static TTF_Font* info_font;

inventory_t* InventoryCreateEmpty(){
    
    inventory_t* inv = (inventory_t*) malloc(sizeof(inventory_t));
    inv->item_count = 0;

    inv->head = NULL;
    inv->current = NULL;
    inv->tail = NULL;

    if(!info_font) info_font = TTF_OpenFont("resources/fonts/8bitOperatorPlus8-Regular.ttf", 24);
    
    SDL_Color arrow_color = {255, 255, 255, 255};
    SDL_Surface* arrow_surf = TTF_RenderText_Solid(info_font, "->", arrow_color);
    inv->arrow_texture = SDL_CreateTextureFromSurface(rend, arrow_surf);
    SDL_FreeSurface(arrow_surf);

    return inv;
}

inventory_item_t* InventorySearch(inventory_t* inv, void* item){
    if (!inv || !item) return NULL;
    
    // Cast to header to get ID safely (assuming common memory layout)
    int search_id = ((item_header_t*)item)->id;
    inventory_item_t* itm_ptr = inv->head;

    while(itm_ptr){
        if(itm_ptr->id == search_id) return itm_ptr;
        itm_ptr = itm_ptr->next_item;
    }
    
    return NULL;
}
void InventoryAddItem(inventory_t* inv, void* item, unsigned int count){
    if (!inv || !item) return;

    item_header_t* header = (item_header_t*)item;
    
    // Check if item already exists to stack it
    inventory_item_t* existing = InventorySearch(inv, item);
    if(existing){
        existing->count += count;
        return;
    }
    
    inventory_item_t* itm = (inventory_item_t*) malloc(sizeof(inventory_item_t));
    if(!inv->head){
        inv->head = itm;
        inv->tail = itm;
        inv->current = itm;
        itm->index = 0;
        itm->prev_item = NULL;
    }
    else {
        itm->index = inv->tail->index + 1;
        inv->tail->next_item = itm;
        itm->prev_item = inv->tail;
        inv->tail = itm;
    }

    itm->id = header->id;
    itm->type = header->type;
    itm->count = count;
    itm->item = item;
    itm->next_item = NULL;
    
    itm->menu_item.x = screen_w / 2 - 200;
    itm->menu_item.y = 200 + (itm->index * 50);
    itm->menu_item.w = 400;
    itm->menu_item.h = 50;
}

void InventoryRemoveItem(inventory_t* inv, void* item, unsigned int count){
    if (!inv || !item) return;
    
    // Check if item already exists to stack it
    inventory_item_t* existing = InventorySearch(inv, item);
    if(existing){
        if( existing->count <= count ){
            if(existing->id == inv->tail->id){
                inv->tail = inv->tail->prev_item;
                inv->tail->next_item = NULL;

                free(existing);
                return;
            }

            if(existing->id == inv->head->id){
                inv->head = inv->head->next_item;
                inv->head->prev_item = NULL;

                free(existing);
                return;
            }

            existing->prev_item->next_item = existing->next_item;
            existing->next_item->prev_item = existing->prev_item;
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
    inventory_item_t* tmp = inv->head;
    while(tmp->next_item){
        inventory_item_t* current = tmp;
        tmp = tmp->next_item;

        free(current);
    }

    if(inv->tail) free(inv->tail);

    free(inv);
    if(info_font) {
        TTF_CloseFont(info_font);
        info_font = NULL;
    }
}

void InventoryDraw(inventory_t* inv){
    SDL_Rect inv_box = {screen_w / 2 - 200, 200, 400, 800};
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

    inventory_item_t* itm = inv->head;
    while(itm){
        // Don't render items that do not exist
        if(itm->id != -1 && itm->count > 0) {
            SDL_Rect item_box = itm->menu_item;
            
            
            char* item_name = "Unknown";
            // Type 0 would be catch_device_t type so cast current item to it
            switch(itm->type){
                case 0:
                    catch_device_t* device = (catch_device_t*) itm->item;
                    item_name = device->name;
                    break;
                case 1:
                    restore_item_t* pot = (restore_item_t*) itm->item;
                    item_name = pot->name;
                    break;
                default:
                    break;
            }

            SDL_Color text_color = {255, 255, 255, 255};
            SDL_Surface* text_surface = TTF_RenderText_Solid(info_font, item_name, text_color);
            if(text_surface) {
                SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surface);
                SDL_FreeSurface(text_surface);

                char item_count[16];
                sprintf(item_count, "%d", itm->count);

                SDL_Surface* count_surface = TTF_RenderText_Solid(info_font, item_count, text_color);
                if(count_surface) {
                    SDL_Texture* count_texture = SDL_CreateTextureFromSurface(rend, count_surface);
                    SDL_FreeSurface(count_surface);

                    
                    int w, h;
                    SDL_QueryTexture(text_texture, NULL, NULL, &w, &h);
                    SDL_Rect text_rect = {item_box.x + 20, item_box.y + 18, w, h};

                    int count_w, count_h;
                    SDL_QueryTexture(count_texture, NULL, NULL, &count_w, &count_h);
                    SDL_Rect count_rect = {item_box.x + 370, item_box.y + 18, count_w, count_h};
                    
                    SDL_RenderCopy(rend, text_texture,  NULL, &text_rect);
                    SDL_RenderCopy(rend, count_texture, NULL, &count_rect);
                    SDL_RenderDrawRect(rend, &item_box);
                    SDL_DestroyTexture(count_texture);
                }
                SDL_DestroyTexture(text_texture);
            }
        }
        itm = itm->next_item;
    }

    int w,h;
    SDL_QueryTexture(inv->arrow_texture, NULL, NULL ,&w, &h);
    
    double time = (double) SDL_GetTicks()/1000;
    SDL_Rect arrow_rect = {
        inv->current->menu_item.x - w - 15 - 10*cos((double) time *3),
        inv->current->menu_item.y + inv->current->menu_item.h / 2 - h / 2,
        w, h
    };

    SDL_RenderCopy(rend, inv->arrow_texture, NULL, &arrow_rect);

    // Set Render color back to black PLEASE FFS DO NOT LET THIS BE CHANGED ME :pray:
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}

void InventoryMoveForward(inventory_t* inv){
    if(inv->current->next_item){
        inv->current = inv->current->next_item;
    }
    // Wrap around to the head
    else{
        inv->current = inv->head;
    }
}

void InventoryMoveBack(inventory_t* inv){
    if(inv->current->prev_item){
        inv->current = inv->current->prev_item;
    }
}

inventory_item_t* InventoryGetCurrent(inventory_t* inv){
    return inv->current;
}