/*
 * bank_helpers.c — NONBANKED helper functions for cross-bank data access.
 *
 * These functions reside in bank 0 (home bank, always mapped) and safely
 * switch ROM banks to load data, then restore the previous bank before
 * returning.
 *
 * Required because BANKED functions in bank 15 cannot call SWITCH_ROM_MBC1()
 * without unmapping their own code — causing illegal opcode crashes.
 *
 * NO #pragma bank here: this file goes in bank 0.
 */

#include <gb/gb.h>
#include <gbdk/platform.h>
#include "bank_helpers.h"
#include "john_text.h"

void load_bkg_data_banked(UINT8 bank, UINT8 base, UINT8 count, const UINT8 *data) NONBANKED {
    UINT8 old_bank = _current_bank;
    SWITCH_ROM_MBC1(bank);
    set_bkg_data(base, count, data);
    SWITCH_ROM_MBC1(old_bank);
}

void verse_lookup_banked(UINT8 chapter, UINT8 verse, UINT32 *out_off, UINT16 *out_len) NONBANKED {
    UINT8 old_bank = _current_bank;
    UINT16 flat_idx;
    SWITCH_ROM_MBC1(BANK(john_verse_idx));
    flat_idx = (UINT16)(john_ch_first[chapter] + (UINT16)(verse - 1u));
    *out_off = john_verse_off[flat_idx];
    *out_len = john_verse_len[flat_idx];
    SWITCH_ROM_MBC1(old_bank);
}
