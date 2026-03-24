/*
 * pet.c -- companion pet that follows the player like Pikachu in Yellow.
 *
 * Uses OAM slots 4-7 (16x16 sprite in 8x8 mode).
 * Sprite tiles 24-31 in VRAM (2 frames x 4 quadrants).
 * Follows the player's position trail with a 1-tile delay.
 * Front-facing only (no directional sprites).
 * Banked to bank 15 to save home bank space.
 */

#pragma bank 15

#include <gb/gb.h>
#include <gb/cgb.h>
#include "pet.h"
#include "pet_sprites.h"
#include "map.h"
#include "font.h"
#include "transition.h"
#include "player_sprites.h"

/* OAM sprite indices */
#define PET_TL 4u
#define PET_TR 5u
#define PET_BL 6u
#define PET_BR 7u

#define OAM_X_OFF 8u
#define OAM_Y_OFF 16u
#define TILE_PX   8u

/* Ring buffer for position trail */
#define TRAIL_SIZE 8u

static UINT8 pet_px, pet_py;       /* pixel position */
static UINT8 pet_tx, pet_ty;       /* current tile position */
static UINT8 pet_walking;
static UINT8 pet_walk_remain;
static INT8  pet_walk_dx, pet_walk_dy;
static UINT8 pet_anim_frame;       /* 0 = idle, 1 = walk */
static UINT8 pet_anim_timer;
static UINT8 pet_visible;
static UINT8 pet_flip_x;          /* 1 = mirror horizontally (moving left) */
static UINT8 cur_pet_idx;

static UINT8 trail_x[TRAIL_SIZE];
static UINT8 trail_y[TRAIL_SIZE];
static UINT8 trail_head;   /* next write position */
static UINT8 trail_count;  /* entries in buffer */


static void update_pet_tiles(void) {
    UINT8 base;

    /* frame 0 -> tiles 24-27, frame 1 -> tiles 28-31 */
    base = PET_SPRITE_BASE + (pet_anim_frame ? 4u : 0u);

    set_sprite_tile(PET_TL, base);
    set_sprite_tile(PET_TR, (UINT8)(base + 1u));
    set_sprite_tile(PET_BL, (UINT8)(base + 2u));
    set_sprite_tile(PET_BR, (UINT8)(base + 3u));
}


static void update_pet_pos(void) {
    UINT8 sx, sy, scroll, props;

    sx = pet_px + OAM_X_OFF;
    scroll = map_get_scroll_y();
    sy = (UINT8)(pet_py + scroll + OAM_Y_OFF);

    if (pet_flip_x) {
        props = S_FLIPX;
        set_sprite_prop(PET_TL, props);
        set_sprite_prop(PET_TR, props);
        set_sprite_prop(PET_BL, props);
        set_sprite_prop(PET_BR, props);
        /* Swap left/right columns so the mirrored tiles line up */
        move_sprite(PET_TR, sx,              sy);
        move_sprite(PET_TL, (UINT8)(sx + 8u), sy);
        move_sprite(PET_BR, sx,              (UINT8)(sy + 8u));
        move_sprite(PET_BL, (UINT8)(sx + 8u), (UINT8)(sy + 8u));
    } else {
        set_sprite_prop(PET_TL, 0);
        set_sprite_prop(PET_TR, 0);
        set_sprite_prop(PET_BL, 0);
        set_sprite_prop(PET_BR, 0);
        move_sprite(PET_TL, sx,              sy);
        move_sprite(PET_TR, (UINT8)(sx + 8u), sy);
        move_sprite(PET_BL, sx,              (UINT8)(sy + 8u));
        move_sprite(PET_BR, (UINT8)(sx + 8u), (UINT8)(sy + 8u));
    }
}


void pet_init(UINT8 tx, UINT8 ty) BANKED {
    pet_tx = tx;
    pet_ty = ty;
    pet_px = (UINT8)(tx * TILE_PX);
    pet_py = (UINT8)(ty * TILE_PX);
    pet_walking = 0;
    pet_walk_remain = 0;
    pet_walk_dx = 0;
    pet_walk_dy = 0;
    pet_anim_frame = 0;
    pet_anim_timer = 0;
    pet_visible = 1;
    pet_flip_x = 0;

    trail_head = 0;
    trail_count = 0;

    pet_sprites_load_pet(cur_pet_idx);

    set_sprite_prop(PET_TL, 0);
    set_sprite_prop(PET_TR, 0);
    set_sprite_prop(PET_BL, 0);
    set_sprite_prop(PET_BR, 0);

    update_pet_tiles();
    update_pet_pos();
}


void pet_push_position(UINT8 tx, UINT8 ty) BANKED {
    trail_x[trail_head] = tx;
    trail_y[trail_head] = ty;
    trail_head = (UINT8)((trail_head + 1u) % TRAIL_SIZE);
    if (trail_count < TRAIL_SIZE) {
        trail_count++;
    }
}


void pet_update(void) BANKED {
    if (!pet_visible) return;

    if (pet_walking) {
        /* Advance pixel position toward target tile */
        pet_px = (UINT8)((INT8)pet_px + pet_walk_dx);
        pet_py = (UINT8)((INT8)pet_py + pet_walk_dy);
        pet_walk_remain--;

        /* Toggle walk animation every 4 frames */
        pet_anim_timer++;
        if (pet_anim_timer >= 4u) {
            pet_anim_timer = 0;
            pet_anim_frame ^= 1u;
        }

        if (pet_walk_remain == 0) {
            /* Snap to grid */
            pet_px = (UINT8)(pet_tx * TILE_PX);
            pet_py = (UINT8)(pet_ty * TILE_PX);
            pet_walking = 0;
            pet_anim_frame = 0;
            pet_anim_timer = 0;
        }
    }

    if (!pet_walking && trail_count > 1) {
        /* Pop oldest position from ring buffer */
        UINT8 read_idx;
        UINT8 target_tx, target_ty;
        INT8 dx, dy;

        read_idx = (UINT8)((trail_head + TRAIL_SIZE - trail_count) % TRAIL_SIZE);
        target_tx = trail_x[read_idx];
        target_ty = trail_y[read_idx];
        trail_count--;

        dx = (INT8)target_tx - (INT8)pet_tx;
        dy = (INT8)target_ty - (INT8)pet_ty;

        /* Only walk if there's actually a tile difference */
        if (dx != 0 || dy != 0) {
            /* Normalize to -1, 0, +1 */
            if (dx > 0) { pet_walk_dx = 1; pet_flip_x = 1; }
            else if (dx < 0) { pet_walk_dx = -1; pet_flip_x = 0; }
            else pet_walk_dx = 0;

            if (dy > 0) pet_walk_dy = 1;
            else if (dy < 0) pet_walk_dy = -1;
            else pet_walk_dy = 0;

            pet_tx = target_tx;
            pet_ty = target_ty;
            pet_walking = 1;
            pet_walk_remain = TILE_PX;
            pet_anim_frame = 1;
            pet_anim_timer = 0;
        }
    }

    update_pet_tiles();
}


void pet_render(void) BANKED {
    if (pet_visible) {
        update_pet_pos();
    }
}


void pet_hide(void) BANKED {
    pet_visible = 0;
    move_sprite(PET_TL, 0, 0);
    move_sprite(PET_TR, 0, 0);
    move_sprite(PET_BL, 0, 0);
    move_sprite(PET_BR, 0, 0);
}


void pet_show(void) BANKED {
    pet_visible = 1;
    update_pet_pos();
}


void pet_set_animal(UINT8 idx) BANKED {
    cur_pet_idx = idx;
    pet_sprites_load_pet(idx);
    update_pet_tiles();
}


UINT8 pet_get_animal(void) BANKED {
    return cur_pet_idx;
}


/* --- Pet preview for the selection screen (banked to save home bank) --- */

#define PET_PREVIEW_TILE  132u
#define PET_PREVIEW_X     9u
#define PET_PREVIEW_Y     7u

static const char * const pet_names[NUM_PETS] = {
    "CHINCHILLA", "CHIPMUNK", "FERRET",
    "GERBIL", "GUINEA PIG", "HAMSTER",
    "HEDGEHOG", "MOUSE", "SQUIRREL"
};

static const UWORD pet_preview_pal[4] = {
    RGB(28, 31, 26),
    RGB(17, 24, 14),
    RGB(6, 13, 10),
    RGB(1, 3, 4)
};

static UINT8 pet_pal_inited;

void pet_draw_preview(UINT8 pet_idx) BANKED {
    unsigned char row[2];
    char num_buf[8];
    UINT8 ones;

    if (!pet_pal_inited) {
        if (_cpu == CGB_TYPE) {
            set_bkg_palette(1, 1, pet_preview_pal);
            fade_set_base_bg_palette(1, pet_preview_pal);
        }
        pet_pal_inited = 1;
    }

    pet_sprites_load_preview(pet_idx);

    row[0] = PET_PREVIEW_TILE;
    row[1] = (unsigned char)(PET_PREVIEW_TILE + 1u);
    set_bkg_tiles(PET_PREVIEW_X, PET_PREVIEW_Y, 2, 1, row);
    row[0] = (unsigned char)(PET_PREVIEW_TILE + 2u);
    row[1] = (unsigned char)(PET_PREVIEW_TILE + 3u);
    set_bkg_tiles(PET_PREVIEW_X, (UINT8)(PET_PREVIEW_Y + 1u), 2, 1, row);

    if (_cpu == CGB_TYPE) {
        unsigned char pal1 = 1;
        VBK_REG = 1;
        set_bkg_tiles(PET_PREVIEW_X, PET_PREVIEW_Y, 1, 1, &pal1);
        set_bkg_tiles((UINT8)(PET_PREVIEW_X + 1u), PET_PREVIEW_Y, 1, 1, &pal1);
        set_bkg_tiles(PET_PREVIEW_X, (UINT8)(PET_PREVIEW_Y + 1u), 1, 1, &pal1);
        set_bkg_tiles((UINT8)(PET_PREVIEW_X + 1u), (UINT8)(PET_PREVIEW_Y + 1u), 1, 1, &pal1);
        VBK_REG = 0;
    }

    draw_text(7, PET_PREVIEW_Y, "<");
    draw_text(12, PET_PREVIEW_Y, ">");

    /* Clear old name and draw new one centered */
    draw_text(2, (UINT8)(PET_PREVIEW_Y + 3u), "                ");
    {
        UINT8 len = 0;
        const char *n = pet_names[pet_idx];
        while (n[len]) len++;
        draw_text((UINT8)((20u - len) / 2u), (UINT8)(PET_PREVIEW_Y + 3u), n);
    }

    /* Draw "N/9" counter */
    draw_text(8, (UINT8)(PET_PREVIEW_Y + 5u), "     ");
    ones = (UINT8)(pet_idx + 1u);
    num_buf[0] = (char)('0' + ones);
    num_buf[1] = '/';
    num_buf[2] = '9';
    num_buf[3] = '\0';
    draw_text(9, (UINT8)(PET_PREVIEW_Y + 5u), num_buf);
}

void pet_reset_preview(void) BANKED {
    pet_pal_inited = 0;
}


/* --- Character preview for the selection screen --- */

#define CHAR_PREVIEW_TILE  128u
#define CHAR_PREVIEW_X     9u
#define CHAR_PREVIEW_Y     7u

static UINT8 char_pal_inited;

void select_draw_char(UINT8 char_idx) BANKED {
    unsigned char row[2];
    char num_buf[8];
    UINT8 tens, ones;

    if (!char_pal_inited) {
        if (_cpu == CGB_TYPE) {
            set_bkg_palette(1, 1, pet_preview_pal);
            fade_set_base_bg_palette(1, pet_preview_pal);
        }
        char_pal_inited = 1;
    }

    player_sprites_load_preview(char_idx);

    row[0] = CHAR_PREVIEW_TILE;
    row[1] = (unsigned char)(CHAR_PREVIEW_TILE + 1u);
    set_bkg_tiles(CHAR_PREVIEW_X, CHAR_PREVIEW_Y, 2, 1, row);
    row[0] = (unsigned char)(CHAR_PREVIEW_TILE + 2u);
    row[1] = (unsigned char)(CHAR_PREVIEW_TILE + 3u);
    set_bkg_tiles(CHAR_PREVIEW_X, (UINT8)(CHAR_PREVIEW_Y + 1u), 2, 1, row);

    if (_cpu == CGB_TYPE) {
        unsigned char pal1 = 1;
        VBK_REG = 1;
        set_bkg_tiles(CHAR_PREVIEW_X, CHAR_PREVIEW_Y, 1, 1, &pal1);
        set_bkg_tiles((UINT8)(CHAR_PREVIEW_X + 1u), CHAR_PREVIEW_Y, 1, 1, &pal1);
        set_bkg_tiles(CHAR_PREVIEW_X, (UINT8)(CHAR_PREVIEW_Y + 1u), 1, 1, &pal1);
        set_bkg_tiles((UINT8)(CHAR_PREVIEW_X + 1u), (UINT8)(CHAR_PREVIEW_Y + 1u), 1, 1, &pal1);
        VBK_REG = 0;
    }

    draw_text(7, CHAR_PREVIEW_Y, "<");
    draw_text(12, CHAR_PREVIEW_Y, ">");

    /* Draw "NN/16" below preview */
    draw_text(7, (UINT8)(CHAR_PREVIEW_Y + 3u), "      ");
    tens = (UINT8)((char_idx + 1u) / 10u);
    ones = (UINT8)((char_idx + 1u) % 10u);
    if (tens > 0) {
        num_buf[0] = (char)('0' + tens);
        num_buf[1] = (char)('0' + ones);
        num_buf[2] = '/';
        num_buf[3] = '1';
        num_buf[4] = '6';
        num_buf[5] = '\0';
        draw_text(8, (UINT8)(CHAR_PREVIEW_Y + 3u), num_buf);
    } else {
        num_buf[0] = (char)('0' + ones);
        num_buf[1] = '/';
        num_buf[2] = '1';
        num_buf[3] = '6';
        num_buf[4] = '\0';
        draw_text(8, (UINT8)(CHAR_PREVIEW_Y + 3u), num_buf);
    }
}

void select_reset_char(void) BANKED {
    char_pal_inited = 0;
}

void credits_draw(void) BANKED {
    draw_text(4, 1, "~ CREDITS ~");
    draw_text(1, 3, "DEVELOPER");
    draw_text(2, 4, "WILLIAM FRICK");
    draw_text(1, 6, "SCRIPTURE TEXT");
    draw_text(2, 7, "WORLD ENGLISH");
    draw_text(2, 8, "BIBLE");
    draw_text(1, 10, "TILE ART - ITCH.IO");
    draw_text(2, 11, "COLORTV");
    draw_text(2, 12, "MATERIALFUTURE");
    draw_text(2, 13, "GAMEBOWGAMES");
    draw_text(1, 15, "HYMN MELODIES");
    draw_text(2, 16, "ORTHODOX CHURCH");
    draw_text(2, 17, "OF AMERICA");
}
