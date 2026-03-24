/*
 * matching.c — 4x4 memory matching card game with biblical symbols.
 * Cards are 2x2 tiles each on a 4-column x 4-row grid.
 * Banked to bank 15 to save home bank space.
 */

#pragma bank 15

#include <gb/gb.h>
#include <gb/cgb.h>
#include "matching.h"
#include "match_tiles.h"
#include "font.h"
#include "sound.h"
#include "bank_helpers.h"

#define GRID_COLS 4u
#define GRID_ROWS 4u
#define NUM_CARDS 16u

/* Card grid tile positions - each card is a 2x2 tile block */
/* Column start X positions (tile coords) */
#define CARD_X0  1u
#define CARD_X1  6u
#define CARD_X2  11u
#define CARD_X3  16u

/* Row start Y positions (tile coords) */
#define CARD_Y0  2u
#define CARD_Y1  6u
#define CARD_Y2  10u
#define CARD_Y3  14u

static const UINT8 card_xs[4] = { CARD_X0, CARD_X1, CARD_X2, CARD_X3 };
static const UINT8 card_ys[4] = { CARD_Y0, CARD_Y1, CARD_Y2, CARD_Y3 };

/* Card state */
#define CARD_FACE_DOWN  0u
#define CARD_FACE_UP    1u
#define CARD_MATCHED    2u

/* Symbol tile offsets (base tile for each symbol's 2x2 block within match_tiles) */
static const UINT8 symbol_tile_offsets[8] = {
    MT_CROSS_TL,
    MT_DOVE_TL,
    MT_FISH_TL,
    MT_CUP_TL,
    MT_LAMB_TL,
    MT_CROWN_TL,
    MT_BREAD_TL,
    MT_FLAME_TL
};

static UINT8 cards[NUM_CARDS];       /* symbol index (0-7) for each card position */
static UINT8 card_state[NUM_CARDS];  /* CARD_FACE_DOWN/UP/MATCHED */
static UINT8 cursor_pos;             /* 0..15 - current card under cursor */
static UINT8 first_pick;             /* index of first revealed card (or 0xFF if none) */
static UINT8 second_pick;            /* index of second revealed card (or 0xFF if none) */
static UINT8 reveal_timer;           /* countdown after revealing second card */
static UINT8 matches_found;          /* number of pairs matched */
static UINT8 won;                    /* 1 if all matched */
static UINT16 rng_state;             /* simple RNG state */
static UINT8 needs_redraw;          /* 1 = full redraw needed */

/* Simple LCG random number generator */
static UINT8 rng_next(void) {
    rng_state = (UINT16)(rng_state * 29u + 37u);
    return (UINT8)(rng_state >> 3);
}

static void shuffle_cards(void) {
    UINT8 i, j, tmp;

    /* Fill pairs: cards[0..1]=0, cards[2..3]=1, ... cards[14..15]=7 */
    for (i = 0; i < NUM_CARDS; i++) {
        cards[i] = (UINT8)(i >> 1);
    }

    /* Fisher-Yates shuffle */
    for (i = NUM_CARDS - 1u; i > 0; i--) {
        j = (UINT8)(rng_next() % (i + 1u));
        tmp = cards[i];
        cards[i] = cards[j];
        cards[j] = tmp;
    }
}

static void draw_card(UINT8 card_idx) {
    UINT8 col, row, tx, ty;
    unsigned char tl, tr, bl, br;

    col = card_idx & 3u;     /* 0..3 */
    row = card_idx >> 2;     /* 0..3 */
    tx = card_xs[col];
    ty = card_ys[row];

    if (card_state[card_idx] == CARD_MATCHED) {
        /* Show the matched symbol so player sees what they found */
        UINT8 sym_base_m;
        sym_base_m = symbol_tile_offsets[cards[card_idx]];
        tl = MATCH_TILE_BASE + sym_base_m;
        tr = MATCH_TILE_BASE + (UINT8)(sym_base_m + 1u);
        bl = MATCH_TILE_BASE + (UINT8)(sym_base_m + 2u);
        br = MATCH_TILE_BASE + MT_MATCHED;  /* checkmark in bottom-right */
    } else if (card_state[card_idx] == CARD_FACE_UP) {
        /* Show symbol */
        UINT8 sym_base;
        sym_base = symbol_tile_offsets[cards[card_idx]];
        tl = MATCH_TILE_BASE + sym_base;
        tr = MATCH_TILE_BASE + (UINT8)(sym_base + 1u);
        bl = MATCH_TILE_BASE + (UINT8)(sym_base + 2u);
        br = MATCH_TILE_BASE + (UINT8)(sym_base + 3u);
    } else {
        /* Face down - show card back */
        tl = MATCH_TILE_BASE + MT_BACK_TL;
        tr = MATCH_TILE_BASE + MT_BACK_TR;
        bl = MATCH_TILE_BASE + MT_BACK_BL;
        br = MATCH_TILE_BASE + MT_BACK_BR;
    }

    set_bkg_tiles(tx, ty, 1, 1, &tl);
    set_bkg_tiles((UINT8)(tx + 1u), ty, 1, 1, &tr);
    set_bkg_tiles(tx, (UINT8)(ty + 1u), 1, 1, &bl);
    set_bkg_tiles((UINT8)(tx + 1u), (UINT8)(ty + 1u), 1, 1, &br);
}

/* Set CGB palette attribute for a card's 2x2 area */
static void set_card_pal(UINT8 card_idx, UINT8 pal) {
    UINT8 col, row, tx, ty;
    unsigned char pal_val;

    col = card_idx & 3u;
    row = card_idx >> 2;
    tx = card_xs[col];
    ty = card_ys[row];
    pal_val = pal;

    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        set_bkg_tiles(tx, ty, 1, 1, &pal_val);
        set_bkg_tiles((UINT8)(tx + 1u), ty, 1, 1, &pal_val);
        set_bkg_tiles(tx, (UINT8)(ty + 1u), 1, 1, &pal_val);
        set_bkg_tiles((UINT8)(tx + 1u), (UINT8)(ty + 1u), 1, 1, &pal_val);
        VBK_REG = 0;
    }
}

/* Draw corner bracket markers around the cursor card */
static void draw_cursor_brackets(UINT8 card_idx) {
    UINT8 col, row, tx, ty;
    unsigned char tl, tr, bl, br;

    col = card_idx & 3u;
    row = card_idx >> 2;
    tx = card_xs[col];
    ty = card_ys[row];

    tl = MATCH_TILE_BASE + MT_CURSOR_TL;
    tr = MATCH_TILE_BASE + MT_CURSOR_TR;
    bl = MATCH_TILE_BASE + MT_CURSOR_BL;
    br = MATCH_TILE_BASE + MT_CURSOR_BR;

    /* Top brackets (row above card) */
    if (ty > 0) {
        set_bkg_tiles(tx, (UINT8)(ty - 1u), 1, 1, &tl);
        set_bkg_tiles((UINT8)(tx + 1u), (UINT8)(ty - 1u), 1, 1, &tr);
    }
    /* Bottom brackets (row below card) */
    if (ty + 2u < 18u) {
        set_bkg_tiles(tx, (UINT8)(ty + 2u), 1, 1, &bl);
        set_bkg_tiles((UINT8)(tx + 1u), (UINT8)(ty + 2u), 1, 1, &br);
    }

    /* Highlight card with accent palette */
    set_card_pal(card_idx, 1);
}

/* Clear cursor brackets and reset palette */
static void clear_cursor_brackets(UINT8 card_idx) {
    UINT8 col, row, tx, ty;
    unsigned char empty;

    col = card_idx & 3u;
    row = card_idx >> 2;
    tx = card_xs[col];
    ty = card_ys[row];

    empty = MATCH_TILE_BASE + MT_EMPTY;

    if (ty > 0) {
        set_bkg_tiles(tx, (UINT8)(ty - 1u), 1, 1, &empty);
        set_bkg_tiles((UINT8)(tx + 1u), (UINT8)(ty - 1u), 1, 1, &empty);
    }
    if (ty + 2u < 18u) {
        set_bkg_tiles(tx, (UINT8)(ty + 2u), 1, 1, &empty);
        set_bkg_tiles((UINT8)(tx + 1u), (UINT8)(ty + 2u), 1, 1, &empty);
    }

    /* Reset card palette to default */
    set_card_pal(card_idx, 0);
}

void matching_init(void) BANKED {
    UINT8 i;
    UWORD pal[4];
    unsigned char blank;

    /* Seed RNG from DIV register (hardware timer - gives some randomness) */
    rng_state = (UINT16)(DIV_REG) | ((UINT16)(DIV_REG) << 8);
    if (rng_state == 0) rng_state = 12345u;

    /* Load matching game tiles from banked ROM (via bank-0 helper) */
    load_bkg_data_banked(MATCH_TILES_BANK, MATCH_TILE_BASE, MATCH_TILE_COUNT, match_tiles_data);

    /* Set palette for matching game — tilesheet green */
    if (_cpu == CGB_TYPE) {
        /* Pal 0: card tiles - tilesheet green gradient */
        pal[0] = RGB(28, 31, 26);  /* lightest green */
        pal[1] = RGB(17, 24, 14);  /* mid-light green */
        pal[2] = RGB(6, 13, 10);   /* mid-dark green */
        pal[3] = RGB(1, 3, 4);     /* darkest green */
        set_bkg_palette(0, 1, pal);

        /* Pal 1: cursor highlight - shifted green (darker bg = highlight) */
        pal[0] = RGB(17, 24, 14);  /* mid-light (darker bg for highlight) */
        pal[1] = RGB(28, 31, 26);  /* lightest green */
        pal[2] = RGB(1, 3, 4);     /* darkest green */
        pal[3] = RGB(6, 13, 10);   /* mid-dark green */
        set_bkg_palette(1, 1, pal);

        /* Pal 2: text palette (c1=darkest for font readability) */
        pal[0] = RGB(28, 31, 26);  /* lightest green (background) */
        pal[1] = RGB(1, 3, 4);     /* darkest green (font glyphs) */
        pal[2] = RGB(6, 13, 10);   /* mid-dark green */
        pal[3] = RGB(17, 24, 14);  /* mid-light green */
        set_bkg_palette(2, 1, pal);
    }

    /* Clear screen */
    blank = 0;
    for (i = 0; i < 18u; i++) {
        UINT8 x;
        for (x = 0; x < 20u; x++) {
            set_bkg_tiles(x, i, 1, 1, &blank);
        }
    }

    /* Draw title */
    draw_text(2, 0, "MEMORY OF SYMBOLS");

    /* Initialize game state */
    shuffle_cards();
    for (i = 0; i < NUM_CARDS; i++) {
        card_state[i] = CARD_FACE_DOWN;
    }
    cursor_pos = 0;
    first_pick = 0xFF;
    second_pick = 0xFF;
    reveal_timer = 0;
    matches_found = 0;
    won = 0;
    needs_redraw = 1;
}

UINT8 matching_update(UBYTE pressed, UBYTE held) BANKED {
    UINT8 old_cursor;

    (void)held;

    /* Exit on B */
    if (pressed & J_B) {
        return 1;
    }

    /* If won, just wait for B to exit */
    if (won) {
        return 0;
    }

    /* If reveal timer is counting down, wait */
    if (reveal_timer > 0) {
        reveal_timer--;
        if (reveal_timer == 0) {
            /* Check if match */
            if (cards[first_pick] == cards[second_pick]) {
                card_state[first_pick] = CARD_MATCHED;
                card_state[second_pick] = CARD_MATCHED;
                matches_found++;
                sfx_match();
                if (matches_found >= 8u) {
                    won = 1;
                }
            } else {
                card_state[first_pick] = CARD_FACE_DOWN;
                card_state[second_pick] = CARD_FACE_DOWN;
                sfx_mismatch();
            }
            first_pick = 0xFF;
            second_pick = 0xFF;
            needs_redraw = 1; /* redraw all cards after reveal */
        }
        return 0;
    }

    /* Cursor movement */
    old_cursor = cursor_pos;
    if (pressed & J_RIGHT) {
        if ((cursor_pos & 3u) < 3u) cursor_pos++;
    }
    if (pressed & J_LEFT) {
        if ((cursor_pos & 3u) > 0u) cursor_pos--;
    }
    if (pressed & J_DOWN) {
        if (cursor_pos < 12u) cursor_pos = (UINT8)(cursor_pos + 4u);
    }
    if (pressed & J_UP) {
        if (cursor_pos >= 4u) cursor_pos = (UINT8)(cursor_pos - 4u);
    }

    /* Clear old cursor brackets and highlight, draw new ones */
    if (old_cursor != cursor_pos) {
        clear_cursor_brackets(old_cursor);
        draw_cursor_brackets(cursor_pos);
    }

    /* Flip card on A */
    if (pressed & J_A) {
        if (card_state[cursor_pos] == CARD_FACE_DOWN) {
            card_state[cursor_pos] = CARD_FACE_UP;
            sfx_card_flip();

            if (first_pick == 0xFF) {
                /* First card revealed */
                first_pick = cursor_pos;
            } else if (cursor_pos != first_pick) {
                /* Second card revealed (must be different from first) */
                second_pick = cursor_pos;
                reveal_timer = 45u; /* ~0.75 seconds display time */
            }
        }
    }

    return 0;
}

void matching_render(void) BANKED {
    UINT8 i;

    if (needs_redraw) {
        /* Full redraw: all cards + cursor + status */
        for (i = 0; i < NUM_CARDS; i++) {
            draw_card(i);
        }
        draw_cursor_brackets(cursor_pos);
        needs_redraw = 0;
    } else {
        /* Incremental: only redraw cards whose state just changed */
        if (first_pick != 0xFF) {
            draw_card(first_pick);
        }
        if (second_pick != 0xFF) {
            draw_card(second_pick);
        }
    }

    /* Win message */
    if (won) {
        draw_text(0, 18u - 1u, " WELL DONE!  B:EXIT ");
    } else {
        draw_text(0, 18u - 1u, "   A:FLIP  B:EXIT   ");
    }
}

UINT8 matching_is_won(void) BANKED {
    return won;
}
