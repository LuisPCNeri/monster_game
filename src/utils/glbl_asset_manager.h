#ifndef __GLBL_ASSET_MANAGER__
#define __GLBL_ASSET_MANAGER__

#include <SDL2/SDL.h>

#define MON_FRONT_SPRITE_SIZE 96
#define SHEET_VERT_PADDING 7
#define SHEET_COLS 8

typedef struct glbl_asset_manager
{
    SDL_Texture* mon_front_sheet;
    SDL_Texture* mon_back_sheet;
    
} glbl_asset_manager;

extern glbl_asset_manager* asset_manager;

#endif
