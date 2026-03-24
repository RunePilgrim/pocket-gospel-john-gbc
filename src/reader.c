/*
 * reader.c — verse-by-verse Gospel of John text reader.
 * Displays one verse at a time with word-wrap and vertical centering.
 * UP/DOWN = previous/next verse, LEFT/RIGHT = skip chapters.
 *
 * Performance: verse text is buffered into WRAM via a pre-computed offset
 * table (john_verse_off/john_verse_len in bank 14). All word-wrapping and
 * rendering operates on the RAM buffer — no per-character bank switching.
 */

#pragma bank 15

#include <gb/gb.h>
#include "font.h"
#include "john_text.h"
#include "bank_helpers.h"

#define TEXT_LINES   14u  /* rows 2-15 for verse text */
#define TEXT_TOP      2u  /* first row of text area */
#define TEXT_X        1u  /* 1-tile left margin for clean indentation */
#define LINE_W       18u  /* 18 chars per line (1 margin each side) */

#define REPEAT_DELAY  8u   /* frames before hold-repeat starts */
#define REPEAT_RATE   4u   /* frames between repeats while held */

#define NUM_CHAPTERS 21u

/* Verse counts for each chapter of John (1-indexed via array index) */
static const UINT8 verse_counts[NUM_CHAPTERS] = {
    51, 25, 36, 54, 47, 71, 53, 59, 41, 42,
    57, 50, 38, 31, 27, 33, 26, 40, 42, 31, 25
};

static UINT8 cur_chapter;   /* 0-based chapter index */
static UINT8 cur_verse;     /* 1-based current verse number */
static UINT8 hold_timer;
static UINT8 repeating;
static UINT8 needs_redraw;
static UINT16 page_offset;  /* byte offset into verse_buf for pagination */

/* RAM buffer for the current verse text (filled by buffer_current_verse) */
static unsigned char verse_buf[JOHN_MAX_VERSE_LEN + 1u];
static UINT16 verse_buf_len;

/* ---- Verse buffering ---- */

/* Look up the current verse in the pre-computed index table (bank 14)
   and bulk-copy its text into verse_buf.
   Uses verse_lookup_banked() (NONBANKED helper in bank 0) to safely
   access the banked index table without unmapping this code. */
static void buffer_current_verse(void) {
    UINT32 off;
    UINT16 len;

    /* Read offset + length via bank-0 helper (safe from bank 15) */
    verse_lookup_banked(cur_chapter, cur_verse, &off, &len);

    if (len > JOHN_MAX_VERSE_LEN) len = JOHN_MAX_VERSE_LEN;

    /* Bulk copy verse text into RAM (john_read_buf is in bank 0, safe) */
    john_read_buf(off, len, verse_buf);
    verse_buf_len = len;
}

/* ---- Word-wrap helpers (operate on verse_buf in WRAM) ---- */

/* Count how many display lines remain from page_offset (for vertical centering) */
static UINT8 count_verse_lines(void) {
    UINT8 line_count = 0;
    UINT16 o = page_offset;

    while (o < verse_buf_len && line_count < TEXT_LINES) {
        UINT8 i, last_sp;
        unsigned char c;

        /* Skip leading whitespace */
        while (o < verse_buf_len) {
            c = verse_buf[o];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
            o++;
        }

        if (o >= verse_buf_len) break;

        /* Simulate filling a line */
        i = 0;
        last_sp = 0;
        while (o < verse_buf_len && i < LINE_W) {
            c = verse_buf[o];

            if (c == '\n' || c == '\r') {
                o++;
                while (o < verse_buf_len &&
                       (verse_buf[o] == '\n' || verse_buf[o] == '\r')) o++;
                break;
            }

            if (c < 32 || c > 126) c = ' ';
            if (c == ' ') last_sp = i;
            i++;
            o++;
        }

        /* Word-wrap: if we filled the line and are mid-word, break at last space */
        if (i == LINE_W && last_sp > 0 && o < verse_buf_len) {
            unsigned char nc = verse_buf[o];
            if (nc != ' ' && nc != '\t' && nc != '\n' && nc != '\r') {
                UINT16 rewind_amt = (UINT16)(i - last_sp - 1u);
                o -= rewind_amt;
            }
        }

        line_count++;
    }

    return line_count;
}

/* Advance page_offset past TEXT_LINES worth of word-wrapped text.
   Returns the new offset (past those lines). */
static UINT16 advance_page(UINT16 from) {
    UINT8 line_count = 0;
    UINT16 o = from;

    while (o < verse_buf_len && line_count < TEXT_LINES) {
        UINT8 i, last_sp;
        unsigned char c;

        while (o < verse_buf_len) {
            c = verse_buf[o];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
            o++;
        }
        if (o >= verse_buf_len) break;

        i = 0;
        last_sp = 0;
        while (o < verse_buf_len && i < LINE_W) {
            c = verse_buf[o];
            if (c == '\n' || c == '\r') {
                o++;
                while (o < verse_buf_len &&
                       (verse_buf[o] == '\n' || verse_buf[o] == '\r')) o++;
                break;
            }
            if (c < 32 || c > 126) c = ' ';
            if (c == ' ') last_sp = i;
            i++;
            o++;
        }
        if (i == LINE_W && last_sp > 0 && o < verse_buf_len) {
            unsigned char nc = verse_buf[o];
            if (nc != ' ' && nc != '\t' && nc != '\n' && nc != '\r') {
                UINT16 rewind_amt = (UINT16)(i - last_sp - 1u);
                o -= rewind_amt;
            }
        }
        line_count++;
    }
    return o;
}

/* Word-wrap render: fill display lines from verse_buf, starting at given row */
static void render_verse_text(UINT8 start_row) {
    UINT8 line;
    UINT16 o = page_offset;
    char buf[21];
    UINT8 i, j, last_sp;
    unsigned char c;

    for (line = 0; line < TEXT_LINES; line++) {
        /* Clear line buffer */
        for (j = 0; j < LINE_W; j++) buf[j] = ' ';
        buf[LINE_W] = 0;

        if (line < start_row || o >= verse_buf_len) {
            /* Blank line (above or below verse text) */
            draw_text(TEXT_X, (UINT8)(TEXT_TOP + line), buf);
            continue;
        }

        /* Skip leading whitespace */
        while (o < verse_buf_len) {
            c = verse_buf[o];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
            o++;
        }

        if (o >= verse_buf_len) {
            /* Past end of verse text, draw blank line */
            draw_text(TEXT_X, (UINT8)(TEXT_TOP + line), buf);
            continue;
        }

        /* Fill line with text */
        i = 0;
        last_sp = 0;
        while (o < verse_buf_len && i < LINE_W) {
            c = verse_buf[o];

            if (c == '\n' || c == '\r') {
                o++;
                /* Skip more newlines */
                while (o < verse_buf_len &&
                       (verse_buf[o] == '\n' || verse_buf[o] == '\r')) o++;
                break;
            }

            if (c < 32 || c > 126) c = ' ';
            if (c == ' ') last_sp = i;
            buf[i++] = (char)c;
            o++;
        }

        /* Word-wrap: if we filled the line and are mid-word, break at last space */
        if (i == LINE_W && last_sp > 0 && o < verse_buf_len) {
            unsigned char nc = verse_buf[o];
            if (nc != ' ' && nc != '\t' && nc != '\n' && nc != '\r') {
                /* Mid-word. Rewind to last space. */
                UINT16 rewind_amt = (UINT16)(i - last_sp - 1u);
                o -= rewind_amt;
                i = (UINT8)(last_sp + 1u);
            }
        }

        /* Pad remainder */
        for (j = i; j < LINE_W; j++) buf[j] = ' ';
        buf[LINE_W] = 0;

        draw_text(TEXT_X, (UINT8)(TEXT_TOP + line), buf);
    }
}

/* ---- Public API ---- */

void reader_init(void) BANKED {
    cur_chapter = 0;
    cur_verse = 1;
    page_offset = 0;
    buffer_current_verse();
    hold_timer = 0;
    repeating = 0;
    needs_redraw = 1;
}

void reader_force_redraw(void) BANKED {
    needs_redraw = 1;
}

void reader_update(UBYTE pressed, UBYTE held) BANKED {
    UINT8 do_next = 0;
    UINT8 do_prev = 0;

    /* LEFT = previous chapter */
    if (pressed & J_LEFT) {
        if (cur_chapter > 0u) {
            cur_chapter--;
            cur_verse = 1;
            page_offset = 0;
            buffer_current_verse();
            needs_redraw = 1;
        }
        hold_timer = 0;
        repeating = 0;
        return;
    }

    /* RIGHT = next chapter */
    if (pressed & J_RIGHT) {
        if (cur_chapter < NUM_CHAPTERS - 1u) {
            cur_chapter++;
            cur_verse = 1;
            page_offset = 0;
            buffer_current_verse();
            needs_redraw = 1;
        }
        hold_timer = 0;
        repeating = 0;
        return;
    }

    /* DOWN = next verse (fresh press) */
    if (pressed & J_DOWN) {
        do_next = 1;
        hold_timer = 0;
        repeating = 0;
    }

    /* UP = previous verse (fresh press) */
    if (pressed & J_UP) {
        do_prev = 1;
        hold_timer = 0;
        repeating = 0;
    }

    /* Hold-to-repeat for DOWN */
    if ((held & J_DOWN) && !(pressed & J_DOWN)) {
        hold_timer++;
        if (!repeating && hold_timer >= REPEAT_DELAY) {
            repeating = 1;
            hold_timer = 0;
        }
        if (repeating && hold_timer >= REPEAT_RATE) {
            hold_timer = 0;
            do_next = 1;
        }
    }

    /* Hold-to-repeat for UP */
    if ((held & J_UP) && !(pressed & J_UP)) {
        hold_timer++;
        if (!repeating && hold_timer >= REPEAT_DELAY) {
            repeating = 1;
            hold_timer = 0;
        }
        if (repeating && hold_timer >= REPEAT_RATE) {
            hold_timer = 0;
            do_prev = 1;
        }
    }

    /* Reset hold state when buttons released */
    if (!(held & (J_DOWN | J_UP))) {
        hold_timer = 0;
        repeating = 0;
    }

    /* Next verse (or next page of long verse) */
    if (do_next) {
        /* Check if current verse has more text past this page */
        UINT16 next_off = advance_page(page_offset);
        if (next_off < verse_buf_len) {
            /* More text in this verse — show next page */
            page_offset = next_off;
            needs_redraw = 1;
        } else if (cur_verse < verse_counts[cur_chapter]) {
            cur_verse++;
            page_offset = 0;
            buffer_current_verse();
            needs_redraw = 1;
        } else if (cur_chapter < NUM_CHAPTERS - 1u) {
            /* Advance to next chapter, verse 1 */
            cur_chapter++;
            cur_verse = 1;
            page_offset = 0;
            buffer_current_verse();
            needs_redraw = 1;
        }
        /* else: at John 21:25, can't go further */
    }

    /* Previous verse (or previous page of long verse) */
    if (do_prev) {
        if (page_offset > 0u) {
            /* Go back to first page of this verse */
            page_offset = 0;
            needs_redraw = 1;
        } else if (cur_verse > 1u) {
            cur_verse--;
            page_offset = 0;
            buffer_current_verse();
            needs_redraw = 1;
        } else if (cur_chapter > 0u) {
            /* Go to last verse of previous chapter */
            cur_chapter--;
            cur_verse = verse_counts[cur_chapter];
            page_offset = 0;
            buffer_current_verse();
            needs_redraw = 1;
        }
        /* else: at John 1:1, can't go back */
    }
}

void reader_render(void) BANKED {
    UINT8 verse_lines;
    UINT8 start_row;

    if (!needs_redraw) return;
    needs_redraw = 0;

    /* === Row 0: Header "~ JOHN [ch]:[verse] ~" centered === */
    {
        char hdr[21];
        char content[21];
        UINT8 pos, ch_num, v_num;
        UINT8 content_len, pad_left, j;

        ch_num = (UINT8)(cur_chapter + 1u);
        v_num = cur_verse;

        pos = 0;
        content[pos++] = '~';
        content[pos++] = ' ';
        content[pos++] = 'J';
        content[pos++] = 'O';
        content[pos++] = 'H';
        content[pos++] = 'N';
        content[pos++] = ' ';

        /* Chapter number */
        if (ch_num >= 10u) {
            content[pos++] = (char)('0' + (ch_num / 10u));
        }
        content[pos++] = (char)('0' + (ch_num % 10u));

        content[pos++] = ':';

        /* Verse number */
        if (v_num >= 10u) {
            content[pos++] = (char)('0' + (v_num / 10u));
        }
        content[pos++] = (char)('0' + (v_num % 10u));

        content[pos++] = ' ';
        content[pos++] = '~';

        content_len = pos;

        /* Center within 20 chars */
        pad_left = (UINT8)((20u - content_len) / 2u);

        for (j = 0; j < 20u; j++) hdr[j] = ' ';
        hdr[20] = 0;

        for (j = 0; j < content_len; j++) {
            hdr[pad_left + j] = content[j];
        }

        draw_text(0, 0, hdr);
    }

    /* === Row 1: Decorative border below header === */
    draw_text(0, 1, "....................");

    /* === Rows 2-15: Verse text (word-wrapped, vertically centered) === */
    verse_lines = count_verse_lines();
    if (verse_lines >= TEXT_LINES) {
        start_row = 0;
    } else {
        start_row = (UINT8)((TEXT_LINES - verse_lines) / 2u);
    }
    render_verse_text(start_row);

    /* === Row 16: Decorative border above nav hints === */
    draw_text(0, 16, "....................");

    /* === Row 17: Navigation hints === */
    draw_text(0, 17, "< CH   UP/DN   CH >");
}
