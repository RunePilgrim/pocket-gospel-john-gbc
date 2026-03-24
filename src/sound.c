/*
 * sound.c — Game Boy audio via direct hardware register writes.
 *
 * Channel allocation:
 *   Ch1 (square + sweep) : sound effects
 *   Ch2 (square)         : background melody
 *   Ch3 (wave)           : bass drone
 *   Ch4 (noise)          : noise-based SFX (currently unused, reserved)
 *
 * Four music tracks based on Orthodox hymns:
 *   Track 0 (Menu):     "Christ is Risen" — Paschal Troparion, upbeat march
 *   Track 1 (Church):   "O Gladsome Light" — Svete Tikhiy, contemplative
 *   Track 2 (Outdoors): "Trisagion" — Holy God, solemn processional
 *   Track 3 (Reader):   Byzantine ison chant — very slow, quiet, meditative
 *
 * SELECT toggles music on/off.
 */

#pragma bank 15

#include <gb/gb.h>
#include <gb/hardware.h>
#include "sound.h"

/* ------------------------------------------------------------------ */
/*  Note frequency values (11-bit, for Ch1/Ch2)                       */
/*  Formula: freq_val = 2048 - (131072 / note_hz)                     */
/*  Ch3 wave uses the same values but sounds one octave lower.        */
/* ------------------------------------------------------------------ */
#define N_REST  0u
#define N_C4    1547u
#define N_D4    1602u
#define N_E4    1650u
#define N_F4    1673u
#define N_G4    1714u
#define N_A4    1750u
#define N_B4    1783u
#define N_C5    1798u

/* ------------------------------------------------------------------ */
/*  Wave RAM pattern — triangle wave for a warm bass tone             */
/* ------------------------------------------------------------------ */
static const UBYTE wave_triangle[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
};

/* ------------------------------------------------------------------ */
/*  Track 0 — "Christ is Risen" (Paschal Troparion, Tone 5)           */
/*  Joyful, march-like. Used for title / menu screen.                 */
/* ------------------------------------------------------------------ */
#define TRACK0_LEN 28u

static const UINT16 track0_freq[TRACK0_LEN] = {
    /* "Christ is risen from the dead" */
    N_E4, N_E4, N_G4, N_A4, N_A4, N_G4, N_E4, N_REST,
    /* "Trampling down death by death" */
    N_E4, N_G4, N_A4, N_C5, N_C5, N_A4, N_REST,
    /* "And on those in the tombs" */
    N_A4, N_G4, N_E4, N_D4, N_E4, N_REST,
    /* "Bestowing life!" */
    N_D4, N_E4, N_G4, N_E4, N_D4, N_E4, N_REST
};

static const UINT8 track0_dur[TRACK0_LEN] = {
    15, 15, 15, 30, 15, 15, 30, 10,
    15, 15, 20, 30, 15, 30, 10,
    15, 15, 15, 20, 30, 10,
    15, 20, 30, 15, 15, 45, 15
};

/* ------------------------------------------------------------------ */
/*  Track 1 — "O Gladsome Light" (Svete Tikhiy, Kievan Chant)        */
/*  Contemplative, slow stepwise motion. Used for church interior.    */
/* ------------------------------------------------------------------ */
#define TRACK1_LEN 32u

static const UINT16 track1_freq[TRACK1_LEN] = {
    /* "O Gladsome Light" */
    N_E4, N_F4, N_E4, N_D4, N_REST,
    /* "of the holy glory" */
    N_D4, N_E4, N_F4, N_E4, N_D4, N_REST,
    /* "of the immortal Father" */
    N_C4, N_D4, N_E4, N_F4, N_E4, N_D4, N_REST,
    /* "Heavenly, holy, blessed" */
    N_E4, N_F4, N_G4, N_F4, N_E4, N_D4, N_REST,
    /* "Jesus Christ" */
    N_E4, N_D4, N_C4, N_REST,
    /* gentle resolution */
    N_D4, N_E4, N_D4
};

static const UINT8 track1_dur[TRACK1_LEN] = {
    40, 40, 40, 60, 20,
    30, 30, 40, 40, 60, 20,
    30, 30, 30, 40, 40, 60, 20,
    30, 30, 40, 40, 40, 60, 20,
    40, 40, 60, 20,
    40, 40, 60
};

/* ------------------------------------------------------------------ */
/*  Track 2 — "Trisagion" (Holy God, Holy Mighty, Tone 2)             */
/*  Solemn, processional, narrow range. Used for outdoors.            */
/* ------------------------------------------------------------------ */
#define TRACK2_LEN 28u

static const UINT16 track2_freq[TRACK2_LEN] = {
    /* "Holy God" */
    N_E4, N_E4, N_D4, N_REST,
    /* "Holy Mighty" */
    N_E4, N_E4, N_F4, N_E4, N_REST,
    /* "Holy Immortal" */
    N_E4, N_F4, N_E4, N_D4, N_REST,
    /* "Have mercy on us" */
    N_D4, N_E4, N_D4, N_C4, N_D4, N_REST,
    /* Second phrase — variation */
    N_E4, N_F4, N_E4, N_D4,
    N_C4, N_D4, N_E4, N_REST
};

static const UINT8 track2_dur[TRACK2_LEN] = {
    24, 24, 36, 18,
    24, 24, 24, 36, 18,
    24, 24, 24, 36, 18,
    18, 24, 18, 24, 36, 24,
    24, 24, 24, 36,
    24, 24, 48, 24
};

/* ------------------------------------------------------------------ */
/*  Track 3 — Byzantine Ison Chant (reader)                           */
/*  Extremely slow, sparse E-Phrygian. Mostly dwells on E with tiny   */
/*  half-step neighbor motion (E-F, D-E). Played at low volume with   */
/*  75% duty for a warm, organ-like tone. ~27 seconds per loop.       */
/* ------------------------------------------------------------------ */
#define TRACK3_LEN 22u

static const UINT16 track3_freq[TRACK3_LEN] = {
    /* Opening E drone */
    N_E4, N_REST,
    /* Small E-F-E ornament (Phrygian half-step) */
    N_E4, N_F4, N_E4, N_REST,
    /* D neighbor, return to E */
    N_D4, N_E4, N_REST,
    /* Gentle ascending arc: E-F-G-F-E */
    N_E4, N_F4, N_G4, N_F4, N_E4, N_REST,
    /* Low descent to C, then rise */
    N_D4, N_C4, N_D4, N_REST,
    /* Final resolution on E (long) */
    N_D4, N_E4, N_REST
};

static const UINT8 track3_dur[TRACK3_LEN] = {
    90, 40,
    70, 60, 90, 30,
    70, 100, 40,
    60, 60, 70, 60, 100, 40,
    70, 80, 70, 40,
    70, 120, 60
};

/* ------------------------------------------------------------------ */
/*  Sequencer state                                                   */
/* ------------------------------------------------------------------ */
static const UINT16 *trk_freq;    /* pointer to current track freq array */
static const UINT8  *trk_dur;     /* pointer to current track dur array */
static UINT8         trk_len;     /* length of current track */
static UINT8         cur_track;   /* current track index (0-2) */

static UINT8  music_pos;       /* current index into melody arrays */
static UINT8  music_timer;     /* frames remaining for current note */
static UINT8  music_on;        /* 1 = playing, 0 = muted/stopped */
static UINT16 music_cur_freq;  /* last triggered freq (avoid retrigger) */

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                  */
/* ------------------------------------------------------------------ */

static void set_track_pointers(UINT8 track) {
    switch (track) {
        case TRACK_CHURCH:
            trk_freq = track1_freq;
            trk_dur  = track1_dur;
            trk_len  = TRACK1_LEN;
            break;
        case TRACK_OUTDOORS:
            trk_freq = track2_freq;
            trk_dur  = track2_dur;
            trk_len  = TRACK2_LEN;
            break;
        case TRACK_READER:
            trk_freq = track3_freq;
            trk_dur  = track3_dur;
            trk_len  = TRACK3_LEN;
            break;
        default: /* TRACK_MENU */
            trk_freq = track0_freq;
            trk_dur  = track0_dur;
            trk_len  = TRACK0_LEN;
            break;
    }
}

static void play_ch2_note(UINT16 freq) {
    if (freq == N_REST) {
        NR22_REG = 0x00;          /* volume 0 */
        NR24_REG = 0x80;          /* trigger to apply silence */
        return;
    }
    if (cur_track == TRACK_READER) {
        NR21_REG = 0xC0;          /* 75 % duty — warm, organ-like tone */
        NR22_REG = 0x30;          /* volume 3, sustain (very quiet) */
    } else {
        NR21_REG = 0x80;          /* 50 % duty, no length */
        NR22_REG = 0x60;          /* volume 6, sustain (no envelope) */
    }
    NR23_REG = (UINT8)(freq & 0xFFu);
    NR24_REG = (UINT8)(0x80u | ((freq >> 8) & 0x07u));
}

static void start_bass_drone(UINT8 track) {
    UINT16 root;
    switch (track) {
        case TRACK_CHURCH:   root = N_C4; break;  /* C3 — warm neutral */
        case TRACK_OUTDOORS: root = N_D4; break;  /* D3 — solemn */
        case TRACK_READER:   root = N_E4; break;  /* E3 — Phrygian root (ison) */
        default:             root = N_E4; break;  /* E3 — bright, joyful */
    }
    NR30_REG = 0x80;              /* enable wave channel */
    NR31_REG = 0x00;              /* no length limit */
    NR32_REG = 0x60;              /* 25 % output level (subtle) */
    NR33_REG = (UINT8)(root & 0xFFu);
    NR34_REG = (UINT8)(0x80u | ((root >> 8) & 0x07u));
}

static void stop_bass_drone(void) {
    NR30_REG = 0x00;              /* disable wave channel */
}

/* ------------------------------------------------------------------ */
/*  Public API — init / update / music control                        */
/* ------------------------------------------------------------------ */

void sound_init(void) BANKED {
    UINT8 i;

    NR52_REG = 0x80;  /* master sound ON */
    NR50_REG = 0x55;  /* volume 5/7 on both L+R */
    NR51_REG = 0xFF;  /* all channels to both speakers */

    /* Load wave RAM (Ch3 must be off while writing) */
    NR30_REG = 0x00;
    for (i = 0; i < 16u; i++) {
        AUD3WAVE[i] = wave_triangle[i];
    }

    cur_track = TRACK_MENU;
    set_track_pointers(cur_track);

    music_pos      = 0;
    music_timer    = 0;
    music_on       = 0;
    music_cur_freq = 0;
}

void sound_update(void) BANKED {
    UINT16 freq;

    if (!music_on) return;

    if (music_timer > 0) {
        music_timer--;
        return;
    }

    /* Time to advance to next note */
    freq = trk_freq[music_pos];

    if (freq != music_cur_freq) {
        play_ch2_note(freq);
        music_cur_freq = freq;
    }

    music_timer = trk_dur[music_pos];
    music_pos++;
    if (music_pos >= trk_len) {
        music_pos = 0;
    }

    /* Decrement once so this frame counts */
    if (music_timer > 0) music_timer--;
}

void music_set_track(UINT8 track) BANKED {
    if (track == cur_track && music_on) return;  /* already playing this */

    /* Silence current melody + drone for a clean gap */
    if (music_on) {
        NR22_REG = 0x00;
        NR24_REG = 0x80;
        stop_bass_drone();
    }

    cur_track = track;
    set_track_pointers(track);
    music_pos = 0;
    music_timer = 8;  /* 8-frame silence gap before new track starts */
    music_cur_freq = 0;

    /* Reader gets quieter master volume for gentle reading ambience */
    if (track == TRACK_READER) {
        NR50_REG = 0x22;  /* master volume 2/7 — very soft */
    } else {
        NR50_REG = 0x55;  /* master volume 5/7 — normal */
    }

    if (music_on) {
        start_bass_drone(track);
    }
}

void music_start(void) BANKED {
    music_pos      = 0;
    music_timer    = 0;
    music_on       = 1;
    music_cur_freq = 0;
    start_bass_drone(cur_track);
}

void music_stop(void) BANKED {
    music_on = 0;
    /* silence melody channel */
    NR22_REG = 0x00;
    NR24_REG = 0x80;
    stop_bass_drone();
    music_cur_freq = 0;
}

UINT8 music_toggle(void) BANKED {
    if (music_on) {
        music_stop();
        return 0;
    }
    music_start();
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Sound effects — all use Ch1 (square + sweep) or Ch4 (noise)       */
/* ------------------------------------------------------------------ */

void sfx_confirm(void) BANKED {
    NR10_REG = 0x00;                     /* no sweep */
    NR11_REG = 0x80;                     /* 50 % duty */
    NR12_REG = 0x83;                     /* vol 8, env down pace 3 */
    NR13_REG = (UINT8)(N_C5 & 0xFFu);   /* C5 — sits above melody range */
    NR14_REG = (UINT8)(0x80u | ((N_C5 >> 8) & 0x07u));
}

void sfx_cancel(void) BANKED {
    NR10_REG = 0x00;
    NR11_REG = 0xC0;                     /* 75 % duty (darker tone) */
    NR12_REG = 0x74;                     /* vol 7, env down pace 4 */
    NR13_REG = (UINT8)(N_C4 & 0xFFu);
    NR14_REG = (UINT8)(0x80u | ((N_C4 >> 8) & 0x07u));
}

void sfx_start(void) BANKED {
    NR10_REG = 0x24;                     /* sweep: pace 2, up, shift 4 */
    NR11_REG = 0x80;                     /* 50 % duty */
    NR12_REG = 0xA2;                     /* vol 10, env down pace 2 */
    NR13_REG = (UINT8)(N_G4 & 0xFFu);
    NR14_REG = (UINT8)(0x80u | ((N_G4 >> 8) & 0x07u));
}

void sfx_card_flip(void) BANKED {
    NR10_REG = 0x00;                     /* no sweep */
    NR11_REG = 0x40;                     /* 25 % duty (thin / bright) */
    NR12_REG = 0xA2;                     /* vol 10, env down pace 2 */
    NR13_REG = (UINT8)(N_G4 & 0xFFu);
    NR14_REG = (UINT8)(0x80u | ((N_G4 >> 8) & 0x07u));
}

void sfx_match(void) BANKED {
    /* ascending sweep gives a "found it!" feel */
    NR10_REG = 0x15;                     /* sweep: pace 1, up, shift 5 */
    NR11_REG = 0x80;                     /* 50 % duty */
    NR12_REG = 0xD3;                     /* vol 13, env down pace 3 */
    NR13_REG = (UINT8)(N_C4 & 0xFFu);
    NR14_REG = (UINT8)(0x80u | ((N_C4 >> 8) & 0x07u));
}

void sfx_mismatch(void) BANKED {
    /* descending sweep = "nope" */
    NR10_REG = 0x1D;                     /* sweep: pace 1, down, shift 5 */
    NR11_REG = 0xC0;                     /* 75 % duty (fuller tone) */
    NR12_REG = 0xB4;                     /* vol 11, env down pace 4 */
    NR13_REG = (UINT8)(N_A4 & 0xFFu);
    NR14_REG = (UINT8)(0x80u | ((N_A4 >> 8) & 0x07u));
}

void sfx_bell(void) BANKED {
    /* Church bell GONG: deep, resonant, long-sustaining low-pitched strike.
       Uses E2 (~82 Hz) for a true bass gong feel — much lower and longer
       than all other SFX.  freq_val 458 = 2048 - 131072/82.41 */

    /* Ch4 (noise): deep metallic strike impact */
    NR41_REG = 0x00;                     /* no length limit (continuous) */
    NR42_REG = 0xF4;                     /* vol 15, env down pace 4 (slow fade) */
    NR43_REG = 0x61;                     /* shift 6, 7-bit (tonal), div 1 — deep metal */
    NR44_REG = 0x80;                     /* trigger, continuous */

    /* Ch1 (square): low E2 resonant hum */
    NR10_REG = 0x00;                     /* no sweep — clean sustain */
    NR11_REG = 0x80;                     /* 50 % duty — rich, full tone */
    NR12_REG = 0xF7;                     /* vol 15, env down pace 7 (longest ring ~4s) */
    NR13_REG = 0xCA;                     /* E2 low byte (458 = 0x01CA) */
    NR14_REG = 0x81;                     /* trigger + E2 high bit */
}
