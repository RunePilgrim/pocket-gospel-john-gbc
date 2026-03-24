/*
 * player.c — player sprite movement, animation, and collision.
 * Uses 4 OAM sprites (8x8 mode) to form a 16x16 character.
 * Smooth pixel-by-pixel movement, 1px per frame (matches Pokemon Gold pace).
 * Turn-in-place: tapping a new direction faces that way without walking.
 */

#include <gb/gb.h>
#include <gb/cgb.h>
#include "player.h"
#include "player_sprites.h"
#include "map.h"
#include "transition.h"

/* Sprite OAM indices */
#define SPR_TL 0
#define SPR_TR 1
#define SPR_BL 2
#define SPR_BR 3

/* OAM hardware offset: sprites draw at (x-8, y-16) */
#define OAM_X_OFF 8u
#define OAM_Y_OFF 16u

/* Movement speed: pixels per frame during a walk step.
   Pokemon Gold uses 2px/frame at ~30fps (overworld runs half-speed).
   Our loop runs at ~60fps, so 1px/frame matches Gold's real-time pace. */
#define WALK_SPEED   1u    /* 1px/frame @ 60fps = same speed as Gold's 2px @ 30fps */
#define TILE_PX      8u    /* pixels per tile */

static UINT8 px, py;          /* pixel position (top-left of 16x16) */
static UINT8 tile_x, tile_y;  /* current tile position (aligned) */
static UINT8 dir;              /* current direction (DIR_*) */
static UINT8 anim_frame;      /* 0 or 1 */
static UINT8 anim_timer;      /* frame counter for walk cycle */
static UINT8 walking;         /* 1 if currently in a smooth walk step */
static UINT8 walk_remain;     /* pixels remaining in current walk step */
static INT8  walk_dx;         /* -1, 0, or +1: walk direction X */
static INT8  walk_dy;         /* -1, 0, or +1: walk direction Y */
static UINT8 visible;
static UINT8 turn_cooldown;   /* frames remaining before walk allowed after turn */
static UINT8 walk_step_count; /* counts tile steps within continuous walk */
static UINT8 step_parity;     /* 0 or 1: alternates H-flip for DOWN/UP walking */
static UINT8 cur_char_idx;    /* currently loaded character index */

/* After turning in place, wait this many frames before allowing a walk.
   At 60fps, 6 frames ≈ 100ms — enough time for a tap to release. */
#define TURN_DELAY   6u

static void update_sprite_tiles(void) {
    UINT8 base_tile;
    UINT8 use_dir;
    UINT8 flip;

    /* RIGHT reuses LEFT tiles with X-flip */
    use_dir = dir;
    flip = 0;
    if (use_dir == DIR_RIGHT) {
        use_dir = DIR_LEFT;
        flip = 1;
    }
    base_tile = (UINT8)(use_dir * 8u + anim_frame * 4u);

    /* For DOWN/UP walking frames, alternate H-flip each stride
       to create left-right footwork (Gold-style 4-phase walk). */
    if (!flip && anim_frame == 1u && step_parity &&
        (dir == DIR_DOWN || dir == DIR_UP)) {
        flip = 1;
    }

    if (flip) {
        /* X-flipped: swap TL<->TR, BL<->BR and set S_FLIPX */
        set_sprite_tile(SPR_TL, (UINT8)(base_tile + 1u));
        set_sprite_tile(SPR_TR, base_tile);
        set_sprite_tile(SPR_BL, (UINT8)(base_tile + 3u));
        set_sprite_tile(SPR_BR, (UINT8)(base_tile + 2u));
        set_sprite_prop(SPR_TL, S_FLIPX);
        set_sprite_prop(SPR_TR, S_FLIPX);
        set_sprite_prop(SPR_BL, S_FLIPX);
        set_sprite_prop(SPR_BR, S_FLIPX);
    } else {
        set_sprite_tile(SPR_TL, base_tile);
        set_sprite_tile(SPR_TR, (UINT8)(base_tile + 1u));
        set_sprite_tile(SPR_BL, (UINT8)(base_tile + 2u));
        set_sprite_tile(SPR_BR, (UINT8)(base_tile + 3u));
        set_sprite_prop(SPR_TL, 0);
        set_sprite_prop(SPR_TR, 0);
        set_sprite_prop(SPR_BL, 0);
        set_sprite_prop(SPR_BR, 0);
    }
}

static void update_sprite_pos(void) {
    UINT8 sx, sy;
    UINT8 scroll;
    sx = px + OAM_X_OFF;
    /* Adjust Y for sky scroll: when BG scrolls up, sprite moves down to match */
    scroll = map_get_scroll_y();
    sy = (UINT8)(py + scroll + OAM_Y_OFF);

    move_sprite(SPR_TL, sx,              sy);
    move_sprite(SPR_TR, (UINT8)(sx + 8u), sy);
    move_sprite(SPR_BL, sx,              (UINT8)(sy + 8u));
    move_sprite(SPR_BR, (UINT8)(sx + 8u), (UINT8)(sy + 8u));
}

/* Try to start a smooth walk in the current facing direction.
   Caller must set dir before calling. Returns 1 if walk started, 0 if blocked. */
static UINT8 try_walk(INT8 dx, INT8 dy) {
    UINT8 target_tx, target_ty;
    UINT8 check_x2, check_y2;

    target_tx = (UINT8)((INT8)tile_x + dx);
    target_ty = (UINT8)((INT8)tile_y + dy);

    /* Bounds check */
    if (dx < 0 && tile_x == 0) return 0;
    if (dy < 0 && tile_y == 0) return 0;
    if (dx > 0 && tile_x >= (MAP_W - 2u)) return 0;
    if (dy > 0 && tile_y >= (MAP_ROWS - 2u)) return 0;

    /* Collision check (16x16 sprite = 2x2 tiles) */
    check_x2 = (UINT8)(target_tx + 1u);
    check_y2 = (UINT8)(target_ty + 1u);

    if (map_is_solid(target_tx, target_ty) ||
        map_is_solid(check_x2, target_ty) ||
        map_is_solid(target_tx, check_y2) ||
        map_is_solid(check_x2, check_y2)) {
        return 0;
    }

    /* Start smooth walk */
    walking = 1;
    walk_remain = TILE_PX;
    walk_dx = dx;
    walk_dy = dy;

    /* Gold-style 4-phase animation: alternate standing/walking per tile step.
       Odd steps show walking frame (with 1px bounce), even steps show standing.
       Step parity toggles each time the walking frame appears, creating
       left-right foot alternation via H-flip on DOWN/UP directions. */
    walk_step_count++;
    anim_frame = (walk_step_count & 1u) ? 1u : 0u;
    if (anim_frame == 1u) {
        step_parity = (UINT8)(step_parity ^ 1u);
    }

    /* Update tile position immediately for collision/interaction purposes */
    tile_x = target_tx;
    tile_y = target_ty;

    return 1;
}

void player_init(UINT8 tx, UINT8 ty) {
    UWORD pal[4];

    tile_x = tx;
    tile_y = ty;
    px = (UINT8)(tx * 8u);
    py = (UINT8)(ty * 8u);
    dir = DIR_DOWN;
    anim_frame = 0;
    anim_timer = 0;
    walking = 0;
    walk_remain = 0;
    walk_dx = 0;
    walk_dy = 0;
    turn_cooldown = 0;
    walk_step_count = 0;
    step_parity = 0;
    visible = 1;

    /* Load sprite tiles for the selected character */
    player_sprites_load_char(cur_char_idx);

    /* Set sprite palette — tilesheet green palette */
    if (_cpu == CGB_TYPE) {
        pal[0] = RGB(0, 0, 0);     /* transparent (not drawn on CGB) */
        pal[1] = RGB(28, 31, 26);  /* lightest green */
        pal[2] = RGB(6, 13, 10);   /* mid-dark green */
        pal[3] = RGB(1, 3, 4);     /* darkest green */
        set_sprite_palette(0, 1, pal);
        fade_set_base_obj_palette(0, pal);
    }
    /* DMG fallback: OBP0 = lightest to darkest (0=transparent,1=light,2=mid,3=dark) */
    OBP0_REG = 0xE4u;  /* 11 10 01 00 = black, dark gray, light gray, white */

    /* Initialize OAM entries */
    set_sprite_prop(SPR_TL, 0);
    set_sprite_prop(SPR_TR, 0);
    set_sprite_prop(SPR_BL, 0);
    set_sprite_prop(SPR_BR, 0);

    update_sprite_tiles();
    update_sprite_pos();

    SHOW_SPRITES;
}

void player_update(UBYTE pressed, UBYTE held) {
    UINT8 step;

    (void)pressed;

    if (walking) {
        /* Continue smooth walk: advance pixels toward target */
        step = WALK_SPEED;
        if (step > walk_remain) step = walk_remain;

        if (walk_dx > 0) px = (UINT8)(px + step);
        else if (walk_dx < 0) px = (UINT8)(px - step);
        if (walk_dy > 0) py = (UINT8)(py + step);
        else if (walk_dy < 0) py = (UINT8)(py - step);

        walk_remain = (UINT8)(walk_remain - step);

        /* Animation frame is set per-step in try_walk (no per-frame toggle).
           Each tile step shows one pose for its full duration:
           odd steps = walking frame (1px bounce), even steps = standing.
           This matches Gold's ~3.75Hz bob rate per 2-tile (16px) stride. */

        if (walk_remain == 0) {
            /* Walk step complete — snap to tile grid */
            walking = 0;
            px = (UINT8)(tile_x * 8u);
            py = (UINT8)(tile_y * 8u);

            /* Chain: if still holding the same direction, keep walking */
            if ((held & J_UP) && dir == DIR_UP) {
                try_walk(0, -1);
            } else if ((held & J_DOWN) && dir == DIR_DOWN) {
                try_walk(0, 1);
            } else if ((held & J_LEFT) && dir == DIR_LEFT) {
                try_walk(-1, 0);
            } else if ((held & J_RIGHT) && dir == DIR_RIGHT) {
                try_walk(1, 0);
            } else {
                /* Stopped — reset to standing pose */
                anim_frame = 0;
                anim_timer = 0;
                walk_step_count = 0;
                step_parity = 0;

                /* Turn toward held direction (saves a frame on direction switch) */
                if (held & J_UP) dir = DIR_UP;
                else if (held & J_DOWN) dir = DIR_DOWN;
                else if (held & J_LEFT) dir = DIR_LEFT;
                else if (held & J_RIGHT) dir = DIR_RIGHT;
            }
        }
    } else {
        /* Not walking — turn-in-place or walk.
         * Cooldown prevents immediate walk after turning direction. */
        if (turn_cooldown > 0) {
            turn_cooldown--;
        }

        if (held & J_UP) {
            if (dir != DIR_UP) { dir = DIR_UP; turn_cooldown = TURN_DELAY; }
            else if (turn_cooldown == 0) try_walk(0, -1);
        } else if (held & J_DOWN) {
            if (dir != DIR_DOWN) { dir = DIR_DOWN; turn_cooldown = TURN_DELAY; }
            else if (turn_cooldown == 0) try_walk(0, 1);
        } else if (held & J_LEFT) {
            if (dir != DIR_LEFT) { dir = DIR_LEFT; turn_cooldown = TURN_DELAY; }
            else if (turn_cooldown == 0) try_walk(-1, 0);
        } else if (held & J_RIGHT) {
            if (dir != DIR_RIGHT) { dir = DIR_RIGHT; turn_cooldown = TURN_DELAY; }
            else if (turn_cooldown == 0) try_walk(1, 0);
        } else {
            /* No direction held — reset cooldown so next press is responsive */
            turn_cooldown = 0;
        }
    }

    update_sprite_tiles();
}

void player_render(void) {
    if (visible) {
        update_sprite_pos();
    }
}

void player_hide(void) {
    visible = 0;
    /* Move sprites offscreen */
    move_sprite(SPR_TL, 0, 0);
    move_sprite(SPR_TR, 0, 0);
    move_sprite(SPR_BL, 0, 0);
    move_sprite(SPR_BR, 0, 0);
}

void player_show(void) {
    visible = 1;
    update_sprite_pos();
}

UINT8 player_get_x(void) { return tile_x; }
UINT8 player_get_y(void) { return tile_y; }
UINT8 player_get_pixel_y(void) { return py; }
UINT8 player_get_dir(void) { return dir; }
UINT8 player_get_character(void) { return cur_char_idx; }

void player_set_character(UINT8 idx) {
    cur_char_idx = idx;
    player_sprites_load_char(idx);
    update_sprite_tiles();
}

void player_reset_sprites(void) {
    player_sprites_load_char(cur_char_idx);
    update_sprite_tiles();
}

void player_get_facing_tile(UINT8 *out_x, UINT8 *out_y) {
    *out_x = tile_x;
    *out_y = tile_y;

    switch (dir) {
        case DIR_UP:
            if (tile_y > 0) *out_y = (UINT8)(tile_y - 1u);
            break;
        case DIR_DOWN:
            *out_y = (UINT8)(tile_y + 2u); /* below the 2-tile sprite */
            break;
        case DIR_LEFT:
            if (tile_x > 0) *out_x = (UINT8)(tile_x - 1u);
            break;
        case DIR_RIGHT:
            *out_x = (UINT8)(tile_x + 2u); /* right of the 2-tile sprite */
            break;
    }
}
