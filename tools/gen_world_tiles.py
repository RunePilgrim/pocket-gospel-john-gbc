#!/usr/bin/env python3
"""
gen_world_tiles.py - Generate 2bpp tile data for church and outdoor maps.

Hybrid approach:
  - Extracts tiles from the tilesheet for most game objects (trees, hedges,
    paths, roof, doors, windows, walls, furniture, fences, etc.)
  - Generates procedural tiles for grass
  - Handcrafts specialized tiles (candle, rope, cross, book, carpet, pond)

Reads:  assets/sprites-and-tilesets/background-tilesets/gameboy_outdoor_assets_v1.2.png
Writes: src/church_tiles.c  (48 tiles, bank 11)
        src/outdoor_tiles.c (70 tiles, bank 12)
"""

import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("ERROR: Pillow required. pip install Pillow")
    sys.exit(1)

PROJECT_ROOT = Path(__file__).resolve().parent.parent
TILESHEET = PROJECT_ROOT / "assets" / "sprites-and-tilesets" / "background-tilesets" / "gameboy_outdoor_assets_v1.2.png"
METAMODE = PROJECT_ROOT / "assets" / "sprites-and-tilesets" / "background-tilesets" / "Metamode pt 1.png"
CHURCH_OUT = PROJECT_ROOT / "src" / "church_tiles.c"
OUTDOOR_OUT = PROJECT_ROOT / "src" / "outdoor_tiles.c"

# GB green palette for color quantization
GB_PALETTE = [
    (224, 248, 207),  # c0 lightest
    (134, 192, 108),  # c1
    ( 48, 104,  80),  # c2
    (  7,  24,  33),  # c3 darkest
]

def nearest_gb(rgb):
    best, bd = 0, 999999
    for i, p in enumerate(GB_PALETTE):
        d = (rgb[0]-p[0])**2 + (rgb[1]-p[1])**2 + (rgb[2]-p[2])**2
        if d < bd:
            best, bd = i, d
    return best

# ---------------------------------------------------------------------------
# Tilesheet extraction
# ---------------------------------------------------------------------------
_sheet_img = None

def open_sheet():
    global _sheet_img
    if _sheet_img is None:
        _sheet_img = Image.open(TILESHEET).convert("RGB")
    return _sheet_img

def extract(col, row):
    """Extract 8x8 tile at (col,row) from tilesheet, return 16 bytes 2bpp."""
    img = open_sheet()
    x0, y0 = col * 8, row * 8
    grid = []
    for y in range(8):
        r = []
        for x in range(8):
            px = img.getpixel((x0 + x, y0 + y))
            r.append(nearest_gb(px[:3]))
        grid.append(r)
    return grid_to_2bpp(grid)

_meta_img = None

def open_meta():
    global _meta_img
    if _meta_img is None:
        _meta_img = Image.open(METAMODE).convert("RGB")
    return _meta_img

def extract_meta(col, row):
    """Extract 8x8 tile at (col,row) from Metamode tilesheet."""
    img = open_meta()
    x0, y0 = col * 8, row * 8
    grid = []
    for y in range(8):
        r = []
        for x in range(8):
            px = img.getpixel((x0 + x, y0 + y))
            r.append(nearest_gb(px[:3]))
        grid.append(r)
    return grid_to_2bpp(grid)

def extract_grid(col, row):
    """Extract 8x8 tile grid at (col,row) from tilesheet as color indices."""
    img = open_sheet()
    x0, y0 = col * 8, row * 8
    grid = []
    for y in range(8):
        r = []
        for x in range(8):
            px = img.getpixel((x0 + x, y0 + y))
            r.append(nearest_gb(px[:3]))
        grid.append(r)
    return grid

def extract_wall(col, row, dark_edges=""):
    """Extract tile and add dark 2-pixel borders on specified edges.
    dark_edges: string with 't','b','l','r' for edges to darken.
    Outer pixel -> c3, inner pixel -> c2 minimum.
    """
    img = open_sheet()
    x0, y0 = col * 8, row * 8
    grid = []
    for y in range(8):
        r = []
        for x in range(8):
            px = img.getpixel((x0 + x, y0 + y))
            c = nearest_gb(px[:3])
            if 't' in dark_edges:
                if y == 0: c = 3
                elif y == 1: c = max(c, 2)
            if 'b' in dark_edges:
                if y == 7: c = 3
                elif y == 6: c = max(c, 2)
            if 'l' in dark_edges:
                if x == 0: c = 3
                elif x == 1: c = max(c, 2)
            if 'r' in dark_edges:
                if x == 7: c = 3
                elif x == 6: c = max(c, 2)
            r.append(c)
        grid.append(r)
    return grid_to_2bpp(grid)

# ---------------------------------------------------------------------------
# 2bpp conversion
# ---------------------------------------------------------------------------
def grid_to_2bpp(grid):
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

def art_to_2bpp(art):
    """Parse 8-line ASCII art string -> 2bpp bytes."""
    lines = [l.strip() for l in art.strip().split('\n') if l.strip()]
    assert len(lines) == 8, f"Need 8 rows, got {len(lines)}"
    grid = []
    for l in lines:
        assert len(l) == 8, f"Need 8 cols, got {len(l)}: '{l}'"
        grid.append([int(c) for c in l])
    return grid_to_2bpp(grid)

def composite_on_floor(art, floor_col, floor_row):
    """Overlay ASCII art on a floor tile. c0 pixels in art become transparent,
    showing the floor texture underneath. Non-c0 pixels replace floor."""
    floor_grid = extract_grid(floor_col, floor_row)
    lines = [l.strip() for l in art.strip().split('\n') if l.strip()]
    assert len(lines) == 8, f"Need 8 rows, got {len(lines)}"
    grid = []
    for y in range(8):
        assert len(lines[y]) == 8, f"Need 8 cols, got {len(lines[y])}"
        row = []
        for x in range(8):
            obj_c = int(lines[y][x])
            if obj_c == 0:
                row.append(floor_grid[y][x])  # transparent: show floor
            else:
                row.append(obj_c)  # opaque: show object
        grid.append(row)
    return grid_to_2bpp(grid)


# ---------------------------------------------------------------------------
# Tile lists
# ---------------------------------------------------------------------------
OUT_TILES = []   # outdoor tile data (list of 16-byte lists)
OUT_NAMES = []
OUT_DESCS = []

CH_TILES = []    # church tile data
CH_NAMES = []
CH_DESCS = []

def ot(name, desc, data):
    OUT_TILES.append(data)
    OUT_NAMES.append(name)
    OUT_DESCS.append(desc)

def ct(name, desc, data):
    CH_TILES.append(data)
    CH_NAMES.append(name)
    CH_DESCS.append(desc)


# ===================================================================
# OUTDOOR TILES (70) — all from gameboy_outdoor_assets_v1.2.png
# ===================================================================
def build_outdoor():
    # --- GRASS: 24 tiles (cols 14-19, rows 4-7) ---
    for r in range(4):
        for c in range(6):
            idx = r * 6 + c
            ot(f"OT_GRASS_{idx:02d}",
               f"grass ({14+c},{4+r})",
               extract(14 + c, 4 + r))

    # --- BUILDING: 16 tiles (cols 12-15, rows 0-3) ---
    for r in range(4):
        for c in range(4):
            ot(f"OT_BLDG_R{r}C{c}",
               f"building ({12+c},{r})",
               extract(12 + c, r))

    # --- TREE 1: 8 tiles (cols 4-5, rows 4-7) ---
    ot("OT_TREE1_R0L", "tree1 crown-left (4,4)",   extract(4, 4))
    ot("OT_TREE1_R0R", "tree1 crown-right (5,4)",  extract(5, 4))
    ot("OT_TREE1_R1L", "tree1 mid-left (4,5)",     extract(4, 5))
    ot("OT_TREE1_R1R", "tree1 mid-right (5,5)",    extract(5, 5))
    ot("OT_TREE1_R2L", "tree1 lower-left (4,6)",   extract(4, 6))
    ot("OT_TREE1_R2R", "tree1 lower-right (5,6)",  extract(5, 6))
    ot("OT_TREE1_R3L", "tree1 trunk-left (4,7)",   extract(4, 7))
    ot("OT_TREE1_R3R", "tree1 trunk-right (5,7)",  extract(5, 7))

    # --- TREE 2: 6 tiles (cols 2-3, rows 5-7) ---
    ot("OT_TREE2_R0L", "tree2 top-left (2,5)",   extract(2, 5))
    ot("OT_TREE2_R0R", "tree2 top-right (3,5)",  extract(3, 5))
    ot("OT_TREE2_R1L", "tree2 mid-left (2,6)",   extract(2, 6))
    ot("OT_TREE2_R1R", "tree2 mid-right (3,6)",  extract(3, 6))
    ot("OT_TREE2_R2L", "tree2 base-left (2,7)",  extract(2, 7))
    ot("OT_TREE2_R2R", "tree2 base-right (3,7)", extract(3, 7))

    # --- HEDGE: 7 tiles (cols 20-23, rows 4-8) ---
    ot("OT_HEDGE_TL", "hedge top-left (20,4)",    extract(20, 4))
    ot("OT_HEDGE_TR", "hedge top-right (23,4)",   extract(23, 4))
    ot("OT_HEDGE_L",  "hedge left (20,6)",        extract(20, 6))
    ot("OT_HEDGE_R",  "hedge right (23,6)",       extract(23, 6))
    ot("OT_HEDGE_BL", "hedge bottom-left (20,8)", extract(20, 8))
    ot("OT_HEDGE_B",  "hedge bottom (21,8)",      extract(21, 8))
    ot("OT_HEDGE_BR", "hedge bottom-right (23,8)",extract(23, 8))

    # --- HEDGE TOP: 1 tile (21,4) ---
    ot("OT_HEDGE_T", "hedge top edge (21,4)", extract(21, 4))

    # --- SKY: 1 tile (solid dark) from (0,24) ---
    ot("OT_SKY", "night sky dark (0,24)", extract(0, 24))

    # --- STARS: 4 tiles with star patterns ---
    ot("OT_STAR_A", "big star (3,0)",   extract(3, 0))
    ot("OT_STAR_B", "small star (5,0)", extract(5, 0))
    ot("OT_STAR_C", "tiny dots (4,2)",  extract(4, 2))
    ot("OT_STAR_D", "tiny dots (5,2)",  extract(5, 2))

    # --- BUSH: 3 tiles (col 11, rows 0-2) ---
    ot("OT_BUSH_A", "bush variant A (11,0)", extract(11, 0))
    ot("OT_BUSH_B", "bush variant B (11,1)", extract(11, 1))
    ot("OT_BUSH_C", "bush variant C (11,2)", extract(11, 2))


# ===================================================================
# CHURCH TILES (48)
# ===================================================================
def build_church():
    # 0-1 - FLOOR: extract from texture library
    ct("CT_FLOOR_STONE", "floor TL", extract(14, 16))
    ct("CT_FLOOR_DARK", "floor TR", extract(15, 16))

    # 2-8 - WALLS: extract brick pattern from building area (12,1) with edge darkening
    ct("CT_WALL_TOP", "top wall edge", extract_wall(12, 1, 't'))
    ct("CT_WALL_MID", "middle wall fill", extract(12, 1))
    ct("CT_WALL_BOTTOM", "wall-to-floor transition", extract_wall(12, 1, 'b'))
    ct("CT_WALL_LEFT", "left wall edge", extract_wall(12, 1, 'l'))
    ct("CT_WALL_RIGHT", "right wall edge", extract_wall(12, 1, 'r'))
    ct("CT_WALL_CORNER_TL", "top-left corner", extract_wall(12, 1, 'tl'))
    ct("CT_WALL_CORNER_TR", "top-right corner", extract_wall(12, 1, 'tr'))

    # 9-11 - PEWS: handcrafted church pew composited on floor
    # Pew at row 14: PEW_LEFT at col 13 (FTL), PEW_MID at cols 14/16 (FTR),
    # PEW_RIGHT at col 17 (FTL). Clean wooden bench design.
    # Floor: FTL=(14,16), FTR=(15,16)
    ct("CT_PEW_LEFT", "pew left end (on FTL floor)", composite_on_floor("""
00000000
03333300
32222230
32112230
03333330
00300030
00300030
00000000
""", 14, 16))
    ct("CT_PEW_MID", "pew middle (on FTR floor)", composite_on_floor("""
00000000
33333333
22222222
21122112
33333333
03000030
00000000
00000000
""", 15, 16))
    ct("CT_PEW_RIGHT", "pew right end (on FTL floor)", composite_on_floor("""
00000000
00333330
03222230
03211230
03333330
03000300
03000300
00000000
""", 14, 16))

    # 12-15 - ALTAR: plant from tilesheet
    ct("CT_ALTAR_TL", "plant top-left", extract(24, 20))
    ct("CT_ALTAR_TR", "plant top-right", extract(25, 20))
    ct("CT_ALTAR_BL", "plant bottom-left", extract(24, 21))
    ct("CT_ALTAR_BR", "plant bottom-right", extract(25, 21))

    # 16 - BOOK: handcrafted (unique object)
    ct("CT_BOOK_OPEN", "open book on altar", art_to_2bpp("""
00000000
01333310
13111131
13101031
13010131
13111131
01333310
00000000
"""))

    # 17-18 - CANDLES (bottom half of tall candle): body + base on FBL floor
    # Floor: FBL=(14,17)
    # Tall candle body — connects to candle top tile above
    ct("CT_CANDLE_UNLIT", "tall candle base (on FBL floor)", composite_on_floor("""
00333000
00333000
00333000
00333000
00333000
03333300
03333300
33333330
""", 14, 17))
    ct("CT_CANDLE_LIT", "tall candle base lit (on FBL floor)", composite_on_floor("""
00333000
00333000
00333000
00333000
00333000
03333300
03333300
33333330
""", 14, 17))

    # 19-22 - ROPE: twisted cord with bell-shaped handle.
    # Rope top: wall-bottom texture with rope emerging
    _rope_wall = extract_grid(12, 1)
    # Add wall-bottom edge darkening (matching CT_WALL_BOTTOM)
    for _x in range(8):
        if _rope_wall[7][_x] < 3: _rope_wall[7][_x] = 3
        if _rope_wall[6][_x] < 2: _rope_wall[6][_x] = 2
    # Rope: 3px wide cord with twist pattern
    for _y in range(3, 8):
        if _y % 2 == 1:
            _rope_wall[_y][2] = 1; _rope_wall[_y][3] = 3; _rope_wall[_y][4] = 1
        else:
            _rope_wall[_y][2] = 3; _rope_wall[_y][3] = 1; _rope_wall[_y][4] = 3
    ct("CT_ROPE_TOP", "bell rope top (wall bg)", grid_to_2bpp(_rope_wall))

    # Rope mid at (col 16, row 5): FBL floor = (14,17)
    # Twisted rope — alternating c1/c3 pattern suggests braiding
    ct("CT_ROPE_MID", "bell rope mid (on FBL floor)", composite_on_floor("""
01310000
03130000
01310000
03130000
01310000
03130000
01310000
03130000
""", 14, 17))
    # Rope bottom at (col 16, row 6): FTR floor = (15,16)
    # Bell shape at bottom — instantly reads as "bell rope"
    ct("CT_ROPE_BOTTOM", "bell rope handle (on FTR floor)", composite_on_floor("""
00131000
00131000
01333100
13111310
13111310
13111310
01333100
00010000
""", 15, 16))
    ct("CT_ROPE_PULLED", "rope pulled (on FTR floor)", composite_on_floor("""
00001310
00013100
00131000
01310000
13100000
31000000
10000000
00000000
""", 15, 16))

    # 23-26 - DOOR: extract from building door area (12,2)-(13,3)
    ct("CT_DOOR_TL", "doorway top-left", extract(12, 2))
    ct("CT_DOOR_TR", "doorway top-right", extract(13, 2))
    ct("CT_DOOR_BL", "doorway bottom-left", extract(12, 3))
    ct("CT_DOOR_BR", "doorway bottom-right", extract(13, 3))

    # 27-30 - WINDOWS: night sky view using star tiles
    ct("CT_WINDOW_TL", "night sky window TL (star)", extract(3, 0))
    ct("CT_WINDOW_TR", "night sky window TR (star)", extract(5, 0))
    ct("CT_WINDOW_BL", "night sky window BL (star)", extract(4, 2))
    ct("CT_WINDOW_BR", "night sky window BR (star)", extract(5, 2))

    # 31-32 - CARPET: handcrafted (unique to church, no tilesheet match)
    ct("CT_CARPET_V", "aisle carpet vertical", art_to_2bpp("""
02222220
02112120
02222220
02121220
02222220
02112120
02222220
02121220
"""))
    ct("CT_CARPET_H", "floor BR", extract(15, 17))

    # 33-36 - TABLE: composited on floor at (cols 2-3, rows 13-14)
    # Table TL (col 2, row 13): FBL=(14,17)
    ct("CT_TABLE_TL", "table TL (on FBL floor)", composite_on_floor("""
00000000
03333333
32222222
32112212
32222222
32212221
32222222
32112212
""", 14, 17))
    # Table TR (col 3, row 13): FBR=(15,17)
    ct("CT_TABLE_TR", "table TR (on FBR floor)", composite_on_floor("""
00000000
33333330
22222223
21221123
22222223
12212223
22222223
21221123
""", 15, 17))
    # Table BL (col 2, row 14): FTR=(15,16)
    ct("CT_TABLE_BL", "table BL (on FTR floor)", composite_on_floor("""
32222222
32222222
03333333
00300030
00300030
00300030
00300030
00000000
""", 15, 16))
    # Table BR (col 3, row 14): FTL=(14,16)
    ct("CT_TABLE_BR", "table BR (on FTL floor)", composite_on_floor("""
22222223
22222223
33333330
03000300
03000300
03000300
03000300
00000000
""", 14, 16))

    # 37-38 - CANDLE TOP: tall candle upper portion on wall-bottom bg
    # Unlit: just the wick tip poking up against wall
    _wall_bot = extract_grid(12, 1)
    for _x in range(8):
        if _wall_bot[7][_x] < 3: _wall_bot[7][_x] = 3
        if _wall_bot[6][_x] < 2: _wall_bot[6][_x] = 2
    # Overlay candle wick/top on wall bg (c0 = transparent)
    _art_unlit_top = [
        "00000000",
        "00000000",
        "00000000",
        "00030000",
        "00030000",
        "00333000",
        "00333000",
        "00333000",
    ]
    _ct_grid = [row[:] for row in _wall_bot]
    for _y in range(8):
        for _x in range(8):
            _c = int(_art_unlit_top[_y][_x])
            if _c != 0:
                _ct_grid[_y][_x] = _c
    ct("CT_CANDLE_TOP_UNLIT", "tall candle top (no flame, on wall bg)", grid_to_2bpp(_ct_grid))

    # Lit: bright flame above wick
    _art_lit_top = [
        "00010000",
        "00111000",
        "01111100",
        "01111100",
        "00111000",
        "00333000",
        "00333000",
        "00333000",
    ]
    _ct_grid2 = [row[:] for row in _wall_bot]
    for _y in range(8):
        for _x in range(8):
            _c = int(_art_lit_top[_y][_x])
            if _c != 0:
                _ct_grid2[_y][_x] = _c
    ct("CT_CANDLE_TOP_LIT", "tall candle top (flame, on wall bg)", grid_to_2bpp(_ct_grid2))

    # 39-40 - PILLAR: extract from building area (14,2)-(14,3)
    ct("CT_PILLAR_TOP", "stone pillar top", extract(14, 2))
    ct("CT_PILLAR_BOT", "stone pillar bottom", extract(14, 3))

    # 41 - FLOOR_CARPET: floor BL
    ct("CT_FLOOR_CARPET", "floor BL", extract(14, 17))

    # 42 - WALL_DECO: handcrafted wall ornament
    ct("CT_WALL_DECO", "wall decoration", art_to_2bpp("""
11101111
11010111
10101011
01010101
10101011
11010111
11101111
11111111
"""))

    # 43 - SHELF: extract from furniture/building area (12,7)
    ct("CT_SHELF", "small shelf", extract(12, 7))

    # 44 - PEW_MID_B: pew middle variant composited on FTL floor (for col 15)
    ct("CT_PEW_MID_B", "pew mid on FTL floor", composite_on_floor("""
00000000
33333333
22222222
21122112
33333333
03000030
00000000
00000000
""", 14, 16))
    # 45 - BENCH_R: reserved (unused)
    ct("CT_BENCH_R", "reserved", art_to_2bpp("""
00000000
00000000
00000000
00000000
00000000
00000000
00000000
00000000
"""))

    # 46 - RAILING: handcrafted (unique church element)
    ct("CT_RAILING", "altar railing", art_to_2bpp("""
00000000
11111111
00000000
01001001
01001001
00000000
11111111
00000000
"""))

    # 47 - RESERVED
    ct("CT_RESERVED", "reserved", art_to_2bpp("""
00000000
00000000
00000000
00000000
00000000
00000000
00000000
00000000
"""))


# ===================================================================
# C file output
# ===================================================================
def format_bytes(data):
    l1 = ", ".join(f"0x{data[i]:02X}" for i in range(8))
    l2 = ", ".join(f"0x{data[i]:02X}" for i in range(8, 16))
    return f"    {l1},\n    {l2}"

def write_c(path, name, desc, bank, count, names, descs, tiles):
    sz = count * 16
    lines = [
        f"/*",
        f" * {name}_tiles.c -- 2bpp tile data for {desc}.",
        f" * {count} tiles, ROM bank {bank}.",
        f" * Generated by tools/gen_world_tiles.py",
        f" */",
        f"",
        f"#include <gb/gb.h>",
        f"#include <gbdk/platform.h>",
        f'#include "{name}_tiles.h"',
        f"",
        f"#pragma bank {bank}",
        f"",
        f"const UINT8 {name}_tiles_data[{sz}] = {{",
        f"",
    ]
    for i, td in enumerate(tiles):
        lines.append(f"    /* {names[i]} ({i}) -- {descs[i]} */")
        lines.append(format_bytes(td) + ",")
        lines.append("")
    lines += [f"}};", f"", f"BANKREF({name}_tiles_data)", f""]
    with open(path, 'w') as f:
        f.write("\n".join(lines))
    print(f"  Written: {path} ({sz} bytes)")


# ===================================================================
# Main
# ===================================================================
def main():
    print("gen_world_tiles.py -- tilesheet extraction + generation")
    print(f"  Tilesheet: {TILESHEET}")
    print()

    build_outdoor()
    build_church()

    assert len(OUT_TILES) == 70, f"outdoor: {len(OUT_TILES)}"
    assert len(CH_TILES) == 48, f"church: {len(CH_TILES)}"

    print(f"  Outdoor: {len(OUT_TILES)} tiles")
    for i, n in enumerate(OUT_NAMES):
        print(f"    [{i:2d}] {n}")
    print(f"  Church: {len(CH_TILES)} tiles")
    for i, n in enumerate(CH_NAMES):
        print(f"    [{i:2d}] {n}")
    print()

    write_c(OUTDOOR_OUT, "outdoor", "the outdoor courtyard", 12, 70,
            OUT_NAMES, OUT_DESCS, OUT_TILES)
    write_c(CHURCH_OUT, "church", "the church interior", 11, 48,
            CH_NAMES, CH_DESCS, CH_TILES)
    print("  Done!")

if __name__ == "__main__":
    main()
