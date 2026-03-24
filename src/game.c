/*
 * game.c — central state machine managing transitions between screens.
 * States: boot, char_select, church, outdoors, reader, matching.
 */

#include <gb/gb.h>
#include <gb/cgb.h>
#include "game.h"
#include "transition.h"
#include "map.h"
#include "player.h"
#include "player_sprites.h"
#include "interact.h"
#include "reader.h"
#include "matching.h"
#include "font.h"
#include "sound.h"
#include "pet.h"
#include "pet_sprites.h"

extern void draw_boot_screen(void);
extern const UWORD boot_pal[4];
extern const UWORD waterfall_pal[4];

/* CGB Palettes — all matched to tilesheet green palette */
/* Tilesheet colors: c0=(224,248,207) c1=(134,192,108) c2=(48,104,80) c3=(7,24,33) */

/* Tilesheet green gradient — shared across church and outdoor areas */
static const UWORD pal_green[4] = {
    RGB(28, 31, 26),   /* lightest green */
    RGB(17, 24, 14),   /* mid-light green */
    RGB(6, 13, 10),    /* mid-dark green */
    RGB(1, 3, 4)       /* darkest green */
};

/* Text palette: c1=darkest for font readability (reader, matching, menus) */
static const UWORD pal_text[4] = {
    RGB(28, 31, 26),   /* lightest green (background) */
    RGB(1, 3, 4),      /* darkest green (font glyphs) */
    RGB(6, 13, 10),    /* mid-dark green */
    RGB(17, 24, 14)    /* mid-light green */
};

static UINT8 current_state;
static UINT8 pending_state;   /* state to transition to */
static UINT8 trans_phase;     /* 0=none, 1=fading out, 2=load new, 3=fading in */
static UINT8 boot_timer;
static UINT8 interact_result; /* result from last interaction check */
static UINT8 confirm_wait;    /* waiting for A/B confirmation on prompt */
static UINT8 player_inited;   /* 0 until first player_init() call */
static UINT8 selected_char;   /* character index chosen on boot screen */
static UINT8 selected_pet;

/* Clear entire BG map to tile 0 and reset CGB palette attributes to pal 0 */
static void clear_screen(void) {
    UINT8 y;
    unsigned char blank_row[20];
    unsigned char attr_row[20];
    UINT8 i;

    for (i = 0; i < 20u; i++) {
        blank_row[i] = 0;
        attr_row[i] = 0;
    }

    for (y = 0; y < 18u; y++) {
        set_bkg_tiles(0, y, 20, 1, blank_row);
    }

    /* Reset CGB palette attributes */
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        for (y = 0; y < 18u; y++) {
            set_bkg_tiles(0, y, 20, 1, attr_row);
        }
        VBK_REG = 0;
    }
}

static void set_all_palettes(const UWORD *pal) {
    if (_cpu == CGB_TYPE) {
        set_bkg_palette(0, 1, pal);
        set_bkg_palette(1, 1, pal);
        set_bkg_palette(2, 1, pal);
    }
    fade_set_base_bg_palette(0, pal);
    fade_set_base_bg_palette(1, pal);
    fade_set_base_bg_palette(2, pal);
}

static void enter_state(UINT8 state) {
    current_state = state;

    switch (state) {
        case STATE_BOOT:
            boot_timer = 0;
            music_set_track(TRACK_MENU);
            if (_cpu == CGB_TYPE) {
                set_bkg_palette(0, 1, boot_pal);
                set_bkg_palette(2, 1, waterfall_pal);
                fade_set_base_bg_palette(0, boot_pal);
                fade_set_base_bg_palette(2, waterfall_pal);
            }
            font_load();
            draw_boot_screen();
            break;

        case STATE_CHURCH:
            music_set_track(TRACK_CHURCH);
            set_all_palettes(pal_green);
            font_load();
            map_load(MAP_CHURCH);
            map_draw();
            {
                MapData *md = map_get_current();
                if (!player_inited) {
                    player_init(md->spawn_x, md->spawn_y);
                    player_inited = 1;
                } else {
                    player_init(md->spawn_x, md->spawn_y);
                    player_show();
                }
                pet_init(md->spawn_x, (UINT8)(md->spawn_y + 2u));
            }
            interact_init();
            confirm_wait = 0;
            SHOW_SPRITES;
            break;

        case STATE_OUTDOORS:
            music_set_track(TRACK_OUTDOORS);
            set_all_palettes(pal_green);
            font_load();
            map_load(MAP_OUTDOORS);
            map_draw();
            {
                MapData *md = map_get_current();
                player_init(md->spawn_x, md->spawn_y);
                player_inited = 1;
                pet_init(md->spawn_x, (UINT8)(md->spawn_y - 2u));
            }
            interact_init();
            confirm_wait = 0;
            SHOW_SPRITES;
            break;

        case STATE_READER:
            music_set_track(TRACK_READER);
            player_hide();
            pet_hide();
            HIDE_SPRITES;
            clear_screen();
            set_all_palettes(pal_text);
            font_load();          /* ensure font tiles are present */
            /* reader_init() only called first time - position persists */
            reader_force_redraw();
            reader_render();
            break;

        case STATE_MATCHING:
            player_hide();
            pet_hide();
            HIDE_SPRITES;
            clear_screen();
            set_all_palettes(pal_text);
            font_load();          /* ensure font tiles are present */
            matching_init();
            matching_render();
            break;

        case STATE_CHAR_SELECT:
            HIDE_SPRITES;
            clear_screen();
            set_all_palettes(pal_text);
            font_load();
            selected_char = 0;
            select_reset_char();
            draw_text(2, 3, "CHOOSE A PILGRIM");
            select_draw_char(selected_char);
            draw_text(5, 15, "PRESS START");
            draw_text(7, 16, "B:BACK");
            break;

        case STATE_PET_SELECT:
            HIDE_SPRITES;
            clear_screen();
            set_all_palettes(pal_text);
            font_load();
            selected_pet = 0;
            pet_reset_preview();
            draw_text(2, 3, "CHOOSE YOUR BUDDY");
            pet_draw_preview(selected_pet);
            draw_text(5, 15, "PRESS START");
            draw_text(7, 16, "B:BACK");
            break;

        case STATE_CREDITS:
            HIDE_SPRITES;
            clear_screen();
            set_all_palettes(pal_text);
            font_load();
            credits_draw();
            break;
    }
}

void game_init(void) {
    current_state = STATE_BOOT;
    pending_state = STATE_BOOT;
    trans_phase = 0;
    boot_timer = 0;
    interact_result = INTERACT_NONE;
    confirm_wait = 0;
    player_inited = 0;
    selected_char = 0;
    selected_pet = 0;

    reader_init(); /* initialize reader offset to 0 */
}

void game_change_state(UINT8 new_state) {
    if (trans_phase != 0) return; /* already transitioning */

    pending_state = new_state;
    trans_phase = 1;
    fade_out_start();
}

UINT8 game_get_state(void) {
    return current_state;
}

UINT8 game_is_transitioning(void) {
    return (trans_phase != 0) ? 1u : 0u;
}

void game_update(UBYTE pressed, UBYTE held) {
    /* Handle fade transitions */
    if (trans_phase == 1) {
        /* Fading out */
        if (fade_update()) {
            /* Fade out complete - load new state */
            fade_to_black();
            trans_phase = 2;
        }
        return;
    }

    if (trans_phase == 2) {
        /* Load new state while screen is black */
        enter_state(pending_state);
        fade_in_start();
        trans_phase = 3;
        return;
    }

    if (trans_phase == 3) {
        /* Fading in */
        if (fade_update()) {
            trans_phase = 0; /* done */
        }
        return;
    }

    /* Normal state update */
    switch (current_state) {
        case STATE_BOOT:
            boot_timer++;
            if (pressed & J_START) {
                sfx_start();
                game_change_state(STATE_CHAR_SELECT);
            }
            if (pressed & J_SELECT) {
                sfx_confirm();
                game_change_state(STATE_CREDITS);
            }
            break;

        case STATE_CHAR_SELECT:
            /* LEFT/RIGHT cycles through characters */
            if (pressed & J_RIGHT) {
                sfx_confirm();
                selected_char = (UINT8)((selected_char + 1u) % NUM_CHARACTERS);
                select_draw_char(selected_char);
            }
            if (pressed & J_LEFT) {
                sfx_confirm();
                if (selected_char == 0) {
                    selected_char = (UINT8)(NUM_CHARACTERS - 1u);
                } else {
                    selected_char = (UINT8)(selected_char - 1u);
                }
                select_draw_char(selected_char);
            }
            if (pressed & J_B) {
                sfx_cancel();
                game_change_state(STATE_BOOT);
            }
            if (pressed & J_START) {
                sfx_start();
                player_set_character(selected_char);
                game_change_state(STATE_PET_SELECT);
            }
            break;

        case STATE_PET_SELECT:
            if (pressed & J_RIGHT) {
                sfx_confirm();
                selected_pet = (UINT8)((selected_pet + 1u) % NUM_PETS);
                pet_draw_preview(selected_pet);
            }
            if (pressed & J_LEFT) {
                sfx_confirm();
                if (selected_pet == 0) {
                    selected_pet = (UINT8)(NUM_PETS - 1u);
                } else {
                    selected_pet = (UINT8)(selected_pet - 1u);
                }
                pet_draw_preview(selected_pet);
            }
            if (pressed & J_B) {
                sfx_cancel();
                game_change_state(STATE_CHAR_SELECT);
            }
            if (pressed & J_START) {
                sfx_start();
                pet_set_animal(selected_pet);
                game_change_state(STATE_CHURCH);
            }
            break;

        case STATE_CREDITS:
            if (pressed & (J_B | J_START)) {
                sfx_cancel();
                game_change_state(STATE_BOOT);
            }
            break;

        case STATE_CHURCH:
        case STATE_OUTDOORS:
            /* SELECT toggles music on/off */
            if (pressed & J_SELECT) {
                music_toggle();
            }

            /* Handle confirmation prompts */
            if (confirm_wait) {
                if (pressed & J_A) {
                    sfx_confirm();
                    interact_hide_prompt();
                    confirm_wait = 0;
                    if (interact_result == INTERACT_READER) {
                        game_change_state(STATE_READER);
                    } else if (interact_result == INTERACT_MATCHING) {
                        game_change_state(STATE_MATCHING);
                    }
                    interact_result = INTERACT_NONE;
                } else if (pressed & J_B) {
                    sfx_cancel();
                    interact_hide_prompt();
                    confirm_wait = 0;
                    interact_result = INTERACT_NONE;
                }
                break;
            }

            /* Dismiss info prompts on any button */
            if (interact_prompt_visible()) {
                if (pressed & (J_A | J_B)) {
                    sfx_confirm();
                    interact_hide_prompt();
                }
                break;
            }

            /* Player movement — track tile changes for pet trail */
            {
                UINT8 prev_px = player_get_x();
                UINT8 prev_py = player_get_y();
                player_update(pressed, held);
                if (player_get_x() != prev_px || player_get_y() != prev_py) {
                    pet_push_position(prev_px, prev_py);
                }
            }
            pet_update();

            /* Sky scroll (outdoor only): use pixel Y for smooth transitions */
            if (current_state == STATE_OUTDOORS) {
                map_update_scroll(player_get_pixel_y(), held);
            }

            /* Check for door transitions (walking onto door tile) */
            {
                UINT8 px_t, py_t;
                MapObject *door_obj;
                px_t = player_get_x();
                py_t = player_get_y();

                /* Check if player is on a door tile */
                door_obj = map_get_object(px_t, py_t);
                if (!door_obj) {
                    door_obj = map_get_object((UINT8)(px_t + 1u), py_t);
                }
                if (!door_obj) {
                    door_obj = map_get_object(px_t, (UINT8)(py_t + 1u));
                }
                if (!door_obj) {
                    door_obj = map_get_object((UINT8)(px_t + 1u), (UINT8)(py_t + 1u));
                }

                if (door_obj) {
                    if (door_obj->type == OBJ_DOOR_OUT && current_state == STATE_CHURCH) {
                        game_change_state(STATE_OUTDOORS);
                        break;
                    }
                    if (door_obj->type == OBJ_DOOR_IN && current_state == STATE_OUTDOORS) {
                        game_change_state(STATE_CHURCH);
                        break;
                    }
                }
            }

            /* A button: interact with facing object */
            if (pressed & J_A) {
                interact_result = interact_check();
                if (interact_result == INTERACT_READER ||
                    interact_result == INTERACT_MATCHING) {
                    confirm_wait = 1; /* wait for A/B confirmation */
                } else if (interact_result == INTERACT_DOOR_OUT) {
                    game_change_state(STATE_OUTDOORS);
                } else if (interact_result == INTERACT_DOOR_IN) {
                    game_change_state(STATE_CHURCH);
                }
                /* CANDLE and BELL are handled immediately by interact_check() */
            }

            /* Update ongoing animations */
            interact_update();
            break;

        case STATE_READER:
            reader_update(pressed, held);
            /* B exits reader back to church */
            if (pressed & J_B) {
                game_change_state(STATE_CHURCH);
            }
            break;

        case STATE_MATCHING:
            if (matching_update(pressed, held)) {
                /* B was pressed - exit to church */
                game_change_state(STATE_CHURCH);
            }
            break;
    }
}

void game_render(void) {
    switch (current_state) {
        case STATE_BOOT:
        case STATE_CHAR_SELECT:
        case STATE_PET_SELECT:
        case STATE_CREDITS:
            break;

        case STATE_CHURCH:
        case STATE_OUTDOORS:
            player_render();
            pet_render();
            break;

        case STATE_READER:
            reader_render();
            break;

        case STATE_MATCHING:
            matching_render();
            break;
    }
}
