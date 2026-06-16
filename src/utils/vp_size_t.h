#ifndef __VIEWPORT_SCALE_H__
#define __VIEWPORT_SCALE_H__

#include <stdint.h>

extern int32_t screen_h;
extern int32_t screen_w;

#define VH(percent) ((int32_t) ((percent) / 100) * screen_h)
#define VW(percent) ((int32_t) ((percent) / 100) * screen_w)

#endif