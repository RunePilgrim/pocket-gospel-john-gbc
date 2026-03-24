/* match_tiles.h — tile definitions for the memory matching game. */

#ifndef MATCH_TILES_H
#define MATCH_TILES_H

#include <gb/gb.h>

#define MATCH_TILE_BASE    98u  /* shares space with map tiles (swapped in) */
#define MATCH_TILE_COUNT   44u

/* Card back (2x2) */
#define MT_BACK_TL         0u
#define MT_BACK_TR         1u
#define MT_BACK_BL         2u
#define MT_BACK_BR         3u

/* Symbol 0: Cross (2x2) */
#define MT_CROSS_TL        4u
#define MT_CROSS_TR        5u
#define MT_CROSS_BL        6u
#define MT_CROSS_BR        7u

/* Symbol 1: Dove (2x2) */
#define MT_DOVE_TL         8u
#define MT_DOVE_TR         9u
#define MT_DOVE_BL         10u
#define MT_DOVE_BR         11u

/* Symbol 2: Fish / Ichthys (2x2) */
#define MT_FISH_TL         12u
#define MT_FISH_TR         13u
#define MT_FISH_BL         14u
#define MT_FISH_BR         15u

/* Symbol 3: Chalice / Cup (2x2) */
#define MT_CUP_TL          16u
#define MT_CUP_TR          17u
#define MT_CUP_BL          18u
#define MT_CUP_BR          19u

/* Symbol 4: Lamb (2x2) */
#define MT_LAMB_TL         20u
#define MT_LAMB_TR         21u
#define MT_LAMB_BL         22u
#define MT_LAMB_BR         23u

/* Symbol 5: Crown of thorns (2x2) */
#define MT_CROWN_TL        24u
#define MT_CROWN_TR        25u
#define MT_CROWN_BL        26u
#define MT_CROWN_BR        27u

/* Symbol 6: Bread / Loaf (2x2) */
#define MT_BREAD_TL        28u
#define MT_BREAD_TR        29u
#define MT_BREAD_BL        30u
#define MT_BREAD_BR        31u

/* Symbol 7: Flame / Fire (2x2) */
#define MT_FLAME_TL        32u
#define MT_FLAME_TR        33u
#define MT_FLAME_BL        34u
#define MT_FLAME_BR        35u

/* UI tiles */
#define MT_CARD_BORDER_H   36u  /* horizontal card border */
#define MT_CARD_BORDER_V   37u  /* vertical card border */
#define MT_CURSOR_TL       38u  /* cursor top-left bracket */
#define MT_CURSOR_TR       39u  /* cursor top-right bracket */
#define MT_CURSOR_BL       40u  /* cursor bottom-left bracket */
#define MT_CURSOR_BR       41u  /* cursor bottom-right bracket */
#define MT_MATCHED         42u  /* matched marker */
#define MT_EMPTY           43u  /* empty/background */

/* Number of unique symbols */
#define MATCH_SYMBOL_COUNT 8u

extern const UINT8 match_tiles_data[];

#define MATCH_TILES_BANK 13u

#endif
