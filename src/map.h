/*
 * map.h — map system for tile-based rooms with collision and objects.
 * Maps are 20x18 tiles (single screen, no scrolling).
 * Banked to bank 15.
 */

#ifndef MAP_MODULE_H
#define MAP_MODULE_H

#include <gb/gb.h>

#define MAP_W       20u
#define MAP_ROWS    18u

/* Map IDs */
#define MAP_CHURCH   0u
#define MAP_OUTDOORS 1u

/* Interactive object types */
#define OBJ_NONE        0u
#define OBJ_CANDLE      1u
#define OBJ_ALTAR_BOOK  2u
#define OBJ_BELL_ROPE   3u
#define OBJ_CARD_TABLE  4u
#define OBJ_DOOR_OUT    5u   /* door leading outside */
#define OBJ_DOOR_IN     6u   /* door leading inside church */

#define MAX_OBJECTS    12u

/* Interactive object on a map */
typedef struct MapObject {
    UINT8 x;       /* tile x position */
    UINT8 y;       /* tile y position */
    UINT8 type;    /* OBJ_* constant */
    UINT8 state;   /* object-specific state (e.g., candle lit/unlit) */
} MapObject;

/* Map descriptor */
typedef struct MapData {
    const UINT8 *tiles;          /* 20x18 tile index array (offsets from tileset base) */
    const UINT8 *collision;      /* 54-byte bitfield (3 per row): 1=solid, 0=walkable */
    const UINT8 *pal_attrs;      /* 20x18 CGB palette attribute array (or NULL for all pal 0) */
    UINT8 tile_base;             /* VRAM tile base index for this tileset */
    UINT8 obj_count;             /* number of interactive objects */
    MapObject objects[MAX_OBJECTS];
    UINT8 spawn_x;              /* default player spawn x (tile) */
    UINT8 spawn_y;              /* default player spawn y (tile) */
} MapData;

/* Load and render a map */
void map_load(UINT8 map_id) BANKED;

/* Draw the current map tiles to BG */
void map_draw(void) BANKED;

/* Check if a tile is solid (1=solid, 0=walkable) */
UINT8 map_is_solid(UINT8 tx, UINT8 ty) BANKED;

/* Get the object at a tile position (returns NULL if none) */
MapObject *map_get_object(UINT8 tx, UINT8 ty) BANKED;

/* Get the current map data */
MapData *map_get_current(void) BANKED;

/* Get current map ID */
UINT8 map_get_current_id(void) BANKED;

/* Update a single tile on the map (e.g., candle lit/unlit swap) */
void map_set_tile(UINT8 tx, UINT8 ty, UINT8 tile_offset) BANKED;

/* Sky scroll: update scroll offset based on player position and input.
   Call once per frame from game_update for outdoor map. */
void map_update_scroll(UINT8 player_ty, UBYTE held) BANKED;

/* Get current sky scroll offset (0=normal, >0=sky visible) */
UINT8 map_get_scroll_y(void) BANKED;

#endif
