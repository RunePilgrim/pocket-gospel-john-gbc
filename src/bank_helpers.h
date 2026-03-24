/* bank_helpers.h — NONBANKED helpers for safe cross-bank data access.
 *
 * BANKED functions (executing from a switchable ROM bank) CANNOT call
 * SWITCH_ROM_MBC1() directly — doing so unmaps their own code, causing
 * the CPU to execute garbage from the newly-mapped bank.
 *
 * These helpers reside in bank 0 (always mapped) and safely perform
 * the bank switch, data operation, and bank restore before returning
 * to the caller.
 */

#ifndef BANK_HELPERS_H
#define BANK_HELPERS_H

#include <gb/gb.h>

/* Load BG tile data from a banked ROM source.
   Switches to 'bank', calls set_bkg_data, then restores the previous bank.
   Safe to call from any bank (including BANKED functions). */
void load_bkg_data_banked(UINT8 bank, UINT8 base, UINT8 count, const UINT8 *data);

/* Look up a verse's offset and length from the banked index table (bank 14).
   Writes the absolute byte offset and byte length to *out_off and *out_len.
   Safe to call from any bank. */
void verse_lookup_banked(UINT8 chapter, UINT8 verse, UINT32 *out_off, UINT16 *out_len);

#endif
