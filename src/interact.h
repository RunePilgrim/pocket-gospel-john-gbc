/* interact.h — object interaction system for A-button actions (banked). */

#ifndef INTERACT_H
#define INTERACT_H

#include <gb/gb.h>

/* Interaction result codes */
#define INTERACT_NONE       0u
#define INTERACT_CANDLE     1u
#define INTERACT_READER     2u
#define INTERACT_BELL       3u
#define INTERACT_MATCHING   4u
#define INTERACT_DOOR_OUT   5u
#define INTERACT_DOOR_IN    6u

/* Initialize the interaction system */
void interact_init(void) BANKED;

/* Check for and handle A-button interaction.
   Returns the interaction result code (INTERACT_*). */
UINT8 interact_check(void) BANKED;

/* Update ongoing interaction animations (bell pull, etc.) */
void interact_update(void) BANKED;

/* Show a text prompt at the bottom of the screen */
void interact_show_prompt(const char *line1, const char *line2) BANKED;

/* Hide the prompt */
void interact_hide_prompt(void) BANKED;

/* Check if prompt is currently visible */
UINT8 interact_prompt_visible(void) BANKED;

#endif
