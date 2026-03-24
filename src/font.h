/* font.h — font loading and text drawing. */

#ifndef FONT_H
#define FONT_H

#include <gb/gb.h>

#define FONT_BASE_TILE 2u

void font_load(void);
void draw_text(UINT8 x, UINT8 y, const char *s);

#endif
