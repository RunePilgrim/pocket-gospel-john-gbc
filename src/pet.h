/* pet.h -- companion pet that follows the player.
 * Banked to bank 15 alongside map.c and interact.c. */

#ifndef PET_H
#define PET_H

#include <gb/gb.h>

/* Place pet at a tile position and clear the trail buffer. */
void pet_init(UINT8 tile_x, UINT8 tile_y) BANKED;

/* Record a tile position into the trail buffer.
   Call when the player leaves a tile (before moving to the next). */
void pet_push_position(UINT8 tx, UINT8 ty) BANKED;

/* Advance pet movement toward the next trail position. */
void pet_update(void) BANKED;

/* Write pet sprite positions to OAM. */
void pet_render(void) BANKED;

/* Hide pet sprites (reader/matching screens). */
void pet_hide(void) BANKED;

/* Show pet sprites. */
void pet_show(void) BANKED;

/* Set which pet animal to display and reload its sprite tiles. */
void pet_set_animal(UINT8 idx) BANKED;

/* Get currently selected pet index. */
UINT8 pet_get_animal(void) BANKED;

/* Draw pet preview on the selection screen (BG tiles 132-135). */
void pet_draw_preview(UINT8 pet_idx) BANKED;

/* Reset preview palette init flag (call when entering pet select). */
void pet_reset_preview(void) BANKED;

/* Draw character preview on the selection screen (BG tiles 128-131). */
void select_draw_char(UINT8 char_idx) BANKED;

/* Reset character preview palette init flag. */
void select_reset_char(void) BANKED;

/* Draw the credits/attribution screen text. */
void credits_draw(void) BANKED;

#endif
