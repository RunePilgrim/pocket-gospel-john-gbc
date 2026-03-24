/*
 * transition.c — CGB palette fade-in/fade-out for screen transitions.
 * Interpolates RGB channels toward/from black over FADE_STEPS steps.
 */

#include <gb/gb.h>
#include <gb/cgb.h>
#include "transition.h"

#define NUM_BG_PALS  3
#define NUM_OBJ_PALS 1

static UWORD base_bg[NUM_BG_PALS][4];
static UWORD base_obj[NUM_OBJ_PALS][4];

static UINT8 fade_dir;       /* 0=none, 1=fading out, 2=fading in */
static UINT8 fade_step;      /* current step (0..FADE_STEPS) */
static UINT8 fade_timer;     /* frame counter within current step */

/* Extract 5-bit R/G/B from GBC color word */
static UINT8 cR(UWORD c) { return (UINT8)(c & 0x1F); }
static UINT8 cG(UWORD c) { return (UINT8)((c >> 5) & 0x1F); }
static UINT8 cB(UWORD c) { return (UINT8)((c >> 10) & 0x1F); }

static UWORD make_rgb(UINT8 r, UINT8 g, UINT8 b) {
    return (UWORD)((UWORD)r | ((UWORD)g << 5) | ((UWORD)b << 10));
}

static UWORD lerp_color(UWORD base, UINT8 step, UINT8 total, UINT8 dir) {
    /* dir=1 (fade out): interpolate from base toward black
       dir=2 (fade in):  interpolate from black toward base */
    UINT8 r, g, b;
    UINT8 br, bg_c, bb;

    br = cR(base);
    bg_c = cG(base);
    bb = cB(base);

    if (dir == 1) {
        /* Fading out: step goes 0..total, brightness decreases */
        r = (UINT8)((br * (total - step)) / total);
        g = (UINT8)((bg_c * (total - step)) / total);
        b = (UINT8)((bb * (total - step)) / total);
    } else {
        /* Fading in: step goes 0..total, brightness increases */
        r = (UINT8)((br * step) / total);
        g = (UINT8)((bg_c * step) / total);
        b = (UINT8)((bb * step) / total);
    }

    return make_rgb(r, g, b);
}

static void apply_fade(void) {
    UINT8 p, c;
    UWORD tmp[4];

    if (_cpu == CGB_TYPE) {
        for (p = 0; p < NUM_BG_PALS; p++) {
            for (c = 0; c < 4; c++) {
                tmp[c] = lerp_color(base_bg[p][c], fade_step, FADE_STEPS, fade_dir);
            }
            set_bkg_palette(p, 1, tmp);
        }
        for (p = 0; p < NUM_OBJ_PALS; p++) {
            for (c = 0; c < 4; c++) {
                tmp[c] = lerp_color(base_obj[p][c], fade_step, FADE_STEPS, fade_dir);
            }
            set_sprite_palette(p, 1, tmp);
        }
    } else {
        /* DMG fallback: step through 4 brightness levels */
        UINT8 shift;
        if (fade_dir == 1) {
            shift = fade_step;
        } else {
            shift = (UINT8)(FADE_STEPS - fade_step);
        }
        switch (shift) {
            case 0: BGP_REG = 0xE4u; break; /* normal */
            case 1: BGP_REG = 0xF9u; break; /* darker */
            case 2: BGP_REG = 0xFEu; break; /* very dark */
            case 3: /* fall through */
            case 4: BGP_REG = 0xFFu; break; /* black */
            default: BGP_REG = 0xFFu; break;
        }
    }
}

void fade_set_base_bg_palette(UINT8 pal_idx, const UWORD colors[4]) {
    UINT8 c;
    if (pal_idx < NUM_BG_PALS) {
        for (c = 0; c < 4; c++) {
            base_bg[pal_idx][c] = colors[c];
        }
    }
}

void fade_set_base_obj_palette(UINT8 pal_idx, const UWORD colors[4]) {
    UINT8 c;
    if (pal_idx < NUM_OBJ_PALS) {
        for (c = 0; c < 4; c++) {
            base_obj[pal_idx][c] = colors[c];
        }
    }
}

void fade_out_start(void) {
    fade_dir = 1;
    fade_step = 0;
    fade_timer = 0;
}

void fade_in_start(void) {
    fade_dir = 2;
    fade_step = 0;
    fade_timer = 0;
}

UINT8 fade_update(void) {
    if (fade_dir == 0) return 1;

    fade_timer++;
    if (fade_timer >= FADE_STEP_FRAMES) {
        fade_timer = 0;
        fade_step++;

        if (fade_step > FADE_STEPS) {
            fade_dir = 0;
            return 1; /* done */
        }

        apply_fade();
    }

    return 0;
}

UINT8 fade_active(void) {
    return (fade_dir != 0) ? 1u : 0u;
}

void fade_to_black(void) {
    UWORD black[4];
    UINT8 p;

    black[0] = 0;
    black[1] = 0;
    black[2] = 0;
    black[3] = 0;

    if (_cpu == CGB_TYPE) {
        for (p = 0; p < NUM_BG_PALS; p++) {
            set_bkg_palette(p, 1, black);
        }
        for (p = 0; p < NUM_OBJ_PALS; p++) {
            set_sprite_palette(p, 1, black);
        }
    } else {
        BGP_REG = 0xFFu;
    }
}

void fade_to_white(void) {
    UWORD white[4];
    UINT8 p;

    white[0] = RGB(31, 31, 31);
    white[1] = RGB(31, 31, 31);
    white[2] = RGB(31, 31, 31);
    white[3] = RGB(31, 31, 31);

    if (_cpu == CGB_TYPE) {
        for (p = 0; p < NUM_BG_PALS; p++) {
            set_bkg_palette(p, 1, white);
        }
        for (p = 0; p < NUM_OBJ_PALS; p++) {
            set_sprite_palette(p, 1, white);
        }
    } else {
        BGP_REG = 0x00u;
    }
}
