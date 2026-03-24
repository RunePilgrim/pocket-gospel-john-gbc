/* outdoor_tiles.h — tile definitions for the outdoor courtyard map.
 *
 * All tiles from gameboy_outdoor_assets_v1.2.png.
 */

#ifndef OUTDOOR_TILES_H
#define OUTDOOR_TILES_H

#include <gb/gb.h>

#define OUTDOOR_TILE_BASE   98u
#define OUTDOOR_TILE_COUNT  70u

/* --- Grass: 24 tiles (cols 14-19, rows 4-7) --- */
#define OT_GRASS_00   0u
#define OT_GRASS_01   1u
#define OT_GRASS_02   2u
#define OT_GRASS_03   3u
#define OT_GRASS_04   4u
#define OT_GRASS_05   5u
#define OT_GRASS_06   6u
#define OT_GRASS_07   7u
#define OT_GRASS_08   8u
#define OT_GRASS_09   9u
#define OT_GRASS_10  10u
#define OT_GRASS_11  11u
#define OT_GRASS_12  12u
#define OT_GRASS_13  13u
#define OT_GRASS_14  14u
#define OT_GRASS_15  15u
#define OT_GRASS_16  16u
#define OT_GRASS_17  17u
#define OT_GRASS_18  18u
#define OT_GRASS_19  19u
#define OT_GRASS_20  20u
#define OT_GRASS_21  21u
#define OT_GRASS_22  22u
#define OT_GRASS_23  23u

/* --- Building: 16 tiles (cols 12-15, rows 0-3) --- */
#define OT_BLDG_R0C0 24u
#define OT_BLDG_R0C1 25u
#define OT_BLDG_R0C2 26u
#define OT_BLDG_R0C3 27u
#define OT_BLDG_R1C0 28u
#define OT_BLDG_R1C1 29u
#define OT_BLDG_R1C2 30u
#define OT_BLDG_R1C3 31u
#define OT_BLDG_R2C0 32u
#define OT_BLDG_R2C1 33u
#define OT_BLDG_R2C2 34u
#define OT_BLDG_R2C3 35u
#define OT_BLDG_R3C0 36u
#define OT_BLDG_R3C1 37u
#define OT_BLDG_R3C2 38u
#define OT_BLDG_R3C3 39u

/* --- Tree 1: 8 tiles (cols 4-5, rows 4-7) --- */
#define OT_TREE1_R0L 40u
#define OT_TREE1_R0R 41u
#define OT_TREE1_R1L 42u
#define OT_TREE1_R1R 43u
#define OT_TREE1_R2L 44u
#define OT_TREE1_R2R 45u
#define OT_TREE1_R3L 46u
#define OT_TREE1_R3R 47u

/* --- Tree 2: 6 tiles (cols 2-3, rows 5-7) --- */
#define OT_TREE2_R0L 48u
#define OT_TREE2_R0R 49u
#define OT_TREE2_R1L 50u
#define OT_TREE2_R1R 51u
#define OT_TREE2_R2L 52u
#define OT_TREE2_R2R 53u

/* --- Hedge: 7 tiles (cols 20-23, rows 4-8) --- */
#define OT_HEDGE_TL  54u
#define OT_HEDGE_TR  55u
#define OT_HEDGE_L   56u
#define OT_HEDGE_R   57u
#define OT_HEDGE_BL  58u
#define OT_HEDGE_B   59u
#define OT_HEDGE_BR  60u

/* --- Hedge top: 1 tile (21,4) --- */
#define OT_HEDGE_T   61u

/* --- Sky: 1 dark tile (0,24) --- */
#define OT_SKY       62u

/* --- Stars: 4 tiles with star patterns --- */
#define OT_STAR_A    63u
#define OT_STAR_B    64u
#define OT_STAR_C    65u
#define OT_STAR_D    66u

/* --- Bush: 3 tiles (col 11, rows 0-2) --- */
#define OT_BUSH_A    67u
#define OT_BUSH_B    68u
#define OT_BUSH_C    69u

extern const UINT8 outdoor_tiles_data[];

#define OUTDOOR_TILES_BANK 12u

#endif
