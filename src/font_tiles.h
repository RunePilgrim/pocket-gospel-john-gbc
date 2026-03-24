/* font_tiles.h — declares the 96-tile font data array (ASCII 32-127). */

#ifndef FONT_TILES_H
#define FONT_TILES_H

#include <stdint.h>
#include <gbdk/platform.h>

#define font_tiles_TILE_COUNT 96
BANKREF_EXTERN(font_tiles)

extern const uint8_t font_tiles_tiles[1536];

#endif
