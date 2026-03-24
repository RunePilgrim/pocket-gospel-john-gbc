/*
 * map.c — map loading, rendering, collision, and object lookup.
 *
 * Two maps: church interior and outdoor courtyard.
 * Single-screen 20x18 tile maps, no scrolling.
 * Collision is a bitfield: 3 bytes/row, MSB-first.
 * CGB palette attributes go to VBK bank 1.
 * Banked to bank 15 to save home bank space.
 */

#pragma bank 15

#include <gb/gb.h>
#include <gb/cgb.h>
#include "map.h"
#include "church_tiles.h"
#include "outdoor_tiles.h"
#include "bank_helpers.h"

/* Church interior map (20x18).
   Walls/windows at top, altar+rope mid, pews/table/bench lower, door at bottom. */

/* Church tile shorthand for map readability */
#define _FLOOR     CT_FLOOR_STONE
#define _FLOORD    CT_FLOOR_DARK
#define _WTOP      CT_WALL_TOP
#define _WMID      CT_WALL_MID
#define _WBOT      CT_WALL_BOTTOM
#define _WLEFT     CT_WALL_LEFT
#define _WRIGHT    CT_WALL_RIGHT
#define _CORNRTL   CT_WALL_CORNER_TL
#define _CORNRTR   CT_WALL_CORNER_TR
#define _PEWL      CT_PEW_LEFT
#define _PEWM      CT_PEW_MID
#define _PEWR      CT_PEW_RIGHT
#define _ALTRTL    CT_ALTAR_TL
#define _ALTRTR    CT_ALTAR_TR
#define _ALTRBL    CT_ALTAR_BL
#define _ALTRBR    CT_ALTAR_BR
#define _BOOK      CT_BOOK_OPEN
#define _CNDL      CT_CANDLE_UNLIT
#define _ROPET     CT_ROPE_TOP
#define _ROPEM     CT_ROPE_MID
#define _ROPEB     CT_ROPE_BOTTOM
#define _DOORTL    CT_DOOR_TL
#define _DOORTR    CT_DOOR_TR
#define _DOORBL    CT_DOOR_BL
#define _DOORBR    CT_DOOR_BR
#define _WINTL     CT_WINDOW_TL
#define _WINTR     CT_WINDOW_TR
#define _WINBL     CT_WINDOW_BL
#define _WINBR     CT_WINDOW_BR
#define _CARPV     CT_CARPET_V
#define _CNDTOP    CT_CANDLE_TOP_UNLIT
#define _CNDTLT    CT_CANDLE_TOP_LIT
#define _PILTOP    CT_PILLAR_TOP
#define _PILBOT    CT_PILLAR_BOT
#define _RAIL      CT_RAILING
#define _FCARP     CT_FLOOR_CARPET
#define _TBLTL     CT_TABLE_TL
#define _TBLTR     CT_TABLE_TR
#define _TBLBL     CT_TABLE_BL
#define _TBLBR     CT_TABLE_BR
#define _BNCHL     CT_PEW_MID_B
#define _BNCHR     CT_BENCH_R
#define _FTL       CT_FLOOR_TL
#define _FTR       CT_FLOOR_TR
#define _FBL       CT_FLOOR_BL
#define _FBR       CT_FLOOR_BR

static const UINT8 church_map_tiles[MAP_W * MAP_ROWS] = {
    /* Row 0:  top wall (no stained glass on back wall) */
    _CORNRTL, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _WTOP, _CORNRTR,
    /* Row 1:  plain wall (cross removed) */
    _WLEFT, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WRIGHT,
    /* Row 2:  left window (cols 3-6) + right window (cols 13-16) — 4 tiles wide */
    _WLEFT, _WMID, _WMID, _WINTL, _WINTR, _WINBL, _WINBR, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WINTL, _WINTR, _WINBL, _WINBR, _WMID, _WMID, _WRIGHT,
    /* Row 3:  left window bottom + right window bottom */
    _WLEFT, _WMID, _WMID, _WINBR, _WINTL, _WINTR, _WINBL, _WMID, _WMID, _WMID, _WMID, _WMID, _WMID, _WINBR, _WINTL, _WINTR, _WINBL, _WMID, _WMID, _WRIGHT,
    /* Row 4:  wall base + candle top at col 2 + rope top at col 16 */
    _WLEFT, _WBOT, _CNDTOP, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _ROPET, _WBOT, _WBOT, _WRIGHT,
    /* Row 5:  candle(2), altar top with book (9-10), rope mid (16) */
    _WLEFT, _FBR, _CNDL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _ALTRTL, _ALTRTR, _FBR, _FBL, _FBR, _FBL, _FBR, _ROPEM, _FBR, _FBL, _WRIGHT,
    /* Row 6:  altar base (9-10), rope bottom handle (16) */
    _WLEFT, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _ALTRBL, _ALTRBR, _FTL, _FTR, _FTL, _FTR, _FTL, _ROPEB, _FTL, _FTR, _WRIGHT,
    /* Row 7:  carpet starts */
    _WLEFT, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _CARPV, _CARPV, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _WRIGHT,
    /* Row 8:  open floor + carpet */
    _WLEFT, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _CARPV, _CARPV, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _WRIGHT,
    /* Row 9:  open floor + carpet */
    _WLEFT, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _CARPV, _CARPV, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _WRIGHT,
    /* Row 10: open floor + carpet */
    _WLEFT, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _CARPV, _CARPV, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _WRIGHT,
    /* Row 11: open floor + carpet */
    _WLEFT, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _CARPV, _CARPV, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _WRIGHT,
    /* Row 12: open floor + carpet */
    _WLEFT, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _CARPV, _CARPV, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _FTL, _FTR, _WRIGHT,
    /* Row 13: card table (cols 2-3) + carpet */
    _WLEFT, _FBR, _TBLTL, _TBLTR, _FBL, _FBR, _FBL, _FBR, _FBL, _CARPV, _CARPV, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _WRIGHT,
    /* Row 14: card table legs + bench/pew cols 13-17 */
    _WLEFT, _FTL, _TBLBL, _TBLBR, _FTR, _FTL, _FTR, _FTL, _FTR, _CARPV, _CARPV, _FTL, _FTR, _PEWL, _PEWM, _BNCHL, _PEWM, _PEWR, _FTR, _WRIGHT,
    /* Row 15: floor before door + carpet */
    _WLEFT, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _CARPV, _CARPV, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _FBR, _FBL, _WRIGHT,
    /* Row 16: door frame (cols 9-10 walkable) */
    _WLEFT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _DOORTL, _DOORTR, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WRIGHT,
    /* Row 17: door base (cols 9-10 walkable) */
    _WLEFT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _DOORBL, _DOORBR, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WBOT, _WRIGHT
};

/* Church collision: 1=solid, 0=walkable
   MSB-first packing: 3 bytes per row, 18 rows = 54 bytes.
   byte_idx = y*3 + (x>>3), bit_mask = 0x80 >> (x & 7).
   Col 0 = byte0 bit7, Col 7 = byte0 bit0, Col 8 = byte1 bit7, etc.
   Col 16..19 = byte2 bits 7..4.
*/
static const UINT8 church_collision[54] = {
    /* Row 0: all solid (top wall) */
    0xFF, 0xFF, 0xF0,
    /* Row 1: all solid (wall + cross) */
    0xFF, 0xFF, 0xF0,
    /* Row 2: all solid (wall + windows + cross) */
    0xFF, 0xFF, 0xF0,
    /* Row 3: all solid (wall + windows bottom) */
    0xFF, 0xFF, 0xF0,
    /* Row 4: all solid (wall base + railing) */
    0xFF, 0xFF, 0xF0,
    /* Row 5: WL . CU . . . . . . AL AR . . . . . RT . . WR */
    /*        S  . S  . . . . . . S  S  . . . . . S  . . S  */
    0xA0, 0x60, 0x90,
    /* Row 6: WL . . . . . . . . AB AQ . . . . . RM . . WR */
    /*        S  . . . . . . . . S  S  . . . . . S  . . S  */
    0x80, 0x60, 0x90,
    /* Row 7: WL floor + carpet + rope bottom (walkable) */
    0x80, 0x00, 0x10,
    /* Row 8-12: open floor + carpet (walls only solid) */
    0x80, 0x00, 0x10,
    0x80, 0x00, 0x10,
    0x80, 0x00, 0x10,
    0x80, 0x00, 0x10,
    0x80, 0x00, 0x10,
    /* Row 13: WL . TB TC . . . . . . . . . . . . . . . WR */
    /*         S  . S  S  . . . . . . . . . . . . . . . S  */
    0xB0, 0x00, 0x10,
    /* Row 14: WL . TD TE . . . . . . . . . BN BN BN BN BR . WR */
    /*         S  . S  S  . . . . . . . . . S  S  S  S  S  . S  */
    0xB0, 0x07, 0xD0,
    /* Row 15: floor before door (walls only) */
    0x80, 0x00, 0x10,
    /* Row 16: door frame, cols 9-10 walkable */
    0xFF, 0x9F, 0xF0,
    /* Row 17: door base, cols 9-10 walkable */
    0xFF, 0x9F, 0xF0
};

/* Church palette attributes (simplified: most tiles use pal 0) */
static const UINT8 church_pal_attrs[MAP_W * MAP_ROWS] = {
    /* Row 0:  plain wall */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* Row 1:  wall + cross(pal0 - stone/dark brown, NOT yellow) */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* Row 2:  left window(pal2, cols 3-6) + right window(pal2, cols 13-16) */
    0,0,0,2,2,2,2,0,0,0,0,0,0,2,2,2,2,0,0,0,
    /* Row 3:  left window bottom(pal2) + right window bottom(pal2) */
    0,0,0,2,2,2,2,0,0,0,0,0,0,2,2,2,2,0,0,0,
    /* Row 4:  wall base + candle top(pal2, col 2) + rope top (col 16 pal2) */
    0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,
    /* Row 5:  candle(pal2) altar(pal1) rope(pal2) */
    0,0,2,0,0,0,0,0,0,1,1,0,0,0,0,0,2,0,0,0,
    /* Row 6:  book(pal1) altar base right(pal1) rope(pal2) */
    0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,2,0,0,0,
    /* Row 7:  carpet(pal2) - no rope here anymore */
    0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 8:  carpet only */
    0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 9:  carpet only */
    0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 10: carpet only */
    0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 11: carpet only */
    0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 12: carpet only */
    0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 13: card table(pal1) + carpet */
    0,0,1,1,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 14: card table legs(pal1) + bench(pal1) + carpet */
    0,0,1,1,0,0,0,0,0,2,2,0,0,1,1,1,1,1,0,0,
    /* Row 15: carpet only */
    0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,
    /* Row 16: door area */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* Row 17: door area */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* Outdoor courtyard map (20x18).
   Hedge border, building at top center, two tree types, 24 grass variants.
   Building door at row 3 cols 9-10 = church entrance.
   All tiles from gameboy_outdoor_assets_v1.2.png. */

/* Building shorthand */
#define _B00 OT_BLDG_R0C0
#define _B01 OT_BLDG_R0C1
#define _B02 OT_BLDG_R0C2
#define _B03 OT_BLDG_R0C3
#define _B10 OT_BLDG_R1C0
#define _B11 OT_BLDG_R1C1
#define _B12 OT_BLDG_R1C2
#define _B13 OT_BLDG_R1C3
#define _B20 OT_BLDG_R2C0
#define _B21 OT_BLDG_R2C1
#define _B22 OT_BLDG_R2C2
#define _B23 OT_BLDG_R2C3
#define _B30 OT_BLDG_R3C0
#define _B31 OT_BLDG_R3C1
#define _B32 OT_BLDG_R3C2
#define _B33 OT_BLDG_R3C3
/* Tree 1 shorthand (2x4) */
#define _1TL OT_TREE1_R0L
#define _1TR OT_TREE1_R0R
#define _1ML OT_TREE1_R1L
#define _1MR OT_TREE1_R1R
#define _1BL OT_TREE1_R2L
#define _1BR OT_TREE1_R2R
#define _1FL OT_TREE1_R3L
#define _1FR OT_TREE1_R3R
/* Tree 2 shorthand (2x3) */
#define _2TL OT_TREE2_R0L
#define _2TR OT_TREE2_R0R
#define _2ML OT_TREE2_R1L
#define _2MR OT_TREE2_R1R
#define _2BL OT_TREE2_R2L
#define _2BR OT_TREE2_R2R
/* Hedge shorthand */
#define _HTL OT_HEDGE_TL
#define _HTR OT_HEDGE_TR
#define _HL  OT_HEDGE_L
#define _HR  OT_HEDGE_R
#define _HBL OT_HEDGE_BL
#define _HB  OT_HEDGE_B
#define _HBR OT_HEDGE_BR
#define _HT  OT_HEDGE_T
/* Bush shorthand */
#define _BA OT_BUSH_A
#define _BB OT_BUSH_B
#define _BC OT_BUSH_C

static const UINT8 outdoor_map_tiles[MAP_W * MAP_ROWS] = {
    /* Row 0:  top hedge + building row 0 at cols 8-11 */
    _HTL, _HT, _HT, _HT, _HT, _HT, _HT, _HT, _B00, _B01, _B02, _B03, _HT, _HT, _HT, _HT, _HT, _HT, _HT, _HTR,
    /* Row 1:  building row 1 + randomized grass */
    _HL, 18, 21, 15, 18, 12, 15, 16, _B10, _B11, _B12, _B13, 14,  7, 11, 12,  8,  9, 13, _HR,
    /* Row 2:  building row 2 */
    _HL,  8,  4,  2, 11,  9,  5,  4, _B20, _B21, _B22, _B23,  6,  5,  1, 23,  0, 23, 19, _HR,
    /* Row 3:  building row 3 (door at cols 9-10) */
    _HL,  3, 20, 21,  4, 21, 22, 15, _B30, _B31, _B32, _B33, 23, 16, 18, 11, 17, 18, 11, _HR,
    /* Row 4:  tree 1 crowns at cols 2-3 and 16-17 */
    _HL, 17, _1TL, _1TR,  7,  6,  4,  3, 14, 12, 11,  9,  3,  1,  0, 22, _1TL, _1TR,  6, _HR,
    /* Row 5:  tree 1 mids */
    _HL,  4, _1ML, _1MR,  0, 17, 21, 22,  6,  0,  3,  5, 19, 13, 16, 18, _1ML, _1MR, 23, _HR,
    /* Row 6:  tree 1 lower canopy */
    _HL,  2, _1BL, _1BR, 16, 15, 11,  9, 23, 22, 18, 16, 12, 10,  6,  5, _1BL, _1BR, 13, _HR,
    /* Row 7:  tree 1 trunk bases */
    _HL, 13, _1FL, _1FR,  9,  2,  4, 21,  8,  9,  2,  3,  5, 22, 23, 16, _1FL, _1FR, 22, _HR,
    /* Row 8:  grass + bushes */
    _HL,  3, 10,  8, _BA,  5,  3,  2,  7, 13, 11, 10,  8, _BB,  5,  4, 20, 18, 17, _HR,
    /* Row 9:  grass + bushes */
    _HL, 23, 18, 19, _BC, _BA, 20, 13, 23,  0,  4, 21, _BB, _BC, 22, 23, 12, 14,  9, _HR,
    /* Row 10: tree 2 crowns at cols 5-6 and 13-14 */
    _HL, 12,  8,  7,  8, _2TL, _2TR,  9, 16, 14, 10,  9, 18, _2TL, _2TR, 10,  5,  3, 23, _HR,
    /* Row 11: tree 2 mids */
    _HL,  0,  1, 18,  0, _2ML, _2MR, 20,  8, 10,  3,  4,  2, _2ML, _2MR, 22, 22, 15, 16, _HR,
    /* Row 12: tree 2 trunk bases */
    _HL, 22, 20, 18, 12, _2BL, _2BR,  7,  1, 23, 22, 20, 14, _2BL, _2BR,  9, 14, 13, 11, _HR,
    /* Row 13: grass + bushes */
    _HL,  9, _BA,  6,  5, 22,  2, 19, 18, _BC, 14,  8,  6,  0,  3,  5, _BB,  0,  4, _HR,
    /* Row 14: grass + bushes */
    _HL, 23, 19, _BC, 13, 12,  8, 14,  2,  9, _BA,  3, 23, 22, 18, _BA, 15, 14, 10, _HR,
    /* Row 15: grass + bushes */
    _HL, 18, 11, 13,  6,  7,  0, _BB, 19, 20, 13, 14, _BC,  9,  2,  3,  8,  9,  2, _HR,
    /* Row 16: grass */
    _HL,  8,  6,  5,  3, 10,  8,  7,  5,  3,  2,  0, 23,  5,  4,  2, 13, 12, 18, _HR,
    /* Row 17: bottom hedge */
    _HBL, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HB, _HBR
};

/* Outdoor collision: MSB-first packing, same scheme as church.
   Solid: hedges, building, trees. Walkable: grass, building door (cols 9-10, row 3). */
static const UINT8 outdoor_collision[54] = {
    /* Row 0:  all solid (hedge + building top) */
    0xFF, 0xFF, 0xF0,
    /* Row 1:  hedge L(0) + building(8-11) + hedge R(19) */
    0x80, 0xF0, 0x10,
    /* Row 2:  same */
    0x80, 0xF0, 0x10,
    /* Row 3:  building door: cols 8-11 all walkable (wide entry) */
    0x80, 0x00, 0x10,
    /* Row 4:  tree 1 at cols 2-3 and 16-17 (crown) */
    0xB0, 0x00, 0xD0,
    /* Row 5:  tree 1 mids */
    0xB0, 0x00, 0xD0,
    /* Row 6:  tree 1 lower */
    0xB0, 0x00, 0xD0,
    /* Row 7:  tree 1 trunk bases */
    0xB0, 0x00, 0xD0,
    /* Row 8:  hedge L/R + bushes at cols 4, 13 */
    0x88, 0x04, 0x10,
    /* Row 9:  hedge L/R + bushes at cols 4, 5, 12, 13 */
    0x8C, 0x0C, 0x10,
    /* Row 10: tree 2 at cols 5-6 and 13-14 (crown) */
    0x86, 0x06, 0x10,
    /* Row 11: tree 2 mids */
    0x86, 0x06, 0x10,
    /* Row 12: tree 2 trunk bases */
    0x86, 0x06, 0x10,
    /* Row 13: hedge L/R + bushes at cols 2, 9, 16 */
    0xA0, 0x40, 0x90,
    /* Row 14: hedge L/R + bushes at cols 3, 10, 15 */
    0x90, 0x21, 0x10,
    /* Row 15: hedge L/R + bushes at cols 7, 12 */
    0x81, 0x08, 0x10,
    /* Row 16: hedge L/R only */
    0x80, 0x00, 0x10,
    /* Row 17: all solid (bottom hedge) */
    0xFF, 0xFF, 0xF0
};

/* Palette attributes: pal 0=green (grass/trees/hedge), pal 1=stone (building) */
static const UINT8 outdoor_pal_attrs[MAP_W * MAP_ROWS] = {
    /* Row 0:  building cols 8-11 = pal 1 (stone) */
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    /* Row 1:  building cols 8-11 = pal 1 */
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    /* Row 2:  building cols 8-11 = pal 1 */
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    /* Row 3:  building cols 8-11 = pal 1 (door row) */
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    /* Rows 4-17: all pal 0 (green) */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* Undefine church tile shorthand */
#undef _FLOOR
#undef _FLOORD
#undef _WTOP
#undef _WMID
#undef _WBOT
#undef _WLEFT
#undef _WRIGHT
#undef _CORNRTL
#undef _CORNRTR
#undef _PEWL
#undef _PEWM
#undef _PEWR
#undef _ALTRTL
#undef _ALTRTR
#undef _ALTRBL
#undef _ALTRBR
#undef _BOOK
#undef _CNDL
#undef _ROPET
#undef _ROPEM
#undef _ROPEB
#undef _DOORTL
#undef _DOORTR
#undef _DOORBL
#undef _DOORBR
#undef _WINTL
#undef _WINTR
#undef _WINBL
#undef _WINBR
#undef _CARPV
#undef _CNDTOP
#undef _CNDTLT
#undef _PILTOP
#undef _PILBOT
#undef _RAIL
#undef _FCARP
#undef _TBLTL
#undef _TBLTR
#undef _TBLBL
#undef _TBLBR
#undef _BNCHL
#undef _BNCHR
#undef _FTL
#undef _FTR
#undef _FBL
#undef _FBR

/* Undefine outdoor tile shorthand */
#undef _B00
#undef _B01
#undef _B02
#undef _B03
#undef _B10
#undef _B11
#undef _B12
#undef _B13
#undef _B20
#undef _B21
#undef _B22
#undef _B23
#undef _B30
#undef _B31
#undef _B32
#undef _B33
#undef _1TL
#undef _1TR
#undef _1ML
#undef _1MR
#undef _1BL
#undef _1BR
#undef _1FL
#undef _1FR
#undef _2TL
#undef _2TR
#undef _2ML
#undef _2MR
#undef _2BL
#undef _2BR
#undef _HTL
#undef _HTR
#undef _HL
#undef _HR
#undef _HBL
#undef _HB
#undef _HBR
#undef _HT
#undef _BA
#undef _BB
#undef _BC

/* Sky scroll: 2 rows of night sky written to VRAM rows 30-31 (above map via wrap).
   SCY_REG scrolls viewport up to reveal sky when player is near the top hedge. */

/* 8 rows of night sky (VRAM rows 24-31), scattered stars on dark background.
   Max scroll reveals 64 pixels of sky — nearly half the screen. */
#define SKY_ROWS     8u
#define SKY_START   24u    /* first VRAM row for sky */
#define SKY_MAX_PX  64u    /* 8 rows × 8 px = 64 px max scroll */
#define _S OT_SKY
#define _A OT_STAR_A
#define _B OT_STAR_B
#define _C OT_STAR_C
#define _D OT_STAR_D

static const UINT8 sky_tiles[SKY_ROWS][MAP_W] = {
    /* Row 24 (top) */ {_S,_S,_S,_B,_S,_S,_S,_S,_S,_S,_S,_S,_S,_S,_C,_S,_S,_S,_S,_S},
    /* Row 25 */       {_S,_S,_S,_S,_S,_S,_S,_D,_S,_S,_S,_S,_S,_S,_S,_S,_S,_S,_B,_S},
    /* Row 26 */       {_S,_S,_A,_S,_S,_S,_S,_S,_S,_S,_S,_C,_S,_S,_S,_S,_D,_S,_S,_S},
    /* Row 27 */       {_S,_S,_S,_S,_S,_B,_S,_S,_S,_D,_S,_S,_S,_S,_S,_C,_S,_S,_S,_S},
    /* Row 28 */       {_S,_C,_S,_S,_S,_S,_S,_S,_B,_S,_S,_S,_S,_A,_S,_S,_S,_S,_S,_S},
    /* Row 29 */       {_S,_S,_S,_S,_D,_S,_S,_S,_S,_S,_B,_S,_S,_S,_S,_S,_S,_C,_S,_S},
    /* Row 30 */       {_S,_S,_A,_S,_S,_S,_C,_S,_S,_S,_S,_S,_S,_B,_S,_S,_S,_S,_D,_S},
    /* Row 31 (bottom)*/{_S,_D,_S,_S,_S,_S,_S,_B,_S,_S,_C,_S,_S,_S,_S,_A,_S,_S,_S,_S},
};

#undef _S
#undef _A
#undef _B
#undef _C
#undef _D

static UINT8 sky_offset;  /* 0=normal, up to 64=full 8-row sky reveal */

/* Map data */

static MapData church_data;
static MapData outdoor_data;
static MapData *current_map;
static UINT8 current_map_id;

static void init_church_map(void) {
    church_data.tiles = church_map_tiles;
    church_data.collision = church_collision;
    church_data.pal_attrs = church_pal_attrs;
    church_data.tile_base = CHURCH_TILE_BASE;
    church_data.spawn_x = 9;
    church_data.spawn_y = 14;

    church_data.obj_count = 10;

    church_data.objects[0].x = 2;     /* candle stand (top-left area) */
    church_data.objects[0].y = 5;
    church_data.objects[0].type = OBJ_CANDLE;
    church_data.objects[0].state = 0; /* unlit */

    church_data.objects[1].x = 9;     /* altar top-left with book (row 5) */
    church_data.objects[1].y = 5;
    church_data.objects[1].type = OBJ_ALTAR_BOOK;
    church_data.objects[1].state = 0;

    church_data.objects[2].x = 10;    /* altar top-right with book (row 5) */
    church_data.objects[2].y = 5;
    church_data.objects[2].type = OBJ_ALTAR_BOOK;
    church_data.objects[2].state = 0;

    church_data.objects[3].x = 9;     /* altar base-left (row 6) */
    church_data.objects[3].y = 6;
    church_data.objects[3].type = OBJ_ALTAR_BOOK;
    church_data.objects[3].state = 0;

    church_data.objects[4].x = 10;    /* altar base-right (row 6) */
    church_data.objects[4].y = 6;
    church_data.objects[4].type = OBJ_ALTAR_BOOK;
    church_data.objects[4].state = 0;

    church_data.objects[5].x = 16;    /* bell rope bottom (row 6, from ceiling) */
    church_data.objects[5].y = 6;
    church_data.objects[5].type = OBJ_BELL_ROPE;
    church_data.objects[5].state = 0;

    church_data.objects[6].x = 2;     /* card table left tile (bottom-left) */
    church_data.objects[6].y = 13;
    church_data.objects[6].type = OBJ_CARD_TABLE;
    church_data.objects[6].state = 0;

    church_data.objects[7].x = 3;     /* card table right tile */
    church_data.objects[7].y = 13;
    church_data.objects[7].type = OBJ_CARD_TABLE;
    church_data.objects[7].state = 0;

    church_data.objects[8].x = 9;     /* door exit left */
    church_data.objects[8].y = 17;
    church_data.objects[8].type = OBJ_DOOR_OUT;
    church_data.objects[8].state = 0;

    church_data.objects[9].x = 10;    /* door exit right */
    church_data.objects[9].y = 17;
    church_data.objects[9].type = OBJ_DOOR_OUT;
    church_data.objects[9].state = 0;
}

static void init_outdoor_map(void) {
    outdoor_data.tiles = outdoor_map_tiles;
    outdoor_data.collision = outdoor_collision;
    outdoor_data.pal_attrs = outdoor_pal_attrs;
    outdoor_data.tile_base = OUTDOOR_TILE_BASE;
    outdoor_data.spawn_x = 9;
    outdoor_data.spawn_y = 5;   /* two rows below building door */

    outdoor_data.obj_count = 2;

    outdoor_data.objects[0].x = 9;     /* building door left (row 3, cols 9-10) */
    outdoor_data.objects[0].y = 3;
    outdoor_data.objects[0].type = OBJ_DOOR_IN;
    outdoor_data.objects[0].state = 0;

    outdoor_data.objects[1].x = 10;    /* building door right */
    outdoor_data.objects[1].y = 3;
    outdoor_data.objects[1].type = OBJ_DOOR_IN;
    outdoor_data.objects[1].state = 0;
}

void map_load(UINT8 map_id) BANKED {
    current_map_id = map_id;

    if (map_id == MAP_CHURCH) {
        init_church_map();
        load_bkg_data_banked(CHURCH_TILES_BANK, CHURCH_TILE_BASE, CHURCH_TILE_COUNT, church_tiles_data);
        current_map = &church_data;
    } else {
        init_outdoor_map();
        load_bkg_data_banked(OUTDOOR_TILES_BANK, OUTDOOR_TILE_BASE, OUTDOOR_TILE_COUNT, outdoor_tiles_data);
        current_map = &outdoor_data;
    }
}

void map_draw(void) BANKED {
    UINT8 x, y;
    unsigned char row_buf[MAP_W];

    if (!current_map) return;

    /* Reset scroll */
    sky_offset = 0;
    SCY_REG = 0;

    /* Write tile indices to BG map */
    for (y = 0; y < MAP_ROWS; y++) {
        for (x = 0; x < MAP_W; x++) {
            row_buf[x] = current_map->tile_base + current_map->tiles[y * MAP_W + x];
        }
        set_bkg_tiles(0, y, MAP_W, 1, row_buf);
    }

    /* Write CGB palette attributes */
    if (_cpu == CGB_TYPE && current_map->pal_attrs) {
        VBK_REG = 1;
        for (y = 0; y < MAP_ROWS; y++) {
            set_bkg_tiles(0, y, MAP_W, 1, &current_map->pal_attrs[y * MAP_W]);
        }
        VBK_REG = 0;
    }

    /* For outdoor map: write 8 rows of night sky to VRAM rows 24-31 */
    if (current_map_id == MAP_OUTDOORS) {
        UINT8 r;
        unsigned char sky_pal_row[MAP_W];

        for (r = 0; r < SKY_ROWS; r++) {
            for (x = 0; x < MAP_W; x++) {
                row_buf[x] = current_map->tile_base + sky_tiles[r][x];
            }
            set_bkg_tiles(0, (UINT8)(SKY_START + r), MAP_W, 1, row_buf);
        }

        /* Sky palette attributes: palette 2 (blue) for all sky rows */
        if (_cpu == CGB_TYPE) {
            for (x = 0; x < MAP_W; x++) {
                sky_pal_row[x] = 2;
            }
            VBK_REG = 1;
            for (r = 0; r < SKY_ROWS; r++) {
                set_bkg_tiles(0, (UINT8)(SKY_START + r), MAP_W, 1, sky_pal_row);
            }
            VBK_REG = 0;
        }
    }

    /* For outdoor map: set initial sky offset based on spawn pixel position */
    if (current_map_id == MAP_OUTDOORS && current_map) {
        UINT8 spawn_py = (UINT8)(current_map->spawn_y * 8u);
        if (spawn_py < 112u) {
            sky_offset = (UINT8)((112u - spawn_py) >> 1);
            if (sky_offset > SKY_MAX_PX) sky_offset = SKY_MAX_PX;
        }
        SCY_REG = (UINT8)(0u - sky_offset);
    }
}

UINT8 map_is_solid(UINT8 tx, UINT8 ty) BANKED {
    UINT8 byte_idx;
    UINT8 bit_mask;

    if (!current_map) return 1;
    if (tx >= MAP_W || ty >= MAP_ROWS) return 1;

    /* 3 bytes per row, MSB-first: bit 7 of byte 0 = column 0 */
    byte_idx = (UINT8)(ty * 3u + (tx >> 3u));
    bit_mask = (UINT8)(0x80u >> (tx & 7u));

    return (current_map->collision[byte_idx] & bit_mask) ? 1u : 0u;
}

MapObject *map_get_object(UINT8 tx, UINT8 ty) BANKED {
    UINT8 i;
    if (!current_map) return 0;

    for (i = 0; i < current_map->obj_count; i++) {
        if (current_map->objects[i].x == tx && current_map->objects[i].y == ty) {
            return &current_map->objects[i];
        }
    }
    return 0;
}

MapData *map_get_current(void) BANKED {
    return current_map;
}

UINT8 map_get_current_id(void) BANKED {
    return current_map_id;
}

void map_set_tile(UINT8 tx, UINT8 ty, UINT8 tile_offset) BANKED {
    unsigned char tile;

    if (!current_map) return;
    if (tx >= MAP_W || ty >= MAP_ROWS) return;

    tile = current_map->tile_base + tile_offset;
    set_bkg_tiles(tx, ty, 1, 1, &tile);
}

void map_update_scroll(UINT8 player_py, UBYTE held) BANKED {
    UINT8 target;
    (void)held;

    if (current_map_id != MAP_OUTDOORS) {
        sky_offset = 0;
        SCY_REG = 0;
        return;
    }

    /* Pixel-based smooth scroll: target proportional to player pixel Y.
       py=0 (top edge) → max sky, py≥112 (tile 14+) → no sky.
       Division by 2 gives ~0.5px scroll per 1px movement = very smooth. */
    if (player_py >= 112u) {
        target = 0;
    } else {
        target = (UINT8)((112u - player_py) >> 1);
        if (target > SKY_MAX_PX) target = SKY_MAX_PX;
    }

    /* Set directly — pixel position already provides per-pixel smoothness */
    sky_offset = target;
    SCY_REG = (UINT8)(0u - sky_offset);
}

UINT8 map_get_scroll_y(void) BANKED {
    return sky_offset;
}
