#ifndef __MONSTER_DEX_H__
#define __MONSTER_DEX_H__

#define HORIZONTAL_MARGIN 250
#define VERTICAL_MARGIN 250

/*
    This draws the monster dex window for the current monster_t* monster. The usage of this function with offset_x is intended for the move learn replacement
    screen. It makes it easier to draw the menu and have space for the move to be learned to the right of the menu.

    \param monster A pointer to the monster which screen user wants to open
    \param screen_w Width of player's screen
    \param screen_h Height of player's screen
    \param offset_x Horizontal offset for the whole window
*/
void DexDrawMonsterInfo(monster_t* monster, int screen_w, int screen_h, int offset_x);

#endif