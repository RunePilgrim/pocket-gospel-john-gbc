/* pet_sprites.h -- companion pet sprite definitions.
 * 8x8 sprite mode: each 16x16 frame = 4 OAM tiles.
 * 2 frames per pet (idle + walk), 8 tiles total.
 * Generated data lives in bank 10 alongside player sprites.
 */

#ifndef PET_SPRITES_H
#define PET_SPRITES_H

#include <gb/gb.h>

#define NUM_PETS           9u
#define PET_SPRITE_TILES   8u   /* 2 frames x 4 quadrants */
#define PET_SPRITE_BASE    24u  /* first VRAM sprite tile for pet */

/* Load a pet's 8 sprite tiles into sprite VRAM at tile 24 */
void pet_sprites_load_pet(UINT8 pet_idx);

/* Load a pet's idle frame as BG tiles 132-135 for selection preview */
void pet_sprites_load_preview(UINT8 pet_idx);

#endif
