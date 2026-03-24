/* player.h — player sprite movement, animation, and collision. */

#ifndef PLAYER_H
#define PLAYER_H

#include <gb/gb.h>

/* Initialize player at a tile position */
void player_init(UINT8 tile_x, UINT8 tile_y);

/* Update player movement from input */
void player_update(UBYTE pressed, UBYTE held);

/* Render player sprite */
void player_render(void);

/* Hide player sprites (for reader/matching screens) */
void player_hide(void);

/* Show player sprites */
void player_show(void);

/* Reset sprite loaded flag (call before re-entering map after VRAM overwrite) */
void player_reset_sprites(void);

/* Get player tile position */
UINT8 player_get_x(void);
UINT8 player_get_y(void);

/* Get player pixel Y position (for smooth scroll calculations) */
UINT8 player_get_pixel_y(void);

/* Get the tile the player is facing */
void player_get_facing_tile(UINT8 *out_x, UINT8 *out_y);

/* Get player direction */
UINT8 player_get_dir(void);

/* Character selection */
UINT8 player_get_character(void);
void player_set_character(UINT8 idx);

#endif
