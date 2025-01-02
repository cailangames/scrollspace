// Code and values that are widely used across files.

#ifndef _COMMON_H_
#define _COMMON_H_

/*
 * Keys
 */
#define KEY_PRESSED(input, key) (input & key)
#define KEY_HELD(input, old_input, key) ((input & key) && (old_input & key))
#define KEY_FIRST_PRESS(input, old_input, key) ((input & key) && !(old_input & key))

/*
 * Screen
 */
// The top of the *visible* screen is 16 pixels from the actual top of the screen.
#define SCREEN_T  16
#define SCREEN_B 144
// The left side of the *visible* screen is 8 pixels from the actual left side of the screen.
#define SCREEN_L 8
#define SCREEN_R 160
#define SCREEN_TILE_WIDTH 20
#define ROW_WIDTH 32
#define COLUMN_HEIGHT 17

/*
 * Tiles
 */
#define TILE_SIZE_BYTES 16
// Bit-shift by 5 is equivalent to multiply by 32, which is the ROW_WIDTH.
#define MAP_ARRAY_INDEX_ROW_OFFSET(ROW_IDX) ((ROW_IDX) << 5)
#define EMPTY_TILE_IDX 0
// Indexes 1-36 are used for fonts.
#define MAPBLOCK_IDX 37
#define MINE_IDX (MAPBLOCK_IDX+2)
#define CRATERBLOCK_IDX (MAPBLOCK_IDX+3)
#define BOMB_ICON_IDX (MAPBLOCK_IDX+5)
#define SHIELD_TILE (MAPBLOCK_IDX+6)
#define HEALTH_KIT_TILE (MAPBLOCK_IDX+7)
#define BOMB_SIL_ICON_IDX (MAPBLOCK_IDX+12)
#define HEALTH_BAR_START (MAPBLOCK_IDX+14)
#define HEALTH_BAR_MIDDLE (MAPBLOCK_IDX+15)
#define HEALTH_BAR_END (MAPBLOCK_IDX+16)
#define COLON_FONT_IDX 57

/*
 * Sprites
 */
#define PLAYER_SPRITE_ID 0
#define DEATH_SPRITE 9
#define CURSOR_SPRITE_ID 22

/*
 * Tunable parameters
 */
#define MAX_BULLETS 3
#define MAX_BOMBS 1
#define BLOCK_HEALTH 4
#define MINE_HEALTH 2
#define SHIELD_ID 254
#define SHIELD_DURATION 8*20
#define HEALTH_KIT_ID 255
#define HEALTH_KIT_VALUE 5
#define COLLISION_DAMAGE 3
#define COLLISION_TIMEOUT 24
// A BOMB_RADIUS of N will create square bomb explosions of (2*N+1, 2*N+1) size in tiles.
#define BOMB_RADIUS 3
#define BULLET_LIFESPAN 20
// All speeds are in pixels per frame.
#define SCROLL_SPEED_NORMAL 1
#define SCROLL_SPEED_HARD 2
#define SCROLL_SPEED_TURBO 3

enum animation_state{
  HIDDEN=0,
  SHOWN=1
};

#endif
