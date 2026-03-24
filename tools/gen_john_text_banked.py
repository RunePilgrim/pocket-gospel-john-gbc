#!/usr/bin/env python3
"""
author: william frick
file name: gen_john_text_banked.py
function: splits assets/john.txt into banked ROM chunks + generates john_text.[ch] accessors.
          also builds a pre-computed verse offset index for fast reader navigation.

Key detail:
- BANKREF(...) and BANKREF_EXTERN(...) are emitted WITHOUT a trailing semicolon to avoid SDCC parse errors.
- Text is preprocessed at build time: boilerplate headers, footnotes, navigation
  markers, and copyright text are stripped. Only clean verse text is stored in ROM.
"""
from pathlib import Path
import sys
import math
import re
import unicodedata

CHUNK_TARGET_BYTES = 12000  # keep well under 16KB
VERSE_IDX_BANK = 14         # ROM bank for the verse index table

# Standard verse counts for the Gospel of John (chapters 1-21)
VERSE_COUNTS = [51, 25, 36, 54, 47, 71, 53, 59, 41, 42,
                57, 50, 38, 31, 27, 33, 26, 40, 42, 31, 25]


def clean_john_text(raw_text: str) -> tuple:
    """
    Preprocess raw john.txt into clean verse text.
    Returns (clean_bytes, chapter_offsets) where chapter_offsets is a list of
    byte positions where each chapter starts in the clean stream.
    """
    # Split by chapter headers
    parts = re.split(r'World English Bible Classic John \d+\s*\n', raw_text)
    ch_nums = re.findall(r'World English Bible Classic John (\d+)', raw_text)

    clean_chapters = []

    for ch_idx, ch_num_str in enumerate(ch_nums):
        ch_num = int(ch_num_str)
        ch_text = parts[ch_idx + 1]
        lines = ch_text.split('\n')

        # Find the verse text line - it's the line starting with " N N " (chapter verse1)
        verse_line = None
        for line in lines:
            stripped = line.strip()
            # Match: "chapter_num 1 " at the start (verse 1)
            if re.match(rf'^{ch_num}\s+1\s+', stripped):
                verse_line = stripped
                break

        if not verse_line:
            print(f"WARNING: No verse text found for John {ch_num}", file=sys.stderr)
            clean_chapters.append(f"[Chapter {ch_num} text not found]")
            continue

        # Strip leading chapter number: "1 1  In the..." -> "1  In the..."
        verse_line = re.sub(rf'^{ch_num}\s+', '', verse_line)

        # Remove inline footnote spans
        # Footnotes are: † text until next verse number, ‡ text..., § text..., * text...
        # The pattern is: footnote_marker + text + (period or end before next verse num)
        # Strategy: remove everything from a footnote marker (†‡§) to the next verse number
        # For *, only remove " * text " patterns (space-star-space-Uppercase...)

        # First handle footnote markers (†, ‡, §)
        # These insert text like: "† The word translated... it."
        # The footnote text ends just before the next verse number pattern " N  "
        # Remove: from marker to next " \d+ " (verse number with double space)
        for marker in ['†', '‡', '§']:
            while marker in verse_line:
                idx = verse_line.index(marker)
                # Find next verse number after this point
                rest = verse_line[idx+1:]
                # Look for next verse number: " NN  " (number followed by double space)
                m = re.search(r'(?<!\d)(\d{1,3})\s{2}', rest)
                if m:
                    # Remove from marker to just before the verse number
                    end_idx = idx + 1 + m.start()
                    verse_line = verse_line[:idx] + verse_line[end_idx:]
                else:
                    # Footnote at end of chapter - remove to end, but keep trailing period
                    verse_line = verse_line[:idx]
                    break

        # Handle * footnotes: " * Text here. "
        # These are typically scripture references: "* Isaiah 40:3 "
        # Remove " * " followed by text up to next verse number
        while True:
            m = re.search(r' \* [A-Z]', verse_line)
            if not m:
                break
            idx = m.start()
            rest = verse_line[idx+2:]
            # Find next verse number
            m2 = re.search(r'(?<!\d)(\d{1,3})\s{2}', rest)
            if m2:
                end_idx = idx + 2 + m2.start()
                verse_line = verse_line[:idx] + ' ' + verse_line[end_idx:]
            else:
                verse_line = verse_line[:idx]
                break

        # Normalize whitespace: collapse multiple spaces to single
        verse_line = re.sub(r'  +', ' ', verse_line)
        verse_line = verse_line.strip()

        # Convert any remaining non-ASCII to ASCII approximations or strip
        clean = []
        for c in verse_line:
            if ord(c) < 128:
                clean.append(c)
            else:
                # Try to transliterate common chars
                name = unicodedata.name(c, '')
                if 'APOSTROPHE' in name or ('SINGLE' in name and 'QUOTATION' in name):
                    clean.append("'")
                elif 'QUOTATION' in name:
                    clean.append('"')
                elif 'DASH' in name or 'MINUS' in name:
                    clean.append('-')
                elif 'ELLIPSIS' in name:
                    clean.append('...')
                # Skip other non-ASCII (Greek chars from footnotes that leaked)

        verse_text = ''.join(clean)

        # Final cleanup: remove double spaces again
        verse_text = re.sub(r'  +', ' ', verse_text)
        verse_text = verse_text.strip()

        # Insert newline before each verse number so each verse starts on its own line.
        # Pattern: space + digit(s) + space at a verse boundary (not inside a word).
        # We look for " N " where N is 2+ (verse 1 is already at the start).
        verse_text = re.sub(r' (\d{1,3}) ', r'\n\1 ', verse_text)

        clean_chapters.append(verse_text)

    # Build the final byte stream with chapter separators (newlines)
    # Use a single newline as chapter separator
    output = []
    chapter_offsets = []
    pos = 0

    for i, ch_text in enumerate(clean_chapters):
        chapter_offsets.append(pos)
        text_bytes = ch_text.encode('ascii', errors='replace')
        output.append(text_bytes)
        pos += len(text_bytes)
        if i < len(clean_chapters) - 1:
            output.append(b'\n')
            pos += 1

    final_data = b''.join(output)

    # Print chapter offsets for embedding in reader.c
    print("Chapter byte offsets for reader.c:", file=sys.stderr)
    offset_strs = []
    for i, off in enumerate(chapter_offsets):
        offset_strs.append(f"{off}UL")
        if (i + 1) % 5 == 0:
            print(f"    {', '.join(offset_strs)},", file=sys.stderr)
            offset_strs = []
    if offset_strs:
        print(f"    {', '.join(offset_strs)}", file=sys.stderr)

    print(f"Total clean text size: {len(final_data)} bytes", file=sys.stderr)

    return final_data, chapter_offsets


def build_verse_index(data: bytes, chapter_offsets: list) -> tuple:
    """
    Walk the processed byte stream and record each verse's text offset and length.
    Returns (verse_offsets, verse_lengths, ch_first, max_len) where:
      verse_offsets[i] = absolute byte offset of verse text start (after "N " prefix)
      verse_lengths[i] = byte length of verse text
      ch_first[ch]     = index of first verse of chapter ch in the flat array
      max_len           = longest verse in bytes
    """
    verse_offsets = []
    verse_lengths = []
    ch_first = []

    num_chapters = len(VERSE_COUNTS)
    total_text = len(data)

    for ch_idx in range(num_chapters):
        ch_start = chapter_offsets[ch_idx]

        # Chapter text ends before the \n separator (or at total_text for last chapter)
        if ch_idx < num_chapters - 1:
            ch_text_end = chapter_offsets[ch_idx + 1] - 1
        else:
            ch_text_end = total_text

        ch_first.append(len(verse_offsets))
        ch_data = data[ch_start:ch_text_end]
        search_from = 0

        for v in range(1, VERSE_COUNTS[ch_idx] + 1):
            if v == 1:
                # Verse 1: skip digit(s) + space at chapter start
                p = 0
                while p < len(ch_data) and chr(ch_data[p]).isdigit():
                    p += 1
                while p < len(ch_data) and ch_data[p] == ord(' '):
                    p += 1
                text_start_local = p
            else:
                # Find "\nN " pattern from current search position
                needle = f"\n{v} ".encode('ascii')
                idx = ch_data.find(needle, search_from)
                if idx == -1:
                    print(f"WARNING: John {ch_idx + 1}:{v} not found in text",
                          file=sys.stderr)
                    text_start_local = len(ch_data)
                else:
                    text_start_local = idx + len(needle)

            # Find verse end: next \n or end of chapter
            next_nl = ch_data.find(b'\n', text_start_local)
            if next_nl == -1:
                text_end_local = len(ch_data)
            else:
                text_end_local = next_nl

            text_len = text_end_local - text_start_local
            verse_offsets.append(ch_start + text_start_local)
            verse_lengths.append(text_len)
            search_from = text_start_local

    max_len = max(verse_lengths) if verse_lengths else 0
    total_verses = len(verse_offsets)
    expected = sum(VERSE_COUNTS)

    print(f"Verse index: {total_verses} verses (expected {expected})", file=sys.stderr)
    print(f"Max verse length: {max_len} bytes", file=sys.stderr)

    if total_verses != expected:
        print(f"ERROR: verse count mismatch!", file=sys.stderr)
        sys.exit(1)

    # Spot-check: John 1:1 should start with "In"
    off = verse_offsets[0]
    check = data[off:off+2]
    if check != b'In':
        print(f"WARNING: John 1:1 starts with {check!r}, expected b'In'",
              file=sys.stderr)

    return verse_offsets, verse_lengths, ch_first, max_len


ACCESSOR_C_TOP = """/*
author: william frick
file name: john_text.c
function: byte-level accessor for banked John text chunks.
*/

#include <gb/gb.h>
#include <gbdk/platform.h>

#include "john_text.h"

/* Ensure BANK(symbol) resolves (pairs with BANKREF(symbol) in the defining file) */
{bankref_externs}

/* Chunk lengths stored in bank 0 (home bank) so they are always accessible.
   DO NOT read john_chunk_XX_len directly — those are in banked ROM and will
   return garbage if the wrong bank is mapped. */
static const UINT16 chunk_lengths[JOHN_CHUNK_COUNT] = {{
{chunk_lengths_array}
}};

static UINT16 chunk_len(UINT8 i) {{
    if (i < 1u || i > JOHN_CHUNK_COUNT) return 0;
    return chunk_lengths[i - 1u];
}}

UINT32 john_total_size(void) {{
    UINT32 total = 0;
    UINT8 i;
    for (i = 1; i <= JOHN_CHUNK_COUNT; i++) {{
        total += (UINT32)chunk_len(i);
    }}
    return total;
}}

unsigned char john_read_byte(UINT32 offset) {{
    UINT8 old_bank = _current_bank;
    UINT32 pos = offset;

    UINT8 i;
    for (i = 1; i <= JOHN_CHUNK_COUNT; i++) {{
        UINT16 len = chunk_len(i);
        if (!len) break;

        if (pos < (UINT32)len) {{
            unsigned char v = 0;

            switch(i) {{
{read_cases}
                default: v = 0; break;
            }}

            SWITCH_ROM_MBC1(old_bank);
            return v;
        }}

        pos -= (UINT32)len;
    }}

    SWITCH_ROM_MBC1(old_bank);
    return 0;
}}

void john_read_buf(UINT32 offset, UINT16 len, unsigned char *dst) {{
    UINT8 old_bank = _current_bank;
    UINT32 pos = offset;
    UINT16 remaining = len;
    UINT16 dst_idx = 0;

    UINT8 i;
    for (i = 1; i <= JOHN_CHUNK_COUNT && remaining > 0; i++) {{
        UINT16 clen = chunk_len(i);
        if (!clen) break;

        if (pos < (UINT32)clen) {{
            UINT16 local = (UINT16)pos;
            UINT16 avail = clen - local;
            UINT16 to_copy = (remaining < avail) ? remaining : avail;
            UINT16 j;

            switch(i) {{
{bulk_read_cases}
                default: break;
            }}

            dst_idx += to_copy;
            remaining -= to_copy;
            pos = 0;
            continue;
        }}

        pos -= (UINT32)clen;
    }}

    SWITCH_ROM_MBC1(old_bank);
}}
"""

HEADER_H_TOP = """/* generated file - do not edit */
#ifndef JOHN_TEXT_H
#define JOHN_TEXT_H

#include <gb/gb.h>
#include <gbdk/platform.h>

#define JOHN_CHUNK_COUNT {chunk_count}
#define JOHN_VERSE_COUNT {verse_count}
#define JOHN_MAX_VERSE_LEN {max_verse_len}
#define JOHN_NUM_CHAPTERS {num_chapters}

"""

HEADER_H_BOTTOM = """
/* Verse index table (bank 14) */
extern const UINT32 john_verse_off[];
extern const UINT16 john_verse_len[];
extern const UINT16 john_ch_first[];
BANKREF_EXTERN(john_verse_idx)

UINT32 john_total_size(void);
unsigned char john_read_byte(UINT32 offset);
void john_read_buf(UINT32 offset, UINT16 len, unsigned char *dst);

#endif
"""

def split_bytes(data: bytes, target: int):
    chunks = []
    n = len(data)
    if n == 0:
        return [b""]
    count = max(1, math.ceil(n / target))
    start = 0
    for _ in range(count):
        end = min(n, start + target)
        chunks.append(data[start:end])
        start = end
    return chunks

def emit_chunk_c(out_dir: Path, idx1: int, chunk: bytes):
    bank_num = idx1
    name = f"john_chunk_{idx1:02d}"
    fname = out_dir / f"john_b{idx1:02d}.c"

    hex_list = [f"0x{b:02X}" for b in chunk]
    wrapped = []
    for i in range(0, len(hex_list), 24):
        wrapped.append(",".join(hex_list[i:i+24]))

    body = "\n".join("    " + line + ("," if n != len(wrapped)-1 else "") for n, line in enumerate(wrapped))

    ctext = f"""/*
author: william frick
file name: {fname.name}
function: banked chunk {idx1} of the John text, stored in ROM bank {bank_num}.
*/

#include <gb/gb.h>
#include <gbdk/platform.h>

#pragma bank {bank_num}

const unsigned char {name}[] = {{
{body}
}};

const UINT16 {name}_len = {len(chunk)};

/* No trailing semicolon on BANKREF */
BANKREF({name})
"""
    fname.write_text(ctext, encoding="utf-8")

def emit_verse_index(out_dir: Path, verse_offsets: list, verse_lengths: list,
                     ch_first: list, max_len: int):
    """Write the pre-computed verse index table into a banked C file."""
    fname = out_dir / "john_bidx.c"

    # Format offset array (UINT32, 8 per line)
    off_lines = []
    for i in range(0, len(verse_offsets), 8):
        batch = verse_offsets[i:i+8]
        off_lines.append("    " + ",".join(f"{v}UL" for v in batch))
    off_body = ",\n".join(off_lines)

    # Format length array (UINT16, 12 per line)
    len_lines = []
    for i in range(0, len(verse_lengths), 12):
        batch = verse_lengths[i:i+12]
        len_lines.append("    " + ",".join(str(v) for v in batch))
    len_body = ",\n".join(len_lines)

    # Format ch_first array
    ch_first_str = ",".join(str(v) for v in ch_first)

    ctext = f"""/* generated file - do not edit */

#include <gb/gb.h>
#include <gbdk/platform.h>

#pragma bank {VERSE_IDX_BANK}

const UINT32 john_verse_off[{len(verse_offsets)}] = {{
{off_body}
}};

const UINT16 john_verse_len[{len(verse_lengths)}] = {{
{len_body}
}};

const UINT16 john_ch_first[{len(ch_first)}] = {{
    {ch_first_str}
}};

/* No trailing semicolon on BANKREF */
BANKREF(john_verse_idx)
"""
    fname.write_text(ctext, encoding="utf-8")

def emit_header(out_dir: Path, chunk_count: int, verse_count: int,
                max_verse_len: int, num_chapters: int):
    h = [HEADER_H_TOP.format(
        chunk_count=chunk_count,
        verse_count=verse_count,
        max_verse_len=max_verse_len,
        num_chapters=num_chapters,
    )]
    for n in range(1, chunk_count + 1):
        h.append(f"extern const unsigned char john_chunk_{n:02d}[];")
        h.append(f"extern const UINT16 john_chunk_{n:02d}_len;")
        h.append("")
    h.append(HEADER_H_BOTTOM)
    (out_dir / "john_text.h").write_text("\n".join(h), encoding="utf-8")

def emit_accessor(out_dir: Path, chunk_count: int, chunk_sizes: list):
    read_cases = []
    bulk_read_cases = []
    bankref_externs = []

    for n in range(1, chunk_count + 1):
        # No trailing semicolon on BANKREF_EXTERN
        bankref_externs.append(f"BANKREF_EXTERN(john_chunk_{n:02d})")
        read_cases.append(
            f"                case {n}: SWITCH_ROM_MBC1(BANK(john_chunk_{n:02d})); v = john_chunk_{n:02d}[(UINT16)pos]; break;"
        )
        bulk_read_cases.append(
            f"                case {n}: SWITCH_ROM_MBC1(BANK(john_chunk_{n:02d})); for (j = 0; j < to_copy; j++) dst[dst_idx + j] = john_chunk_{n:02d}[local + j]; break;"
        )

    # Verse index BANKREF_EXTERN so BANK(john_verse_idx) resolves
    bankref_externs.append("BANKREF_EXTERN(john_verse_idx)")

    # Format chunk lengths array for bank-0 storage
    chunk_lengths_strs = [f"    {sz}u" for sz in chunk_sizes]
    chunk_lengths_array = ",\n".join(chunk_lengths_strs)

    ctext = ACCESSOR_C_TOP.format(
        bankref_externs="\n".join(bankref_externs),
        chunk_lengths_array=chunk_lengths_array,
        read_cases="\n".join(read_cases),
        bulk_read_cases="\n".join(bulk_read_cases),
    )
    (out_dir / "john_text.c").write_text(ctext, encoding="utf-8")

def main():
    if len(sys.argv) != 3:
        print("usage: gen_john_text_banked.py <input_txt> <out_dir>", file=sys.stderr)
        return 2

    in_path = Path(sys.argv[1])
    out_dir = Path(sys.argv[2])
    out_dir.mkdir(parents=True, exist_ok=True)

    raw_text = in_path.read_text(encoding='utf-8')
    data, chapter_offsets = clean_john_text(raw_text)

    chunks = split_bytes(data, CHUNK_TARGET_BYTES)

    for p in out_dir.glob("john_b*.c"):
        p.unlink()

    for i, ch in enumerate(chunks, start=1):
        emit_chunk_c(out_dir, i, ch)

    # Build verse index from processed text
    verse_offsets, verse_lengths, ch_first, max_len = build_verse_index(
        data, chapter_offsets)

    emit_header(out_dir, len(chunks), len(verse_offsets), max_len,
                len(VERSE_COUNTS))
    chunk_sizes = [len(ch) for ch in chunks]
    emit_accessor(out_dir, len(chunks), chunk_sizes)
    emit_verse_index(out_dir, verse_offsets, verse_lengths, ch_first, max_len)

    (out_dir / ".john_gen_stamp").write_text("", encoding="utf-8")
    print(f"Generated {len(chunks)} banked chunk file(s) + verse index into {out_dir}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
