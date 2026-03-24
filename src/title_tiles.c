/*
 * title_tiles.c — 2bpp tile data for the title screen.
 * Pigeons (8 tiles), waterfall (16 tiles), cross (6 tiles).
 */

#include <gb/gb.h>
#include <gbdk/platform.h>
#include "title_tiles.h"

#pragma bank 10

BANKREF(pigeon_tiles)

/*
 * PIGEON PIXEL ART - 16x16, facing right
 *
 * Frame 1 (sitting pigeon, facing right):
 * Legend: .=c0(cream) 1=c1(tan) 2=c2(green) 3=c3(black)
 *
 *   TL tile (cols 0-7)    TR tile (cols 8-15)
 * Row 0:  ........         ........
 * Row 1:  ........         ..333...
 * Row 2:  ........         .31113..
 * Row 3:  ........         .31113..
 * Row 4:  .....333         331113..
 * Row 5:  ...33223         33111333
 * Row 6:  ..32222.         .33113..
 * Row 7:  .322222.         ..3333..
 *
 *   BL tile (cols 0-7)    BR tile (cols 8-15)
 * Row 0:  32222223         32222...
 * Row 1:  32222223         32222...
 * Row 2:  .3222223         32222...
 * Row 3:  .3222223         32222...
 * Row 4:  ..322223         32223...
 * Row 5:  ..322223         3223....
 * Row 6:  ...33..3         33.33...
 * Row 7:  ...33..3         33.33...
 */

const UINT8 pigeon_tiles[] = {
    /* === Frame 1 === */

    /* TITLE_PIG_TL (tile 0): pigeon frame 1, top-left */
    /* Row 0: ........  colors: 00000000 -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 1: ........  colors: 00000000 -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 2: ........  colors: 00000000 -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 3: ........  colors: 00000000 -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 4: .....333  colors: 00000333 -> lo=0x07 hi=0x07 */
    0x07, 0x07,
    /* Row 5: ...33223  colors: 00033223 -> lo=0x1B hi=0x0D */
    0x1B, 0x0D,
    /* Row 6: ..32222.  colors: 00322220 -> lo=0x20 hi=0x3E */
    0x20, 0x3E,
    /* Row 7: .322222.  colors: 03222220 -> lo=0x40 hi=0x7E */
    0x40, 0x7E,

    /* TITLE_PIG_TR (tile 1): pigeon frame 1, top-right */
    /* Row 0: ........  colors: 00000000 -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 1: ..333...  colors: 00333000 -> lo=0x38 hi=0x38 */
    0x38, 0x38,
    /* Row 2: .31113..  colors: 03111300 -> lo=0x7C hi=0x44 */
    0x7C, 0x44,
    /* Row 3: .31113..  colors: 03111300 -> lo=0x7C hi=0x44 */
    0x7C, 0x44,
    /* Row 4: 331113..  colors: 33111300 -> lo=0xFC hi=0xC4 */
    0xFC, 0xC4,
    /* Row 5: 33111333  colors: 33111333 -> lo=0xFF hi=0xC7 */
    0xFF, 0xC7,
    /* Row 6: .33113..  colors: 03311300 -> lo=0x7C hi=0x6C */
    0x7C, 0x6C,
    /* Row 7: ..3333..  colors: 00333300 -> lo=0x3C hi=0x3C */
    0x3C, 0x3C,

    /* TITLE_PIG_BL (tile 2): pigeon frame 1, bottom-left */
    /* Row 0: 32222223  colors: 32222223 -> lo=0x81 hi=0xFF */
    0x81, 0xFF,
    /* Row 1: 32222223  colors: 32222223 -> lo=0x81 hi=0xFF */
    0x81, 0xFF,
    /* Row 2: .3222223  colors: 03222223 -> lo=0x41 hi=0x7F */
    0x41, 0x7F,
    /* Row 3: .3222223  colors: 03222223 -> lo=0x41 hi=0x7F */
    0x41, 0x7F,
    /* Row 4: ..322223  colors: 00322223 -> lo=0x21 hi=0x3F */
    0x21, 0x3F,
    /* Row 5: ..322223  colors: 00322223 -> lo=0x21 hi=0x3F */
    0x21, 0x3F,
    /* Row 6: ...33..3  colors: 00033003 -> lo=0x19 hi=0x19 */
    0x19, 0x19,
    /* Row 7: ...33..3  colors: 00033003 -> lo=0x19 hi=0x19 */
    0x19, 0x19,

    /* TITLE_PIG_BR (tile 3): pigeon frame 1, bottom-right */
    /* Row 0: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,
    /* Row 1: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,
    /* Row 2: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,
    /* Row 3: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,
    /* Row 4: 32223...  colors: 32223000 -> lo=0x90 hi=0xF8 */
    0x90, 0xF8,
    /* Row 5: 3223....  colors: 32230000 -> lo=0xB0 hi=0xF0 */
    0xB0, 0xF0,
    /* Row 6: 33.33...  colors: 33033000 -> lo=0xD8 hi=0xD8 */
    0xD8, 0xD8,
    /* Row 7: 33.33...  colors: 33033000 -> lo=0xD8 hi=0xD8 */
    0xD8, 0xD8,

    /* === Frame 2 (slight wing/head shift) === */
    /*
     * Frame 2: pigeon with head tilted up, wing slightly raised
     *
     *   TL tile (cols 0-7)    TR tile (cols 8-15)
     * Row 0:  ........         .333....
     * Row 1:  ........         31113...
     * Row 2:  ........         31113...
     * Row 3:  ....3333         31113...
     * Row 4:  ...33223         31111333
     * Row 5:  ..32222.         .3311333
     * Row 6:  .322222.         ..3333..
     * Row 7:  32222223         32222...
     *
     *   BL tile (cols 0-7)    BR tile (cols 8-15)
     * Row 0:  32222223         32222...
     * Row 1:  .3222223         32222...
     * Row 2:  ..322223         32222...
     * Row 3:  ..322223         32223...
     * Row 4:  ..322223         3223....
     * Row 5:  ...32233         33......
     * Row 6:  ...33..3         3..33...
     * Row 7:  ...33..3         3..33...
     */

    /* TITLE_PIG2_TL (tile 4): pigeon frame 2, top-left */
    /* Row 0: ........  -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 1: ........  -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 2: ........  -> lo=0x00 hi=0x00 */
    0x00, 0x00,
    /* Row 3: ....3333  colors: 00003333 -> lo=0x0F hi=0x0F */
    0x0F, 0x0F,
    /* Row 4: ...33223  colors: 00033223 -> lo=0x1B hi=0x0D */
    0x1B, 0x0D,
    /* Row 5: ..32222.  colors: 00322220 -> lo=0x20 hi=0x3E */
    0x20, 0x3E,
    /* Row 6: .322222.  colors: 03222220 -> lo=0x40 hi=0x7E */
    0x40, 0x7E,
    /* Row 7: 32222223  colors: 32222223 -> lo=0x81 hi=0xFF */
    0x81, 0xFF,

    /* TITLE_PIG2_TR (tile 5): pigeon frame 2, top-right */
    /* Row 0: .333....  colors: 03330000 -> lo=0x70 hi=0x70 */
    0x70, 0x70,
    /* Row 1: 31113...  colors: 31113000 -> lo=0xF8 hi=0x88 */
    0xF8, 0x88,
    /* Row 2: 31113...  colors: 31113000 -> lo=0xF8 hi=0x88 */
    0xF8, 0x88,
    /* Row 3: 31113...  colors: 31113000 -> lo=0xF8 hi=0x88 */
    0xF8, 0x88,
    /* Row 4: 31111333  colors: 31111333 -> lo=0xFF hi=0x87 */
    0xFF, 0x87,
    /* Row 5: .3311333  colors: 03311333 -> lo=0x7F hi=0x6F */
    0x7F, 0x6F,
    /* Row 6: ..3333..  colors: 00333300 -> lo=0x3C hi=0x3C */
    0x3C, 0x3C,
    /* Row 7: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,

    /* TITLE_PIG2_BL (tile 6): pigeon frame 2, bottom-left */
    /* Row 0: 32222223  colors: 32222223 -> lo=0x81 hi=0xFF */
    0x81, 0xFF,
    /* Row 1: .3222223  colors: 03222223 -> lo=0x41 hi=0x7F */
    0x41, 0x7F,
    /* Row 2: ..322223  colors: 00322223 -> lo=0x21 hi=0x3F */
    0x21, 0x3F,
    /* Row 3: ..322223  colors: 00322223 -> lo=0x21 hi=0x3F */
    0x21, 0x3F,
    /* Row 4: ..322223  colors: 00322223 -> lo=0x21 hi=0x3F */
    0x21, 0x3F,
    /* Row 5: ...32233  colors: 00032233 -> lo=0x1B hi=0x1F */
    0x1B, 0x1F,
    /* Row 6: ...33..3  colors: 00033003 -> lo=0x19 hi=0x19 */
    0x19, 0x19,
    /* Row 7: ...33..3  colors: 00033003 -> lo=0x19 hi=0x19 */
    0x19, 0x19,

    /* TITLE_PIG2_BR (tile 7): pigeon frame 2, bottom-right */
    /* Row 0: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,
    /* Row 1: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,
    /* Row 2: 32222...  colors: 32222000 -> lo=0x80 hi=0xF8 */
    0x80, 0xF8,
    /* Row 3: 32223...  colors: 32223000 -> lo=0x90 hi=0xF8 */
    0x90, 0xF8,
    /* Row 4: 3223....  colors: 32230000 -> lo=0xB0 hi=0xF0 */
    0xB0, 0xF0,
    /* Row 5: 33......  colors: 33000000 -> lo=0xC0 hi=0xC0 */
    0xC0, 0xC0,
    /* Row 6: 3..33...  colors: 30033000 -> lo=0x98 hi=0x98 */
    0x98, 0x98,
    /* Row 7: 3..33...  colors: 30033000 -> lo=0x98 hi=0x98 */
    0x98, 0x98
};

/*
 * WATERFALL PIXEL ART - 16x16, 4 animation frames
 *
 * Design: two water columns with foam highlights that shift down each frame.
 * Legend: .=c0(cream/bg) 1=c1(white foam) 2=c2(blue water) 3=c3(dark blue shadow)
 *
 * The waterfall is two thick streams (cols 1-6 and 9-14) separated by
 * a gap in the middle. Foam highlights (c1) create the illusion of flow
 * by shifting downward 2 rows per frame.
 *
 * Base pattern (water body = c2, shadow edges = c3, foam highlights = c1):
 *   Left column occupies cols 0-7, Right column occupies cols 8-15
 *   Pattern: 3=shadow on edges, 2=water body, 1=foam streaks inside
 *
 * Frame offset: foam rows shift down by 2 each frame, wrapping around.
 *
 * Each frame's 16x16 layout (TL, TR, BL, BR):
 *   TL = left half rows 0-7, TR = right half rows 0-7
 *   BL = left half rows 8-15, BR = right half rows 8-15
 */

const UINT8 waterfall_tiles[] = {
    /* === Frame 1 === */
    /* Foam at rows 0-1, 8-9 (of 16); water elsewhere */

    /* TITLE_WFALL_F1_TL (tile 0): frame 1, top-left */
    /* Row 0: .3112213  foam row -> lo=0x73 hi=0x6D  (cols: 03112213) */
    /* Row 0: .3112213 -> 0,3,1,1,2,2,1,3 */
    /*   lo bits: 0 1 1 1 0 0 1 1 = 0x73 */
    /*   hi bits: 0 1 0 0 1 1 0 1 = 0x4D */
    0x73, 0x4D,
    /* Row 1: .3112213 -> same foam pattern */
    0x73, 0x4D,
    /* Row 2: .3222233 -> 0,3,2,2,2,2,3,3 */
    /*   lo bits: 0 1 0 0 0 0 1 1 = 0x43 */
    /*   hi bits: 0 1 1 1 1 1 1 1 = 0x7F */
    0x43, 0x7F,
    /* Row 3: .3222233 -> same */
    0x43, 0x7F,
    /* Row 4: .3222233 -> same */
    0x43, 0x7F,
    /* Row 5: .3222233 -> same */
    0x43, 0x7F,
    /* Row 6: .3222233 -> same */
    0x43, 0x7F,
    /* Row 7: .3222233 -> same */
    0x43, 0x7F,

    /* TITLE_WFALL_F1_TR (tile 1): frame 1, top-right */
    /* Row 0: 31122.3. -> 3,1,1,2,2,0,3,0  foam row */
    /*   lo bits: 1 1 1 0 0 0 1 0 = 0xE2 */
    /*   hi bits: 1 0 0 1 1 0 1 0 = 0x9A */
    0xE2, 0x9A,
    /* Row 1: 31122.3. -> same */
    0xE2, 0x9A,
    /* Row 2: 32222.3. -> 3,2,2,2,2,0,3,0 */
    /*   lo bits: 1 0 0 0 0 0 1 0 = 0x82 */
    /*   hi bits: 1 1 1 1 1 0 1 0 = 0xFA */
    0x82, 0xFA,
    /* Row 3: 32222.3. */
    0x82, 0xFA,
    /* Row 4: 32222.3. */
    0x82, 0xFA,
    /* Row 5: 32222.3. */
    0x82, 0xFA,
    /* Row 6: 32222.3. */
    0x82, 0xFA,
    /* Row 7: 32222.3. */
    0x82, 0xFA,

    /* TITLE_WFALL_F1_BL (tile 2): frame 1, bottom-left */
    /* Row 8: .3112213 -> foam row */
    0x73, 0x4D,
    /* Row 9: .3112213 -> foam row */
    0x73, 0x4D,
    /* Row 10: .3222233 -> water */
    0x43, 0x7F,
    /* Row 11: .3222233 */
    0x43, 0x7F,
    /* Row 12: .3222233 */
    0x43, 0x7F,
    /* Row 13: .3222233 */
    0x43, 0x7F,
    /* Row 14: .3222233 */
    0x43, 0x7F,
    /* Row 15: .3222233 */
    0x43, 0x7F,

    /* TITLE_WFALL_F1_BR (tile 3): frame 1, bottom-right */
    /* Row 8: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 9: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 10: 32222.3. -> water */
    0x82, 0xFA,
    /* Row 11: 32222.3. */
    0x82, 0xFA,
    /* Row 12: 32222.3. */
    0x82, 0xFA,
    /* Row 13: 32222.3. */
    0x82, 0xFA,
    /* Row 14: 32222.3. */
    0x82, 0xFA,
    /* Row 15: 32222.3. */
    0x82, 0xFA,

    /* === Frame 2 === */
    /* Foam shifted down 2 rows: foam at rows 2-3, 10-11 */

    /* TITLE_WFALL_F2_TL (tile 4): frame 2, top-left */
    /* Row 0: .3222233 -> water */
    0x43, 0x7F,
    /* Row 1: .3222233 */
    0x43, 0x7F,
    /* Row 2: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 3: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 4: .3222233 -> water */
    0x43, 0x7F,
    /* Row 5: .3222233 */
    0x43, 0x7F,
    /* Row 6: .3222233 */
    0x43, 0x7F,
    /* Row 7: .3222233 */
    0x43, 0x7F,

    /* TITLE_WFALL_F2_TR (tile 5): frame 2, top-right */
    /* Row 0: 32222.3. */
    0x82, 0xFA,
    /* Row 1: 32222.3. */
    0x82, 0xFA,
    /* Row 2: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 3: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 4: 32222.3. */
    0x82, 0xFA,
    /* Row 5: 32222.3. */
    0x82, 0xFA,
    /* Row 6: 32222.3. */
    0x82, 0xFA,
    /* Row 7: 32222.3. */
    0x82, 0xFA,

    /* TITLE_WFALL_F2_BL (tile 6): frame 2, bottom-left */
    /* Row 8: .3222233 */
    0x43, 0x7F,
    /* Row 9: .3222233 */
    0x43, 0x7F,
    /* Row 10: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 11: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 12: .3222233 */
    0x43, 0x7F,
    /* Row 13: .3222233 */
    0x43, 0x7F,
    /* Row 14: .3222233 */
    0x43, 0x7F,
    /* Row 15: .3222233 */
    0x43, 0x7F,

    /* TITLE_WFALL_F2_BR (tile 7): frame 2, bottom-right */
    /* Row 8: 32222.3. */
    0x82, 0xFA,
    /* Row 9: 32222.3. */
    0x82, 0xFA,
    /* Row 10: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 11: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 12: 32222.3. */
    0x82, 0xFA,
    /* Row 13: 32222.3. */
    0x82, 0xFA,
    /* Row 14: 32222.3. */
    0x82, 0xFA,
    /* Row 15: 32222.3. */
    0x82, 0xFA,

    /* === Frame 3 === */
    /* Foam shifted down 4 rows: foam at rows 4-5, 12-13 */

    /* TITLE_WFALL_F3_TL (tile 8): frame 3, top-left */
    /* Row 0: .3222233 */
    0x43, 0x7F,
    /* Row 1: .3222233 */
    0x43, 0x7F,
    /* Row 2: .3222233 */
    0x43, 0x7F,
    /* Row 3: .3222233 */
    0x43, 0x7F,
    /* Row 4: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 5: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 6: .3222233 */
    0x43, 0x7F,
    /* Row 7: .3222233 */
    0x43, 0x7F,

    /* TITLE_WFALL_F3_TR (tile 9): frame 3, top-right */
    /* Row 0: 32222.3. */
    0x82, 0xFA,
    /* Row 1: 32222.3. */
    0x82, 0xFA,
    /* Row 2: 32222.3. */
    0x82, 0xFA,
    /* Row 3: 32222.3. */
    0x82, 0xFA,
    /* Row 4: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 5: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 6: 32222.3. */
    0x82, 0xFA,
    /* Row 7: 32222.3. */
    0x82, 0xFA,

    /* TITLE_WFALL_F3_BL (tile 10): frame 3, bottom-left */
    /* Row 8: .3222233 */
    0x43, 0x7F,
    /* Row 9: .3222233 */
    0x43, 0x7F,
    /* Row 10: .3222233 */
    0x43, 0x7F,
    /* Row 11: .3222233 */
    0x43, 0x7F,
    /* Row 12: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 13: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 14: .3222233 */
    0x43, 0x7F,
    /* Row 15: .3222233 */
    0x43, 0x7F,

    /* TITLE_WFALL_F3_BR (tile 11): frame 3, bottom-right */
    /* Row 8: 32222.3. */
    0x82, 0xFA,
    /* Row 9: 32222.3. */
    0x82, 0xFA,
    /* Row 10: 32222.3. */
    0x82, 0xFA,
    /* Row 11: 32222.3. */
    0x82, 0xFA,
    /* Row 12: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 13: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 14: 32222.3. */
    0x82, 0xFA,
    /* Row 15: 32222.3. */
    0x82, 0xFA,

    /* === Frame 4 === */
    /* Foam shifted down 6 rows: foam at rows 6-7, 14-15 */

    /* TITLE_WFALL_F4_TL (tile 12): frame 4, top-left */
    /* Row 0: .3222233 */
    0x43, 0x7F,
    /* Row 1: .3222233 */
    0x43, 0x7F,
    /* Row 2: .3222233 */
    0x43, 0x7F,
    /* Row 3: .3222233 */
    0x43, 0x7F,
    /* Row 4: .3222233 */
    0x43, 0x7F,
    /* Row 5: .3222233 */
    0x43, 0x7F,
    /* Row 6: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 7: .3112213 -> foam */
    0x73, 0x4D,

    /* TITLE_WFALL_F4_TR (tile 13): frame 4, top-right */
    /* Row 0: 32222.3. */
    0x82, 0xFA,
    /* Row 1: 32222.3. */
    0x82, 0xFA,
    /* Row 2: 32222.3. */
    0x82, 0xFA,
    /* Row 3: 32222.3. */
    0x82, 0xFA,
    /* Row 4: 32222.3. */
    0x82, 0xFA,
    /* Row 5: 32222.3. */
    0x82, 0xFA,
    /* Row 6: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 7: 31122.3. -> foam */
    0xE2, 0x9A,

    /* TITLE_WFALL_F4_BL (tile 14): frame 4, bottom-left */
    /* Row 8: .3222233 */
    0x43, 0x7F,
    /* Row 9: .3222233 */
    0x43, 0x7F,
    /* Row 10: .3222233 */
    0x43, 0x7F,
    /* Row 11: .3222233 */
    0x43, 0x7F,
    /* Row 12: .3222233 */
    0x43, 0x7F,
    /* Row 13: .3222233 */
    0x43, 0x7F,
    /* Row 14: .3112213 -> foam */
    0x73, 0x4D,
    /* Row 15: .3112213 -> foam */
    0x73, 0x4D,

    /* TITLE_WFALL_F4_BR (tile 15): frame 4, bottom-right */
    /* Row 8: 32222.3. */
    0x82, 0xFA,
    /* Row 9: 32222.3. */
    0x82, 0xFA,
    /* Row 10: 32222.3. */
    0x82, 0xFA,
    /* Row 11: 32222.3. */
    0x82, 0xFA,
    /* Row 12: 32222.3. */
    0x82, 0xFA,
    /* Row 13: 32222.3. */
    0x82, 0xFA,
    /* Row 14: 31122.3. -> foam */
    0xE2, 0x9A,
    /* Row 15: 31122.3. -> foam */
    0xE2, 0x9A
};

/*
 * ORTHODOX THREE-BAR CROSS - 16x24 (2 wide x 3 tall)
 *
 * Uses cross palette 0: c0=cream bg, c1=dark body, c2=medium accent, c3=near black
 * The cross body is drawn primarily in c1 (dark) with c3 outline and c2 accent.
 *
 * Full 16x24 pixel grid:
 * Legend: .=c0(cream) 1=c1(dark body) 2=c2(accent) 3=c3(outline)
 *
 *  TL (cols 0-7)   TR (cols 8-15)     Description
 * -----------------------------------------------
 * Row 0:  ......33  33......          top of shaft
 * Row 1:  .....311  113.....          shaft with outline
 * Row 2:  ..333111  111333..          titulus (small top bar)
 * Row 3:  ..311111  111113..          titulus body
 * Row 4:  ..333111  111333..          titulus bottom outline
 * Row 5:  .....311  113.....          shaft
 * Row 6:  33333311  113333.3          main crossbeam top outline
 * Row 7:  31111111  111111.3          main crossbeam body
 *
 *  ML (cols 0-7)   MR (cols 8-15)
 * -----------------------------------------------
 * Row 0:  31111111  111111.3          main crossbeam body
 * Row 1:  33333311  11333333          main crossbeam bottom outline
 * Row 2:  .....311  113.....          shaft
 * Row 3:  .....311  113.....          shaft
 * Row 4:  .....311  113.....          shaft
 * Row 5:  .....311  113.....          shaft
 * Row 6:  ..333111  11133...          footrest (angled: left-high)
 * Row 7:  .....311  1133....          footrest (angled: right-low)
 *
 *  BL (cols 0-7)   BR (cols 8-15)
 * -----------------------------------------------
 * Row 0:  .....311  113.....          shaft
 * Row 1:  .....311  113.....          shaft
 * Row 2:  .....311  113.....          shaft
 * Row 3:  .....311  113.....          shaft
 * Row 4:  ....3221  1223....          shaft widens at base (accent)
 * Row 5:  ....3111  1113....          wide base
 * Row 6:  .....333  333.....          base outline
 * Row 7:  ........  ........          empty
 */

const UINT8 orthodox_cross_tiles[] = {
    /* TITLE_CROSS_TL: top-left (top bar + crossbeam left half) */

    /* Row 0: ......33  colors: 00000033 */
    /*   lo bits: 0 0 0 0 0 0 1 1 = 0x03  hi bits: 0 0 0 0 0 0 1 1 = 0x03 */
    0x03, 0x03,
    /* Row 1: .....311  colors: 00000311 */
    /*   lo bits: 0 0 0 0 0 1 1 1 = 0x07  hi bits: 0 0 0 0 0 1 0 0 = 0x04 */
    0x07, 0x04,
    /* Row 2: ..333111  colors: 00333111 */
    /*   lo bits: 0 0 1 1 1 1 1 1 = 0x3F  hi bits: 0 0 1 1 1 0 0 0 = 0x38 */
    0x3F, 0x38,
    /* Row 3: ..311111  colors: 00311111 */
    /*   lo bits: 0 0 1 1 1 1 1 1 = 0x3F  hi bits: 0 0 1 0 0 0 0 0 = 0x20 */
    0x3F, 0x20,
    /* Row 4: ..333111  colors: 00333111 */
    0x3F, 0x38,
    /* Row 5: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 6: 33333311  colors: 33333311 */
    /*   lo bits: 1 1 1 1 1 1 1 1 = 0xFF  hi bits: 1 1 1 1 1 1 0 0 = 0xFC */
    0xFF, 0xFC,
    /* Row 7: 31111111  colors: 31111111 */
    /*   lo bits: 1 1 1 1 1 1 1 1 = 0xFF  hi bits: 1 0 0 0 0 0 0 0 = 0x80 */
    0xFF, 0x80,

    /* TITLE_CROSS_TR: top-right (top bar + crossbeam right half) */

    /* Row 0: 33......  colors: 33000000 */
    /*   lo bits: 1 1 0 0 0 0 0 0 = 0xC0  hi bits: 1 1 0 0 0 0 0 0 = 0xC0 */
    0xC0, 0xC0,
    /* Row 1: 113.....  colors: 11300000 */
    /*   lo bits: 1 1 1 0 0 0 0 0 = 0xE0  hi bits: 0 0 1 0 0 0 0 0 = 0x20 */
    0xE0, 0x20,
    /* Row 2: 111333..  colors: 11133300 */
    /*   lo bits: 1 1 1 1 1 1 0 0 = 0xFC  hi bits: 0 0 0 1 1 1 0 0 = 0x1C */
    0xFC, 0x1C,
    /* Row 3: 111113..  colors: 11111300 */
    /*   lo bits: 1 1 1 1 1 1 0 0 = 0xFC  hi bits: 0 0 0 0 0 1 0 0 = 0x04 */
    0xFC, 0x04,
    /* Row 4: 111333..  colors: 11133300 */
    0xFC, 0x1C,
    /* Row 5: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 6: 11333333  colors: 11333333 */
    /*   lo bits: 1 1 1 1 1 1 1 1 = 0xFF  hi bits: 0 0 1 1 1 1 1 1 = 0x3F */
    0xFF, 0x3F,
    /* Row 7: 11111113  colors: 11111113 */
    /*   lo bits: 1 1 1 1 1 1 1 1 = 0xFF  hi bits: 0 0 0 0 0 0 0 1 = 0x01 */
    0xFF, 0x01,

    /* TITLE_CROSS_ML: middle-left (shaft + footrest left half) */

    /* Row 0: 31111111  colors: 31111111 */
    0xFF, 0x80,
    /* Row 1: 33333311  colors: 33333311 */
    0xFF, 0xFC,
    /* Row 2: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 3: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 4: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 5: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 6: ..333111  colors: 00333111 (footrest left-high) */
    0x3F, 0x38,
    /* Row 7: .....311  colors: 00000311 (footrest continues at shaft) */
    0x07, 0x04,

    /* TITLE_CROSS_MR: middle-right (shaft + footrest right half) */

    /* Row 0: 11111113  colors: 11111113 */
    0xFF, 0x01,
    /* Row 1: 11333333  colors: 11333333 */
    /*   lo bits: 1 1 1 1 1 1 1 1 = 0xFF  hi bits: 0 0 1 1 1 1 1 1 = 0x3F */
    0xFF, 0x3F,
    /* Row 2: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 3: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 4: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 5: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 6: 11133...  colors: 11133000 (footrest right-low) */
    /*   lo bits: 1 1 1 1 1 0 0 0 = 0xF8  hi bits: 0 0 0 1 1 0 0 0 = 0x18 */
    0xF8, 0x18,
    /* Row 7: 1133....  colors: 11330000 */
    /*   lo bits: 1 1 1 1 0 0 0 0 = 0xF0  hi bits: 0 0 1 1 0 0 0 0 = 0x30 */
    0xF0, 0x30,

    /* TITLE_CROSS_BL: bottom-left (shaft base) */

    /* Row 0: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 1: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 2: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 3: .....311  colors: 00000311 */
    0x07, 0x04,
    /* Row 4: ....3221  colors: 00003221 (base widens with accent) */
    /*   lo bits: 0 0 0 0 1 0 0 1 = 0x09  hi bits: 0 0 0 0 1 1 1 0 = 0x0E */
    0x09, 0x0E,
    /* Row 5: ....3111  colors: 00003111 */
    /*   lo bits: 0 0 0 0 1 1 1 1 = 0x0F  hi bits: 0 0 0 0 1 0 0 0 = 0x08 */
    0x0F, 0x08,
    /* Row 6: .....333  colors: 00000333 */
    /*   lo bits: 0 0 0 0 0 1 1 1 = 0x07  hi bits: 0 0 0 0 0 1 1 1 = 0x07 */
    0x07, 0x07,
    /* Row 7: ........  colors: 00000000 */
    0x00, 0x00,

    /* TITLE_CROSS_BR: bottom-right (shaft base) */

    /* Row 0: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 1: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 2: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 3: 113.....  colors: 11300000 */
    0xE0, 0x20,
    /* Row 4: 1223....  colors: 12230000 (base widens with accent) */
    /*   lo bits: 1 0 0 1 0 0 0 0 = 0x90  hi bits: 0 1 1 1 0 0 0 0 = 0x70 */
    0x90, 0x70,
    /* Row 5: 1113....  colors: 11130000 */
    /*   lo bits: 1 1 1 1 0 0 0 0 = 0xF0  hi bits: 0 0 0 1 0 0 0 0 = 0x10 */
    0xF0, 0x10,
    /* Row 6: 333.....  colors: 33300000 */
    /*   lo bits: 1 1 1 0 0 0 0 0 = 0xE0  hi bits: 1 1 1 0 0 0 0 0 = 0xE0 */
    0xE0, 0xE0,
    /* Row 7: ........  colors: 00000000 */
    0x00, 0x00
};

void title_tiles_load(void) NONBANKED {
    UINT8 old_bank = _current_bank;
    SWITCH_ROM_MBC1(10);
    /* Load pigeons at indices 98-105, waterfall at 106-121, cross at 122-127 */
    set_bkg_data(TITLE_TILE_BASE, TITLE_PIGEON_TILES, pigeon_tiles);
    set_bkg_data(TITLE_TILE_BASE + TITLE_PIGEON_TILES, TITLE_WATERFALL_TILES, waterfall_tiles);
    set_bkg_data(TITLE_TILE_BASE + TITLE_PIGEON_TILES + TITLE_WATERFALL_TILES, TITLE_CROSS_TILES, orthodox_cross_tiles);
    SWITCH_ROM_MBC1(old_bank);
}
