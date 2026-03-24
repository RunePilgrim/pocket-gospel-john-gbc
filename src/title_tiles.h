/*
 * title_tiles.h — tile definitions for the title screen.
 * Pigeons, waterfall animation, and cross graphic.
 */

#ifndef TITLE_TILES_H
#define TITLE_TILES_H

#include <gb/gb.h>

/* Layout: pigeons at 98-105, waterfall at 106-121, cross at 122-127 */
#define TITLE_TILE_BASE        98u
#define TITLE_PIGEON_TILES     8u
#define TITLE_WATERFALL_TILES  16u
#define TITLE_CROSS_TILES      6u
#define TITLE_TILE_COUNT       30u   /* 8 pigeon + 16 waterfall + 6 cross */

/* --- Pigeon frame 1 (16x16, tiles 98-101) --- */
#define TITLE_PIG_TL           98u
#define TITLE_PIG_TR           99u
#define TITLE_PIG_BL          100u
#define TITLE_PIG_BR          101u

/* --- Pigeon frame 2 (16x16, tiles 102-105) --- */
#define TITLE_PIG2_TL         102u
#define TITLE_PIG2_TR         103u
#define TITLE_PIG2_BL         104u
#define TITLE_PIG2_BR         105u

/* --- Waterfall frame 1 (16x16, tiles 106-109) --- */
#define TITLE_WFALL_F1_TL     106u
#define TITLE_WFALL_F1_TR     107u
#define TITLE_WFALL_F1_BL     108u
#define TITLE_WFALL_F1_BR     109u

/* --- Waterfall frame 2 (16x16, tiles 110-113) --- */
#define TITLE_WFALL_F2_TL     110u
#define TITLE_WFALL_F2_TR     111u
#define TITLE_WFALL_F2_BL     112u
#define TITLE_WFALL_F2_BR     113u

/* --- Waterfall frame 3 (16x16, tiles 114-117) --- */
#define TITLE_WFALL_F3_TL     114u
#define TITLE_WFALL_F3_TR     115u
#define TITLE_WFALL_F3_BL     116u
#define TITLE_WFALL_F3_BR     117u

/* --- Waterfall frame 4 (16x16, tiles 118-121) --- */
#define TITLE_WFALL_F4_TL     118u
#define TITLE_WFALL_F4_TR     119u
#define TITLE_WFALL_F4_BL     120u
#define TITLE_WFALL_F4_BR     121u

/* --- Three-bar cross (16x24 = 2 wide x 3 tall, tiles 122-127) --- */
#define TITLE_CROSS_TL        122u
#define TITLE_CROSS_TR        123u
#define TITLE_CROSS_ML        124u   /* middle-left */
#define TITLE_CROSS_MR        125u   /* middle-right */
#define TITLE_CROSS_BL        126u
#define TITLE_CROSS_BR        127u

extern const UINT8 pigeon_tiles[];
extern const UINT8 waterfall_tiles[];
extern const UINT8 orthodox_cross_tiles[];

void title_tiles_load(void);

#endif
