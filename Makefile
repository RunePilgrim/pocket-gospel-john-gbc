# author: william frick
# file name: Makefile
# function: builds pocket_gospel_john_gbc.gb - church world + scripture reader.

ROM_NAME = pocket_gospel_john_gbc.gb

LCC = /Users/runepilgrim/gbdk/bin/lcc
EMULATOR = /Applications/SameBoy.app/Contents/MacOS/SameBoy

SRC = src/main.c src/font.c src/font_tiles.c src/reader.c src/john_text.c \
      src/game.c src/transition.c src/map.c src/player.c src/interact.c \
      src/matching.c src/sound.c src/bank_helpers.c \
      src/church_tiles.c src/outdoor_tiles.c \
      src/match_tiles.c src/player_sprites.c src/title_tiles.c \
      src/pet.c src/pet_sprites.c

JOHN_TXT = assets/john.txt
JOHN_GEN_SCRIPT = tools/gen_john_text_banked.py
JOHN_GEN_STAMP  = src/.john_gen_stamp

CFLAGS = -Wm-yC -Wm-yoA
LFLAGS = -Wl-j -Wl-m -Wl-yt1

all: $(ROM_NAME)

$(JOHN_GEN_STAMP): $(JOHN_TXT) $(JOHN_GEN_SCRIPT)
	python3 $(JOHN_GEN_SCRIPT) $(JOHN_TXT) src
	touch $(JOHN_GEN_STAMP)

$(ROM_NAME): $(SRC) $(JOHN_GEN_STAMP)
	$(LCC) $(CFLAGS) $(LFLAGS) -o $(ROM_NAME) $(SRC) src/john_b*.c

run: $(ROM_NAME)
	$(EMULATOR) $(ROM_NAME)

clean:
	rm -f $(ROM_NAME) pocket_gospel_john_gbc.map pocket_gospel_john_gbc.noi
	rm -f src/john_b*.c src/.john_gen_stamp
