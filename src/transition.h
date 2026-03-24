/* transition.h — palette fade-in/fade-out for screen transitions. */

#ifndef TRANSITION_H
#define TRANSITION_H

#include <gb/gb.h>

/* Fade speed: number of frames per fade step (lower = faster) */
#define FADE_STEP_FRAMES 3
#define FADE_STEPS       4

/* Start a fade-to-black */
void fade_out_start(void);

/* Start a fade-from-black */
void fade_in_start(void);

/* Tick the fade each frame. Returns 1 when complete, 0 if still fading. */
UINT8 fade_update(void);

/* Check if currently fading */
UINT8 fade_active(void);

/* Store current palettes as the "base" palettes to fade from/to */
void fade_set_base_bg_palette(UINT8 pal_idx, const UWORD colors[4]);

/* Store base sprite palette for fade system */
void fade_set_base_obj_palette(UINT8 pal_idx, const UWORD colors[4]);

/* Force all-black (call after fade_out completes, before loading new scene) */
void fade_to_black(void);

/* Force all-white */
void fade_to_white(void);

#endif
