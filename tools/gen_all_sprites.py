#!/usr/bin/env python3
"""
gen_all_sprites.py - Convert RPG spritesheet PNGs into banked C sprite data.

Reads all RPG_xxx.png files from assets/sprites-and-tilesets/sprites/ and
produces src/player_sprites.c containing 2bpp tile data for all characters.

Each character has 24 tiles (3 directions x 2 frames x 4 quadrants).
Layout matches the existing code expectation:
  Tiles 0-3:   DOWN_F0 (standing)  TL, TR, BL, BR
  Tiles 4-7:   DOWN_F1 (walking)
  Tiles 8-11:  UP_F0 (standing)
  Tiles 12-15: UP_F1 (walking)
  Tiles 16-19: LEFT_F0 (standing)
  Tiles 20-23: LEFT_F1 (walking)
RIGHT reuses LEFT tiles with S_FLIPX at runtime.

Frame order in the source PNGs:
  16x96 vertical / 96x16 horizontal (6 frames):
    [DOWN_stand, UP_stand, LEFT_stand, DOWN_walk, UP_walk, LEFT_walk]
  128x16 horizontal (8 frames):
    [DOWN_stand, DOWN_walk, UP_stand, UP_walk, LEFT_stand, LEFT_walk, extra, extra]
"""

import os
import sys
from PIL import Image

SPRITE_DIR = "assets/sprites-and-tilesets/sprites"
OUTPUT = "src/player_sprites.c"

# PNG pixel colors -> 2bpp color index
# Color 0 = transparent, 1 = lightest, 2 = mid, 3 = darkest
COLOR_MAP = {
    (0, 255, 0): 0,        # chroma green = transparent
    (224, 248, 207): 1,     # lightest
    (134, 192, 108): 2,     # mid
    (7, 24, 33): 3,         # darkest
    (0, 0, 0): 3,           # RPG_013 uses pure black for darkest
}

# For 6-frame sheets: PNG frame index for each code slot
# Code expects: DOWN_F0, DOWN_F1, UP_F0, UP_F1, LEFT_F0, LEFT_F1
# PNG has: 0=DOWN_S, 1=UP_S, 2=LEFT_S, 3=DOWN_W, 4=UP_W, 5=LEFT_W
FRAME_MAP_6 = [0, 3, 1, 4, 2, 5]

# For 8-frame sheets: PNG frame index for each code slot
# PNG has: 0=DOWN_S, 1=DOWN_W, 2=UP_S, 3=UP_W, 4=LEFT_S, 5=LEFT_W, 6=extra, 7=extra
FRAME_MAP_8 = [0, 1, 2, 3, 4, 5]


def pixel_to_color_idx(img, x, y):
    """Map a PNG pixel to a 2bpp color index (0-3)."""
    px = img.getpixel((x, y))
    rgb = (px[0], px[1], px[2])
    idx = COLOR_MAP.get(rgb)
    if idx is None:
        # Find closest match by Euclidean distance
        best = 0
        best_dist = 999999
        for ref, ci in COLOR_MAP.items():
            d = sum((a - b) ** 2 for a, b in zip(rgb, ref))
            if d < best_dist:
                best_dist = d
                best = ci
        return best
    return idx


def extract_tile_8x8(img, px_x, px_y):
    """Extract one 8x8 tile starting at (px_x, px_y), return 16 bytes (2bpp)."""
    tile = []
    for row in range(8):
        lo = 0
        hi = 0
        for col in range(8):
            cidx = pixel_to_color_idx(img, px_x + col, px_y + row)
            bit = 7 - col
            if cidx & 1:
                lo |= (1 << bit)
            if cidx & 2:
                hi |= (1 << bit)
        tile.append(lo)
        tile.append(hi)
    return tile


def extract_frame_16x16(img, frame_px_x, frame_px_y):
    """Extract a 16x16 frame as 4 tiles: TL, TR, BL, BR (each 16 bytes)."""
    tiles = []
    # TL (top-left 8x8)
    tiles.extend(extract_tile_8x8(img, frame_px_x, frame_px_y))
    # TR (top-right 8x8)
    tiles.extend(extract_tile_8x8(img, frame_px_x + 8, frame_px_y))
    # BL (bottom-left 8x8)
    tiles.extend(extract_tile_8x8(img, frame_px_x, frame_px_y + 8))
    # BR (bottom-right 8x8)
    tiles.extend(extract_tile_8x8(img, frame_px_x + 8, frame_px_y + 8))
    return tiles


def get_frame_positions(img):
    """Return list of (px_x, px_y) for each frame, and the frame map to use."""
    w, h = img.size
    if w == 16 and h == 96:
        # Vertical strip: 6 frames stacked
        positions = [(0, i * 16) for i in range(6)]
        return positions, FRAME_MAP_6
    elif w == 96 and h == 16:
        # Horizontal strip: 6 frames side by side
        positions = [(i * 16, 0) for i in range(6)]
        return positions, FRAME_MAP_6
    elif w == 128 and h == 16:
        # Horizontal strip: 8 frames side by side
        positions = [(i * 16, 0) for i in range(8)]
        return positions, FRAME_MAP_8
    else:
        raise ValueError(f"Unknown spritesheet size: {w}x{h}")


def convert_spritesheet(filepath):
    """Convert a spritesheet PNG to 384 bytes of 2bpp tile data.

    Returns the 24 tiles (384 bytes) ordered as the code expects:
    DOWN_F0(4 tiles), DOWN_F1(4), UP_F0(4), UP_F1(4), LEFT_F0(4), LEFT_F1(4)
    """
    img = Image.open(filepath).convert("RGBA")
    positions, frame_map = get_frame_positions(img)

    all_tiles = []
    for code_slot in range(6):
        png_frame_idx = frame_map[code_slot]
        fx, fy = positions[png_frame_idx]
        frame_tiles = extract_frame_16x16(img, fx, fy)
        all_tiles.extend(frame_tiles)

    assert len(all_tiles) == 384, f"Expected 384 bytes, got {len(all_tiles)}"
    return all_tiles


def format_c_array(data, name, indent="    "):
    """Format a byte array as a C initializer."""
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_vals = ", ".join(f"0x{b:02X}" for b in chunk)
        lines.append(f"{indent}{hex_vals}")
    return ",\n".join(lines)


def main():
    # Discover and sort sprite files
    files = sorted(
        f for f in os.listdir(SPRITE_DIR)
        if f.startswith("RPG_") and f.endswith(".png")
    )
    if not files:
        print("ERROR: No RPG_xxx.png files found in", SPRITE_DIR)
        sys.exit(1)

    print(f"Found {len(files)} sprite sheets:")
    for f in files:
        print(f"  {f}")

    # Convert all
    characters = []
    for f in files:
        path = os.path.join(SPRITE_DIR, f)
        data = convert_spritesheet(path)
        name = f.replace(".png", "").replace("-", "_").lower()
        characters.append((name, data))
        print(f"  Converted {f}: {len(data)} bytes")

    num_chars = len(characters)

    total_bytes = num_chars * 384

    # Generate C file
    with open(OUTPUT, "w") as out:
        out.write("/*\n")
        out.write(" * player_sprites.c - player sprite tile data for all characters.\n")
        out.write(f" * AUTOGENERATED by tools/gen_all_sprites.py - do not edit by hand.\n")
        out.write(f" * {num_chars} characters, 24 tiles each (3 dirs x 2 frames x 4 quads).\n")
        out.write(f" * Single contiguous array ({total_bytes} bytes) indexed by character.\n")
        out.write(" * RIGHT reuses LEFT tiles with S_FLIPX at runtime.\n")
        out.write(" */\n\n")
        out.write("#include <gb/gb.h>\n")
        out.write("#include <gbdk/platform.h>\n")
        out.write('#include "player_sprites.h"\n\n')
        out.write("#pragma bank 10\n\n")

        # Emit one big contiguous array with all character data
        out.write(f"/* All {num_chars} characters in one contiguous array.\n")
        out.write(f"   Character N starts at offset N*384. */\n")
        out.write(f"static const UINT8 all_sprite_data[{total_bytes}] = {{\n")
        for i, (name, data) in enumerate(characters):
            out.write(f"\n    /* Character {i}: {name} (offset {i*384}) */\n")
            out.write(format_c_array(data, name))
            if i < num_chars - 1:
                out.write(",\n")
            else:
                out.write("\n")
        out.write("};\n\n")

        # Compact NONBANKED load function — no switch, just index multiply
        out.write("/* Load all 24 sprite tiles for a character into sprite VRAM. */\n")
        out.write("void player_sprites_load_char(UINT8 idx) NONBANKED {\n")
        out.write("    UINT8 old_bank = _current_bank;\n")
        out.write(f"    if (idx >= {num_chars}u) idx = 0;\n")
        out.write("    SWITCH_ROM_MBC1(10);\n")
        out.write("    set_sprite_data(0, 24u, all_sprite_data + (UINT16)idx * 384u);\n")
        out.write("    SWITCH_ROM_MBC1(old_bank);\n")
        out.write("}\n\n")

        # Preview loader — loads DOWN_F0 (first 4 tiles = 64 bytes) as BG tiles
        out.write("/* Load DOWN standing preview (4 tiles) as BG tiles at index 128. */\n")
        out.write("void player_sprites_load_preview(UINT8 idx) NONBANKED {\n")
        out.write("    UINT8 old_bank = _current_bank;\n")
        out.write(f"    if (idx >= {num_chars}u) idx = 0;\n")
        out.write("    SWITCH_ROM_MBC1(10);\n")
        out.write("    set_bkg_data(128u, 4u, all_sprite_data + (UINT16)idx * 384u);\n")
        out.write("    SWITCH_ROM_MBC1(old_bank);\n")
        out.write("}\n\n")

        # Backward-compatible wrapper
        out.write("void player_sprites_load(void) NONBANKED {\n")
        out.write("    player_sprites_load_char(0);\n")
        out.write("}\n")

    print(f"\nWrote {OUTPUT}: {num_chars} characters, "
          f"{num_chars * 384} bytes of sprite data total")


if __name__ == "__main__":
    main()
