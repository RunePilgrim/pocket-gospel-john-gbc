/*
 * interact.c — handles A-button interactions with map objects.
 * Checks the tile the player is facing and triggers actions.
 * Banked to bank 15 to save home bank space.
 */

#pragma bank 15

#include <gb/gb.h>
#include <gb/cgb.h>
#include "interact.h"
#include "map.h"
#include "player.h"
#include "player_sprites.h" /* for DIR_* constants */
#include "font.h"
#include "church_tiles.h"
#include "sound.h"

static UINT8 prompt_active;
static UINT8 bell_anim_timer;
static UINT8 bell_anim_active;
static UINT8 bell_orig_tile_x;
static UINT8 bell_orig_tile_y;

void interact_init(void) BANKED {
    prompt_active = 0;
    bell_anim_timer = 0;
    bell_anim_active = 0;
}

void interact_show_prompt(const char *line1, const char *line2) BANKED {
    /* Draw prompt text at bottom of screen (rows 16-17) */
    UINT8 x;
    unsigned char blank = 0;
    unsigned char pal0 = 0;

    /* Clear the rows */
    for (x = 0; x < 20; x++) {
        set_bkg_tiles(x, 16, 1, 1, &blank);
        set_bkg_tiles(x, 17, 1, 1, &blank);
    }

    /* Reset CGB palette attributes to palette 0 for text readability */
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        for (x = 0; x < 20; x++) {
            set_bkg_tiles(x, 16, 1, 1, &pal0);
            set_bkg_tiles(x, 17, 1, 1, &pal0);
        }
        VBK_REG = 0;
    }

    if (line1) draw_text(0, 16, line1);
    if (line2) draw_text(0, 17, line2);

    prompt_active = 1;
}

void interact_hide_prompt(void) BANKED {
    if (prompt_active) {
        /* Redraw the map tiles for rows 16-17 */
        MapData *m;
        UINT8 x;
        unsigned char row_buf[20];

        m = map_get_current();
        if (m) {
            for (x = 0; x < MAP_W; x++) {
                row_buf[x] = m->tile_base + m->tiles[16u * MAP_W + x];
            }
            set_bkg_tiles(0, 16, MAP_W, 1, row_buf);

            for (x = 0; x < MAP_W; x++) {
                row_buf[x] = m->tile_base + m->tiles[17u * MAP_W + x];
            }
            set_bkg_tiles(0, 17, MAP_W, 1, row_buf);
        }
        prompt_active = 0;
    }
}

UINT8 interact_prompt_visible(void) BANKED {
    return prompt_active;
}

UINT8 interact_check(void) BANKED {
    UINT8 fx, fy;
    UINT8 px_t, py_t;
    UINT8 d;
    MapObject *obj;

    player_get_facing_tile(&fx, &fy);
    px_t = player_get_x();
    py_t = player_get_y();
    d = player_get_dir();

    /* Primary: check the facing tile */
    obj = map_get_object(fx, fy);

    /* Secondary: for UP/DOWN facing, also check the other column of the facing edge;
       for LEFT/RIGHT facing, also check the other row of the facing edge.
       This ensures 2x2 player can interact with centered objects. */
    if (!obj) {
        if (d == DIR_UP || d == DIR_DOWN) {
            obj = map_get_object((UINT8)(fx + 1u), fy);
        } else {
            obj = map_get_object(fx, (UINT8)(fy + 1u));
        }
    }

    /* Tertiary: check tiles the player is standing on (for doors) */
    if (!obj) {
        obj = map_get_object(px_t, py_t);
    }
    if (!obj) {
        obj = map_get_object((UINT8)(px_t + 1u), py_t);
    }
    if (!obj) {
        obj = map_get_object(px_t, (UINT8)(py_t + 1u));
    }
    if (!obj) {
        obj = map_get_object((UINT8)(px_t + 1u), (UINT8)(py_t + 1u));
    }

    if (!obj) return INTERACT_NONE;

    switch (obj->type) {
        case OBJ_CANDLE:
            sfx_confirm();
            if (obj->state == 0) {
                /* Light the torch — swap both top and bottom tiles */
                obj->state = 1;
                map_set_tile(obj->x, obj->y, CT_CANDLE_LIT);
                map_set_tile(obj->x, (UINT8)(obj->y - 1u), CT_CANDLE_TOP_LIT);
                interact_show_prompt(" THE TORCH BLAZES  ", " WITH WARM LIGHT   ");
            } else {
                /* Extinguish the torch — swap both tiles back */
                obj->state = 0;
                map_set_tile(obj->x, obj->y, CT_CANDLE_UNLIT);
                map_set_tile(obj->x, (UINT8)(obj->y - 1u), CT_CANDLE_TOP_UNLIT);
                interact_show_prompt(" THE FLAME DIES OUT", " DARKNESS RETURNS  ");
            }
            return INTERACT_CANDLE;

        case OBJ_ALTAR_BOOK:
            interact_show_prompt("THE BOOK LIES OPEN", "A:READ   B:CANCEL");
            return INTERACT_READER;

        case OBJ_BELL_ROPE:
            /* Start bell pull animation */
            bell_anim_active = 1;
            bell_anim_timer = 0;
            bell_orig_tile_x = obj->x;
            bell_orig_tile_y = obj->y;
            /* Swap to pulled rope tile */
            map_set_tile(obj->x, obj->y, CT_ROPE_PULLED);
            sfx_bell();
            interact_show_prompt("THE BELL RINGS OUT", "ACROSS THE LAND");
            return INTERACT_BELL;

        case OBJ_CARD_TABLE:
            interact_show_prompt("A SET OF CARDS", "A:PLAY   B:CANCEL");
            return INTERACT_MATCHING;

        case OBJ_DOOR_OUT:
            return INTERACT_DOOR_OUT;

        case OBJ_DOOR_IN:
            return INTERACT_DOOR_IN;

        default:
            return INTERACT_NONE;
    }
}

void interact_update(void) BANKED {
    /* Bell rope animation */
    if (bell_anim_active) {
        bell_anim_timer++;

        if (bell_anim_timer == 15u) {
            /* Rope returns to normal */
            map_set_tile(bell_orig_tile_x, bell_orig_tile_y, CT_ROPE_BOTTOM);
            bell_anim_active = 0;
        } else if (bell_anim_timer == 8u) {
            /* Brief screen shake effect - shift BG by 1 pixel */
            SCY_REG = 1;
        } else if (bell_anim_timer == 10u) {
            SCY_REG = 0; /* reset */
        } else if (bell_anim_timer == 12u) {
            SCY_REG = 1;
        } else if (bell_anim_timer == 14u) {
            SCY_REG = 0;
        }
    }
}
