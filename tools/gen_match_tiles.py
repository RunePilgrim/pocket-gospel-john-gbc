#!/usr/bin/env python3
"""
gen_match_tiles.py - Generate 2bpp tile data for the memory matching game.

44 tiles: card back (4) + 8 symbols x 4 tiles (32) + 8 UI tiles.
All symbols defined as 16x16 ASCII art grids for easy editing.

Reads:  nothing (all handcrafted art)
Writes: src/match_tiles.c  (44 tiles, bank 13)
"""

import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
MATCH_OUT = PROJECT_ROOT / "src" / "match_tiles.c"


def grid_to_2bpp(grid):
    """Convert 8x8 grid of palette indices (0-3) to 16 bytes 2bpp."""
    data = []
    for row in grid:
        lo = hi = 0
        for bit in range(8):
            c = row[bit]
            lo |= ((c & 1) << (7 - bit))
            hi |= (((c >> 1) & 1) << (7 - bit))
        data.append(lo)
        data.append(hi)
    return data


def art16_to_tiles(art_str):
    """Parse 16-line ASCII art (16x16) into 4 tiles: [TL, TR, BL, BR]."""
    lines = [l.strip() for l in art_str.strip().split('\n') if l.strip()]
    assert len(lines) == 16, f"Need 16 rows, got {len(lines)}"
    for i, l in enumerate(lines):
        assert len(l) == 16, f"Row {i}: need 16 cols, got {len(l)}: '{l}'"

    tl = [[int(lines[y][x]) for x in range(8)] for y in range(8)]
    tr = [[int(lines[y][x]) for x in range(8, 16)] for y in range(8)]
    bl = [[int(lines[y][x]) for x in range(8)] for y in range(8, 16)]
    br = [[int(lines[y][x]) for x in range(8, 16)] for y in range(8, 16)]

    return [grid_to_2bpp(tl), grid_to_2bpp(tr),
            grid_to_2bpp(bl), grid_to_2bpp(br)]


def art8_to_tile(art_str):
    """Parse 8-line ASCII art (8x8) into 1 tile (16 bytes)."""
    lines = [l.strip() for l in art_str.strip().split('\n') if l.strip()]
    assert len(lines) == 8, f"Need 8 rows, got {len(lines)}"
    grid = [[int(lines[y][x]) for x in range(8)] for y in range(8)]
    return grid_to_2bpp(grid)


# ===================================================================
# CARD BACK (16x16)
# ===================================================================
CARD_BACK = """
3333333333333333
3122112211221123
3211221122112213
3122112211221123
3211221122112213
3122113113221123
3211223223112213
3122113113221123
3211223223112213
3122112211221123
3211221122112213
3122112211221123
3211221122112213
3122112211221123
3211221122112213
3333333333333333
"""

# ===================================================================
# SYMBOL 0: CROSS — clean Latin cross with c3 outline, c1 fill
# Large, bold, unmistakable.
# ===================================================================
CROSS = """
3333333333333333
3000000000000003
3000003333000003
3000003113000003
3000003113000003
3003333113333003
3003111111113003
3003333113333003
3000003113000003
3000003113000003
3000003113000003
3000003113000003
3000003333000003
3000000000000003
3000000000000003
3333333333333333
"""

# ===================================================================
# SYMBOL 1: DOVE — bird in flight facing right, larger and filled
# c1=body, c2=wing shading, c3=eye+outline
# ===================================================================
DOVE = """
3333333333333333
3000000000000003
3000000003310003
3000000031130003
3000001111310003
3001111111110003
3012211111100003
3011111111100003
3001111111100003
3000011111000003
3000001110000003
3000011001100003
3000110000110003
3000000000000003
3000000000000003
3333333333333333
"""

# ===================================================================
# SYMBOL 2: FISH (Ichthys) — classic fish outline, forked tail
# c1=outline, c2=eye, c3=border
# ===================================================================
FISH = """
3333333333333333
3000000000000003
3000000000000003
3000011111011003
3001100000110103
3010000020001003
3100000000000103
3100000000000103
3010000000001003
3001100000110003
3000011111011003
3000000000001003
3000000000000003
3000000000000003
3000000000000003
3333333333333333
"""

# ===================================================================
# SYMBOL 3: CHALICE — communion cup/goblet with c2 fill
# c3=outline, c2=body, c1=highlights
# ===================================================================
CUP = """
3333333333333333
3000000000000003
3001333333331003
3001322222231003
3000132222310003
3000013222100003
3000001321000003
3000000330000003
3000000330000003
3000000330000003
3000001331000003
3000013222100003
3000133333310003
3000000000000003
3000000000000003
3333333333333333
"""

# ===================================================================
# SYMBOL 4: LAMB — sheep profile with fluffy c2 wool
# c2=wool, c1=face/legs, c3=outline/eye/hooves
# ===================================================================
LAMB = """
3333333333333333
3000000000000003
3000000001331003
3000000013003003
3000022213101003
3000222221311003
3002222222110003
3002222222210003
3001222222110003
3000133333100003
3000010000100003
3000010000100003
3000013000130003
3000000000000003
3000000000000003
3333333333333333
"""

# ===================================================================
# SYMBOL 5: CROWN OF THORNS — circular ring with c3 thorns
# c1=ring body, c3=thorns+outline
# ===================================================================
CROWN = """
3333333333333333
3000003003000003
3000030000300003
3000011111100003
3000110000110003
3001100000011003
3031000000001303
3010000000000103
3010000000000103
3031000000001303
3001100000011003
3000110000110003
3000011111100003
3000030000300003
3000003003000003
3333333333333333
"""

# ===================================================================
# SYMBOL 6: BREAD — round loaf with cross-score
# c2=bread body, c1=highlights, c3=outline+score lines
# ===================================================================
BREAD = """
3333333333333333
3000000000000003
3000001111000003
3000112222110003
3001222322221003
3001222322221003
3012233333322103
3012233333322103
3001222322221003
3001222322221003
3000112222110003
3000001111000003
3000000000000003
3000000000000003
3000000000000003
3333333333333333
"""

# ===================================================================
# SYMBOL 7: FLAME — teardrop candle flame
# c1=inner bright, c2=outer glow, c3=border
# ===================================================================
FLAME = """
3333333333333333
3000000110000003
3000001221000003
3000001221000003
3000012112100003
3000012112100003
3000121001210003
3000121001210003
3000122112210003
3000012222100003
3000012222100003
3000001221000003
3000000330000003
3000000110000003
3000000000000003
3333333333333333
"""

# ===================================================================
# UI TILES (8x8 each)
# ===================================================================
CARD_BORDER_H = """
00000000
00000000
00000000
33333333
33333333
00000000
00000000
00000000
"""

CARD_BORDER_V = """
00033000
00033000
00033000
00033000
00033000
00033000
00033000
00033000
"""

CURSOR_TL = """
11111111
11111111
11000000
11000000
00000000
00000000
00000000
00000000
"""

CURSOR_TR = """
11111111
11111111
00000011
00000011
00000000
00000000
00000000
00000000
"""

CURSOR_BL = """
00000000
00000000
00000000
00000000
11000000
11000000
11111111
11111111
"""

CURSOR_BR = """
00000000
00000000
00000000
00000000
00000011
00000011
11111111
11111111
"""

MATCHED = """
00000000
00000001
00000011
00000110
11001100
01111000
00110000
00000000
"""

EMPTY = """
00000000
00000000
00000000
00000000
00000000
00000000
00000000
00000000
"""


# ===================================================================
# C file output
# ===================================================================
def format_tile(data, comment=""):
    """Format 16 bytes as C hex lines with optional comment."""
    l1 = ", ".join(f"0x{data[i]:02X}" for i in range(8))
    l2 = ", ".join(f"0x{data[i]:02X}" for i in range(8, 16))
    lines = []
    if comment:
        lines.append(f"    /* {comment} */")
    lines.append(f"    {l1},")
    lines.append(f"    {l2},")
    lines.append("")
    return "\n".join(lines)


def main():
    print("gen_match_tiles.py -- memory game tile generation")

    tiles = []   # list of (name, data_16bytes)
    tile_names = []

    # Card back (4 tiles)
    back = art16_to_tiles(CARD_BACK)
    for suffix, data in zip(["TL", "TR", "BL", "BR"], back):
        tiles.append((f"MT_BACK_{suffix}", data))

    # 8 symbols (4 tiles each = 32 tiles)
    symbols = [
        ("CROSS", CROSS),
        ("DOVE", DOVE),
        ("FISH", FISH),
        ("CUP", CUP),
        ("LAMB", LAMB),
        ("CROWN", CROWN),
        ("BREAD", BREAD),
        ("FLAME", FLAME),
    ]
    for sym_name, art in symbols:
        sym_tiles = art16_to_tiles(art)
        for suffix, data in zip(["TL", "TR", "BL", "BR"], sym_tiles):
            tiles.append((f"MT_{sym_name}_{suffix}", data))

    # UI tiles (8 tiles)
    ui_tiles = [
        ("MT_CARD_BORDER_H", CARD_BORDER_H),
        ("MT_CARD_BORDER_V", CARD_BORDER_V),
        ("MT_CURSOR_TL", CURSOR_TL),
        ("MT_CURSOR_TR", CURSOR_TR),
        ("MT_CURSOR_BL", CURSOR_BL),
        ("MT_CURSOR_BR", CURSOR_BR),
        ("MT_MATCHED", MATCHED),
        ("MT_EMPTY", EMPTY),
    ]
    for name, art in ui_tiles:
        tiles.append((name, art8_to_tile(art)))

    count = len(tiles)
    assert count == 44, f"Expected 44 tiles, got {count}"
    sz = count * 16

    print(f"  {count} tiles, {sz} bytes")

    # Write C file
    lines = [
        "/*",
        " * match_tiles.c -- 2bpp tile data for the memory matching game.",
        f" * {count} tiles, ROM bank 13.",
        " * Generated by tools/gen_match_tiles.py",
        " */",
        "",
        "#include <gb/gb.h>",
        "#include <gbdk/platform.h>",
        '#include "match_tiles.h"',
        "",
        "#pragma bank 13",
        "",
        f"const UINT8 match_tiles_data[{sz}] = {{",
        "",
    ]

    for i, (name, data) in enumerate(tiles):
        lines.append(format_tile(data, f"{name} ({i})"))

    lines.append("};")
    lines.append("")
    lines.append("BANKREF(match_tiles_data)")
    lines.append("")

    with open(MATCH_OUT, 'w') as f:
        f.write("\n".join(lines))
    print(f"  Written: {MATCH_OUT}")
    print("  Done!")


if __name__ == "__main__":
    main()
