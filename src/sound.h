/*
 * sound.h — Game Boy audio: background music + sound effects.
 * Music runs on Ch2 (square) + Ch3 (wave bass drone).
 * SFX run on Ch1 (square w/ sweep) + Ch4 (noise).
 */

#ifndef SOUND_H
#define SOUND_H

#include <gb/gb.h>

/* Initialize sound hardware (call once at boot) */
void sound_init(void) BANKED;

/* Advance music sequencer (call once per frame in main loop) */
void sound_update(void) BANKED;

/* Music track identifiers */
#define TRACK_MENU     0u  /* "Christ is Risen" — Paschal Troparion */
#define TRACK_CHURCH   1u  /* "O Gladsome Light" — Svete Tikhiy */
#define TRACK_OUTDOORS 2u  /* "Trisagion" — Holy God, Holy Mighty */
#define TRACK_READER   3u  /* Byzantine ison chant — quiet, contemplative */

/* Music control */
void music_set_track(UINT8 track) BANKED;
void music_start(void) BANKED;
void music_stop(void) BANKED;
UINT8 music_toggle(void) BANKED;

/* Sound effects */
void sfx_confirm(void) BANKED;
void sfx_cancel(void) BANKED;
void sfx_start(void) BANKED;
void sfx_card_flip(void) BANKED;
void sfx_match(void) BANKED;
void sfx_mismatch(void) BANKED;
void sfx_bell(void) BANKED;

#endif
