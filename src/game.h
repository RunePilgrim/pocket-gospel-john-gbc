/* game.h — game state machine and transition management. */

#ifndef GAME_H
#define GAME_H

#include <gb/gb.h>

/* Game states */
#define STATE_BOOT         0
#define STATE_CHURCH       1
#define STATE_OUTDOORS     2
#define STATE_READER       3
#define STATE_MATCHING     4
#define STATE_CHAR_SELECT  5
#define STATE_PET_SELECT   6
#define STATE_CREDITS      7

/* Transition states */
#define TRANS_NONE      0
#define TRANS_FADE_OUT  1
#define TRANS_FADE_IN   2

void game_init(void);
void game_update(UBYTE pressed, UBYTE held);
void game_render(void);

/* Request a state change with fade transition */
void game_change_state(UINT8 new_state);

/* Get current state */
UINT8 game_get_state(void);

/* Check if a transition is in progress */
UINT8 game_is_transitioning(void);

#endif
