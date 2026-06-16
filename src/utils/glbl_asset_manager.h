#ifndef __GLBL_ASSET_MANAGER__
#define __GLBL_ASSET_MANAGER__

#include <SDL2/SDL.h>

typedef struct glbl_asset_manager
{
    SDL_Texture* mon_front_sheet;
    SDL_Texture* mon_back_sheet;
    
} glbl_asset_manager;

extern glbl_asset_manager* asset_manager;

#endif