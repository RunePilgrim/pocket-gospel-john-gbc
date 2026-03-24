/* matching.h — memory matching card game interface (banked to bank 15). */

#ifndef MATCHING_H
#define MATCHING_H

#include <gb/gb.h>

/* Initialize/reset the matching game */
void matching_init(void) BANKED;

/* Update game logic from input. Returns 1 if player pressed B to exit. */
UINT8 matching_update(UBYTE pressed, UBYTE held) BANKED;

/* Render the matching game screen */
void matching_render(void) BANKED;

/* Check if the game is won (all pairs matched) */
UINT8 matching_is_won(void) BANKED;

#endif
