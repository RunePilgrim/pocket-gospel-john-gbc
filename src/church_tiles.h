/* church_tiles.h — tile definitions for the church interior map.
 * Tiles extracted from gameboy_outdoor_assets_v1.2.png.
 */

#ifndef CHURCH_TILES_H
#define CHURCH_TILES_H

#include <gb/gb.h>

/* Tile indices (offset from 0; loaded at CHURCH_TILE_BASE in VRAM) */
#define CHURCH_TILE_BASE   98u
#define CHURCH_TILE_COUNT  48u

/* Named tile offsets (add CHURCH_TILE_BASE to get VRAM index) */
#define CT_FLOOR_TL        0u   /* floor top-left (14,16) */
#define CT_FLOOR_TR        1u   /* floor top-right (15,16) */
#define CT_WALL_TOP        2u   /* top wall edge */
#define CT_WALL_MID        3u   /* middle wall fill */
#define CT_WALL_BOTTOM     4u   /* wall-to-floor transition */
#define CT_WALL_LEFT       5u   /* left wall edge */
#define CT_WALL_RIGHT      6u   /* right wall edge */
#define CT_WALL_CORNER_TL  7u   /* top-left corner */
#define CT_WALL_CORNER_TR  8u   /* top-right corner */
#define CT_PEW_LEFT        9u   /* pew left end */
#define CT_PEW_MID         10u  /* pew middle */
#define CT_PEW_RIGHT       11u  /* pew right end */
#define CT_PLANT_TL        12u  /* potted plant top-left (24,20) */
#define CT_PLANT_TR        13u  /* potted plant top-right (25,20) */
#define CT_PLANT_BL        14u  /* potted plant bottom-left (24,21) */
#define CT_PLANT_BR        15u  /* potted plant bottom-right (25,21) */
#define CT_BOOK_OPEN       16u  /* open book on altar */
#define CT_CANDLE_UNLIT    17u  /* candle holder (no flame) */
#define CT_CANDLE_LIT      18u  /* candle holder (flame) */
#define CT_ROPE_TOP        19u  /* bell rope top (attached to ceiling) */
#define CT_ROPE_MID        20u  /* bell rope middle */
#define CT_ROPE_BOTTOM     21u  /* bell rope handle/tassel */
#define CT_ROPE_PULLED     22u  /* rope pulled state */
#define CT_DOOR_TL         23u  /* doorway top-left */
#define CT_DOOR_TR         24u  /* doorway top-right */
#define CT_DOOR_BL         25u  /* doorway bottom-left (walkable) */
#define CT_DOOR_BR         26u  /* doorway bottom-right (walkable) */
#define CT_WINDOW_TL       27u  /* stained glass top-left */
#define CT_WINDOW_TR       28u  /* stained glass top-right */
#define CT_WINDOW_BL       29u  /* stained glass bottom-left */
#define CT_WINDOW_BR       30u  /* stained glass bottom-right */
#define CT_CARPET_V        31u  /* aisle carpet vertical */
#define CT_FLOOR_BR        32u  /* floor bottom-right (15,17) */
#define CT_TABLE_TL        33u  /* card table top-left */
#define CT_TABLE_TR        34u  /* card table top-right */
#define CT_TABLE_BL        35u  /* card table bottom-left */
#define CT_TABLE_BR        36u  /* card table bottom-right */
#define CT_CANDLE_TOP_UNLIT 37u  /* tall candle top (no flame) */
#define CT_CANDLE_TOP_LIT   38u  /* tall candle top (flame) */
#define CT_PILLAR_TOP      39u  /* stone pillar top */
#define CT_PILLAR_BOT      40u  /* stone pillar bottom */
#define CT_FLOOR_BL        41u  /* floor bottom-left (14,17) */
#define CT_WALL_DECO       42u  /* wall decoration */
#define CT_SHELF           43u  /* small shelf */
#define CT_PEW_MID_B       44u  /* pew middle on FTL floor variant */
#define CT_BENCH_R         45u  /* reserved */
#define CT_RAILING         46u  /* altar railing */
#define CT_RESERVED        47u  /* reserved */

/* Backward-compatible aliases */
#define CT_FLOOR_STONE     CT_FLOOR_TL
#define CT_FLOOR_DARK      CT_FLOOR_TR
#define CT_ALTAR_TL        CT_PLANT_TL
#define CT_ALTAR_TR        CT_PLANT_TR
#define CT_ALTAR_BL        CT_PLANT_BL
#define CT_ALTAR_BR        CT_PLANT_BR

extern const UINT8 church_tiles_data[];

#define CHURCH_TILES_BANK 11u

#endif
