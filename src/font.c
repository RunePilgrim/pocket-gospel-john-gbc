/* font.c — loads font tiles and draws text to the BG map. */

#include <gb/gb.h>
#include <gbdk/platform.h>

#include "font.h"
#include "font_tiles.h"

void font_load(void) {
    UINT8 old_bank = _current_bank;

    /* font_tiles may land in a bank; BANK(font_tiles) resolves at link time */
    SWITCH_ROM_MBC1(BANK(font_tiles));
    set_bkg_data(FONT_BASE_TILE, font_tiles_TILE_COUNT, font_tiles_tiles);
    SWITCH_ROM_MBC1(old_bank);
}

void draw_text(UINT8 x, UINT8 y, const char *s) {
    unsigned char buf[20];
    UINT8 n = 0;

    while (s[n] && n < 20) {
        unsigned char c = (unsigned char)s[n];

        if (c == ' ') {
            buf[n] = 0;               // tile 0 = blank background tile
        } else {
            if (c < 32 || c > 127) c = 32;
            buf[n] = (unsigned char)(FONT_BASE_TILE + (c - 32));
        }
        n++;
    }

    if (n) set_bkg_tiles(x, y, n, 1, buf);
}
