/*
author: william frick
file name: john_text.c
function: byte-level accessor for banked John text chunks.
*/

#include <gb/gb.h>
#include <gbdk/platform.h>

#include "john_text.h"

/* Ensure BANK(symbol) resolves (pairs with BANKREF(symbol) in the defining file) */
BANKREF_EXTERN(john_chunk_01)
BANKREF_EXTERN(john_chunk_02)
BANKREF_EXTERN(john_chunk_03)
BANKREF_EXTERN(john_chunk_04)
BANKREF_EXTERN(john_chunk_05)
BANKREF_EXTERN(john_chunk_06)
BANKREF_EXTERN(john_chunk_07)
BANKREF_EXTERN(john_chunk_08)
BANKREF_EXTERN(john_chunk_09)
BANKREF_EXTERN(john_verse_idx)

/* Chunk lengths stored in bank 0 (home bank) so they are always accessible.
   DO NOT read john_chunk_XX_len directly — those are in banked ROM and will
   return garbage if the wrong bank is mapped. */
static const UINT16 chunk_lengths[JOHN_CHUNK_COUNT] = {
    12000u,
    12000u,
    12000u,
    12000u,
    12000u,
    12000u,
    12000u,
    12000u,
    302u
};

static UINT16 chunk_len(UINT8 i) {
    if (i < 1u || i > JOHN_CHUNK_COUNT) return 0;
    return chunk_lengths[i - 1u];
}

UINT32 john_total_size(void) {
    UINT32 total = 0;
    UINT8 i;
    for (i = 1; i <= JOHN_CHUNK_COUNT; i++) {
        total += (UINT32)chunk_len(i);
    }
    return total;
}

unsigned char john_read_byte(UINT32 offset) {
    UINT8 old_bank = _current_bank;
    UINT32 pos = offset;

    UINT8 i;
    for (i = 1; i <= JOHN_CHUNK_COUNT; i++) {
        UINT16 len = chunk_len(i);
        if (!len) break;

        if (pos < (UINT32)len) {
            unsigned char v = 0;

            switch(i) {
                case 1: SWITCH_ROM_MBC1(BANK(john_chunk_01)); v = john_chunk_01[(UINT16)pos]; break;
                case 2: SWITCH_ROM_MBC1(BANK(john_chunk_02)); v = john_chunk_02[(UINT16)pos]; break;
                case 3: SWITCH_ROM_MBC1(BANK(john_chunk_03)); v = john_chunk_03[(UINT16)pos]; break;
                case 4: SWITCH_ROM_MBC1(BANK(john_chunk_04)); v = john_chunk_04[(UINT16)pos]; break;
                case 5: SWITCH_ROM_MBC1(BANK(john_chunk_05)); v = john_chunk_05[(UINT16)pos]; break;
                case 6: SWITCH_ROM_MBC1(BANK(john_chunk_06)); v = john_chunk_06[(UINT16)pos]; break;
                case 7: SWITCH_ROM_MBC1(BANK(john_chunk_07)); v = john_chunk_07[(UINT16)pos]; break;
                case 8: SWITCH_ROM_MBC1(BANK(john_chunk_08)); v = john_chunk_08[(UINT16)pos]; break;
                case 9: SWITCH_ROM_MBC1(BANK(john_chunk_09)); v = john_chunk_09[(UINT16)pos]; break;
                default: v = 0; break;
            }

            SWITCH_ROM_MBC1(old_bank);
            return v;
        }

        pos -= (UINT32)len;
    }

    SWITCH_ROM_MBC1(old_bank);
    return 0;
}

void john_read_buf(UINT32 offset, UINT16 len, unsigned char *dst) {
    UINT8 old_bank = _current_bank;
    UINT32 pos = offset;
    UINT16 remaining = len;
    UINT16 dst_idx = 0;

    UINT8 i;
    for (i = 1; i <= JOHN_CHUNK_COUNT && remaining > 0; i++) {
        UINT16 clen = chunk_len(i);
        if (!clen) break;

        if (pos < (UINT32)clen) {
            UINT16 local = (UINT16)pos;
            UINT16 avail = clen - local;
            UINT16 to_copy = (remaining < avail) ? remaining : avail;
            UINT16 j;

            switch(i) {
                case 1: SWITCH_ROM_MBC1(BANK(john_chunk_01)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_01[local + j]; break;
                case 2: SWITCH_ROM_MBC1(BANK(john_chunk_02)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_02[local + j]; break;
                case 3: SWITCH_ROM_MBC1(BANK(john_chunk_03)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_03[local + j]; break;
                case 4: SWITCH_ROM_MBC1(BANK(john_chunk_04)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_04[local + j]; break;
                case 5: SWITCH_ROM_MBC1(BANK(john_chunk_05)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_05[local + j]; break;
                case 6: SWITCH_ROM_MBC1(BANK(john_chunk_06)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_06[local + j]; break;
                case 7: SWITCH_ROM_MBC1(BANK(john_chunk_07)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_07[local + j]; break;
                case 8: SWITCH_ROM_MBC1(BANK(john_chunk_08)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_08[local + j]; break;
                case 9: SWITCH_ROM_MBC1(BANK(john_chunk_09)); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_09[local + j]; break;
                default: break;
            }

            dst_idx += to_copy;
            remaining -= to_copy;
            pos = 0;
            continue;
        }

        pos -= (UINT32)clen;
    }

    SWITCH_ROM_MBC1(old_bank);
}
