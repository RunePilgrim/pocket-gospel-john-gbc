# Pocket Gospel: The Book of John

A Game Boy Color application that presents the full text of the Gospel of John (World English Bible) in a readable, navigable format. Built in C using GBDK-2020.

Developed by William Frick as a solo capstone project (CMSC 495, Spring 2026).

## Features

- Full text of the Gospel of John (879 verses, 21 chapters) with verse-by-verse navigation, word-wrap, and vertical centering
- Interactive church interior with altar, candle, bell rope, card table, and door
- Outdoor courtyard with scrolling night sky using hardware parallax
- 16 selectable pilgrim characters with 4-directional walk animation
- 9 companion pets that follow the player around
- 4x4 memory matching game with biblical symbols
- Four background music tracks based on Orthodox hymn melodies
- CGB color palettes with smooth fade transitions between screens
- 256 KB MBC1 ROM, CGB-only

## Running the ROM

Download `pocket_gospel_john_gbc.gb` and open it in any Game Boy Color emulator:

- **SameBoy** (recommended): https://sameboy.github.io/
- **BGB**: https://bgb.bircd.org/
- **mGBA**: https://mgba.io/

The ROM also runs on original hardware via flash cartridge (tested on EverDrive GB X7 + ModRetro Chromatic). It requires a Game Boy Color — it will not run on the original DMG Game Boy.

## Building from Source

Requirements:
- **GBDK-2020**: https://github.com/gbdk-2020/gbdk-2020
- **Python 3** with **Pillow** (`pip install Pillow`) for asset generation scripts
- macOS, Linux, or Windows with Make

Update the `LCC` path in the Makefile to point to your GBDK installation, then:

```
make clean && make
```

This regenerates the banked Gospel text from `assets/john.txt`, compiles all source files, and produces the ROM.

To regenerate tile or sprite data from the source PNGs in `assets/sprites-and-tilesets/`:

```
python3 tools/gen_world_tiles.py
python3 tools/gen_all_sprites.py
python3 tools/gen_pet_sprites.py
python3 tools/gen_font_tiles.py
python3 tools/gen_match_tiles.py
```

## Project Structure

```
src/                  C source and headers (compiled into ROM)
assets/john.txt       Gospel of John plain text (World English Bible)
assets/sprites-and-tilesets/  Source PNGs for tile and sprite generation
tools/                Python scripts for asset generation
```

## Controls

| Screen | Input | Action |
|--------|-------|--------|
| Title | START | Begin |
| Title | SELECT | View credits |
| Character / Pet Select | LEFT/RIGHT | Cycle options |
| Character / Pet Select | START | Confirm |
| Character / Pet Select | B | Go back |
| Church / Outdoors | D-pad | Move |
| Church / Outdoors | A | Interact |
| Church / Outdoors | SELECT | Toggle music |
| Scripture Reader | UP/DOWN | Previous / next verse |
| Scripture Reader | LEFT/RIGHT | Previous / next chapter |
| Scripture Reader | B | Return to church |
| Matching Game | D-pad | Move cursor |
| Matching Game | A | Flip card |
| Matching Game | B | Exit to church |

## Credits

- Scripture text: [World English Bible](https://worldenglish.bible/) (public domain)
- Player and environment tiles: [colortv/SP001](https://colortv.itch.io/sp001)
- Background tilesets: [materialfuture/gameboy-assets](https://materialfuture.itch.io/gameboy-assets)
- Pet sprites: [gamebowgames/16x16-small-animals](https://gamebowgames.itch.io/16x16-small-animals-for-use-with-gbstudio)
- Hymn melodies: [Orthodox Church of America](https://www.oca.org/liturgics/music)

## License

See [LICENSE](LICENSE) for details.
