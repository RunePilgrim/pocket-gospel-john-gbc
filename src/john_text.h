/* generated file - do not edit */
#ifndef JOHN_TEXT_H
#define JOHN_TEXT_H

#include <gb/gb.h>
#include <gbdk/platform.h>

#define JOHN_CHUNK_COUNT 9
#define JOHN_VERSE_COUNT 879
#define JOHN_MAX_VERSE_LEN 290
#define JOHN_NUM_CHAPTERS 21


extern const unsigned char john_chunk_01[];
extern const UINT16 john_chunk_01_len;

extern const unsigned char john_chunk_02[];
extern const UINT16 john_chunk_02_len;

extern const unsigned char john_chunk_03[];
extern const UINT16 john_chunk_03_len;

extern const unsigned char john_chunk_04[];
extern const UINT16 john_chunk_04_len;

extern const unsigned char john_chunk_05[];
extern const UINT16 john_chunk_05_len;

extern const unsigned char john_chunk_06[];
extern const UINT16 john_chunk_06_len;

extern const unsigned char john_chunk_07[];
extern const UINT16 john_chunk_07_len;

extern const unsigned char john_chunk_08[];
extern const UINT16 john_chunk_08_len;

extern const unsigned char john_chunk_09[];
extern const UINT16 john_chunk_09_len;


/* Verse index table (bank 14) */
extern const UINT32 john_verse_off[];
extern const UINT16 john_verse_len[];
extern const UINT16 john_ch_first[];
BANKREF_EXTERN(john_verse_idx)

UINT32 john_total_size(void);
unsigned char john_read_byte(UINT32 offset);
void john_read_buf(UINT32 offset, UINT16 len, unsigned char *dst);

#endif
