/* main.c — entry point, title screen, and game loop. */

#include <gb/gb.h>
#include <gb/cgb.h>
#include "font.h"
#include "game.h"
#include "transition.h"
#include "title_tiles.h"
#include "sound.h"

/* Boot/title screen palette 0: text palette (c1=darkest for font readability) */
const UWORD boot_pal[4] = {
    RGB(28, 31, 26),   /* lightest green (background) */
    RGB(1, 3, 4),      /* darkest green (font glyphs) */
    RGB(6, 13, 10),    /* mid-dark green */
    RGB(17, 24, 14)    /* mid-light green */
};

/* Palette 2: waterfall - tilesheet green gradient */
const UWORD waterfall_pal[4] = {
    RGB(28, 31, 26),   /* lightest green */
    RGB(17, 24, 14),   /* mid-light green */
    RGB(6, 13, 10),    /* mid-dark green */
    RGB(1, 3, 4)       /* darkest green */
};

static UINT8 wfall_frame;
static UINT8 wfall_timer;
#define WFALL_ANIM_SPEED 12u  /* frames between waterfall animation steps */

void draw_boot_screen(void) {
    UINT8 x, y;
    unsigned char tile0 = 0;
    unsigned char pal0  = 0;
    unsigned char pal2  = 2;
    unsigned char row[2];

    /* Clear BG map to tile 0 */
    for (y = 0; y < 18; y++) {
        for (x = 0; x < 20; x++) {
            set_bkg_tiles(x, y, 1, 1, &tile0);
        }
    }

    /* Set CGB attributes to palette 0 by default */
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        for (y = 0; y < 18; y++) {
            for (x = 0; x < 20; x++) {
                set_bkg_tiles(x, y, 1, 1, &pal0);
            }
        }
        VBK_REG = 0;
    }

    /* Load title tiles (pigeons + waterfall + cross) */
    title_tiles_load();

    /* --- Waterfall columns (left cols 0-1, right cols 18-19) --- */
    /* Fill from row 2 to row 15 for tall columns framing the screen */
    for (y = 2; y < 16; y += 2) {
        /* Left waterfall (cols 0-1) */
        row[0] = TITLE_WFALL_F1_TL;
        row[1] = TITLE_WFALL_F1_TR;
        set_bkg_tiles(0, y, 2, 1, row);
        row[0] = TITLE_WFALL_F1_BL;
        row[1] = TITLE_WFALL_F1_BR;
        set_bkg_tiles(0, (UINT8)(y + 1u), 2, 1, row);

        /* Right waterfall (cols 18-19) */
        row[0] = TITLE_WFALL_F1_TL;
        row[1] = TITLE_WFALL_F1_TR;
        set_bkg_tiles(18, y, 2, 1, row);
        row[0] = TITLE_WFALL_F1_BL;
        row[1] = TITLE_WFALL_F1_BR;
        set_bkg_tiles(18, (UINT8)(y + 1u), 2, 1, row);
    }

    /* Set waterfall tiles to palette 2 */
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        for (y = 2; y < 16; y++) {
            set_bkg_tiles(0, y, 1, 1, &pal2);
            set_bkg_tiles(1, y, 1, 1, &pal2);
            set_bkg_tiles(18, y, 1, 1, &pal2);
            set_bkg_tiles(19, y, 1, 1, &pal2);
        }
        VBK_REG = 0;
    }

    /* --- Text above cross (centered vertically between waterfalls rows 2-15) --- */
    /* Content: text(1) + gap(1) + cross(3) + gap(1) + text(1) = 7 rows
       Start at row 5 to center: 5, 7-9, 11 */
    draw_text(3, 5, "THE GOSPEL OF");

    /* --- Three-bar cross (2x3 tiles, centered at cols 9-10) --- */
    row[0] = TITLE_CROSS_TL;
    row[1] = TITLE_CROSS_TR;
    set_bkg_tiles(9, 7, 2, 1, row);
    row[0] = TITLE_CROSS_ML;
    row[1] = TITLE_CROSS_MR;
    set_bkg_tiles(9, 8, 2, 1, row);
    row[0] = TITLE_CROSS_BL;
    row[1] = TITLE_CROSS_BR;
    set_bkg_tiles(9, 9, 2, 1, row);

    /* "JOHN" centered below cross */
    draw_text(8, 11, "JOHN");

    draw_text(5, 16, "PRESS START");
    draw_text(3, 17, "SELECT:CREDITS");

    /* Init waterfall animation state */
    wfall_frame = 0;
    wfall_timer = 0;
}

/* Update waterfall animation tiles on the title screen */
static void update_waterfall_anim(void) {
    UINT8 y;
    unsigned char row[2];
    unsigned char tl, tr, bl, br;

    wfall_timer++;
    if (wfall_timer < WFALL_ANIM_SPEED) return;
    wfall_timer = 0;

    wfall_frame = (UINT8)((wfall_frame + 1u) & 3u); /* 0-3 */

    /* Pick tiles for current frame */
    switch (wfall_frame) {
        case 1:
            tl = TITLE_WFALL_F2_TL; tr = TITLE_WFALL_F2_TR;
            bl = TITLE_WFALL_F2_BL; br = TITLE_WFALL_F2_BR;
            break;
        case 2:
            tl = TITLE_WFALL_F3_TL; tr = TITLE_WFALL_F3_TR;
            bl = TITLE_WFALL_F3_BL; br = TITLE_WFALL_F3_BR;
            break;
        case 3:
            tl = TITLE_WFALL_F4_TL; tr = TITLE_WFALL_F4_TR;
            bl = TITLE_WFALL_F4_BL; br = TITLE_WFALL_F4_BR;
            break;
        default:
            tl = TITLE_WFALL_F1_TL; tr = TITLE_WFALL_F1_TR;
            bl = TITLE_WFALL_F1_BL; br = TITLE_WFALL_F1_BR;
            break;
    }

    for (y = 2; y < 16; y += 2) {
        row[0] = tl; row[1] = tr;
        set_bkg_tiles(0, y, 2, 1, row);
        set_bkg_tiles(18, y, 2, 1, row);
        row[0] = bl; row[1] = br;
        set_bkg_tiles(0, (UINT8)(y + 1u), 2, 1, row);
        set_bkg_tiles(18, (UINT8)(y + 1u), 2, 1, row);
    }
}

void main(void) {
    UBYTE prev_keys;
    UBYTE cur_keys;
    UBYTE pressed;

    DISPLAY_OFF;

    SHOW_BKG;
    HIDE_WIN;
    HIDE_SPRITES;

    /* Set up CGB palettes */
    if (_cpu == CGB_TYPE) {
        set_bkg_palette(0, 1, boot_pal);
        set_bkg_palette(2, 1, waterfall_pal);
        fade_set_base_bg_palette(0, boot_pal);
        fade_set_base_bg_palette(2, waterfall_pal);
    } else {
        BGP_REG = 0xE4u;
    }

    /* Initialize sound hardware */
    sound_init();

    /* Load font */
    font_load();

    /* Draw boot screen */
    draw_boot_screen();

    /* Initialize game state machine */
    game_init();

    /* Start background music */
    music_start();

    DISPLAY_ON;

    prev_keys = 0;

    /* Main game loop */
    while (1) {
        wait_vbl_done();

        cur_keys = joypad();
        pressed = (UBYTE)(cur_keys & ~prev_keys);
        prev_keys = cur_keys;

        /* Advance music sequencer */
        sound_update();

        /* Animate waterfall on title screen */
        if (game_get_state() == STATE_BOOT) {
            update_waterfall_anim();
        }

        game_update(pressed, cur_keys);
        game_render();
    }
}
