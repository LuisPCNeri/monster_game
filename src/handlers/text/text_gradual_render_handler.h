#ifndef __TEXT_GRADUAL_RENDER_HANDLER__
#define __TEXT_GRADUAL_RENDER_HANDLER__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>

/*
    \brief Renders text char by char. The text will be always rendered from the left to right starting at the top left corner of the rect.
    \param text The text to be rendered.
    \param dst A pointer to the Rect where the text will be rendered.
    \param font The font with which to render the text.
    \param delta_time The time passed since last call.
    \param renderer The current window renderer.
*/
void RenderTextByChar(const char* text, SDL_Rect* dst, TTF_Font* font, uint32_t delta_time, SDL_Renderer* renderer );

#endif