/*
 * player_sprites.h — 16x16 player character sprite definitions.
 * 8x8 sprite mode: each 16x16 frame = 4 OAM tiles.
 * 3 directions x 2 frames = 24 sprite tiles per character.
 * 16 selectable characters loaded from RPG spritesheet PNGs.
 * RIGHT reuses LEFT tiles with OAM X-flip.
 */

#ifndef PLAYER_SPRITES_H
#define PLAYER_SPRITES_H

#include <gb/gb.h>

#define PLAYER_SPRITE_TILES  24u  /* total 8x8 tiles stored in VRAM */
#define NUM_CHARACTERS       16u  /* number of selectable characters */

/* Direction indices */
#define DIR_DOWN   0u
#define DIR_UP     1u
#define DIR_LEFT   2u
#define DIR_RIGHT  3u

/* Load a specific character's sprite tiles into sprite VRAM */
void player_sprites_load_char(UINT8 char_idx);

/* Load a character's DOWN_F0 preview into BG tiles 128-131 */
void player_sprites_load_preview(UINT8 char_idx);

/* Backward-compatible: loads character 0 */
void player_sprites_load(void);

#endif
