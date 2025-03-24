// Code and values that are widely used across files

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

#include "sprites.h"

// The following are used for performance testing of individual components.
#define ENABLE_SCORING 1
#define ENABLE_MUSIC 1
#define ENABLE_WEAPONS 1
#define ENABLE_COLLISIONS 1
#define ENABLE_INTRO 1

/*
 * Keys
 */
#define KEY_PRESSED(input, key) ((input) & (key))
#define KEY_HELD(input, old_input, key) (((input) & (key)) && ((old_input) & (key)))
#define KEY_FIRST_PRESS(input, old_input, key) (((input) & (key)) && !((old_input) & (key)))

/*
 * Screen
 */
// The top of the *visible* screen is 16 pixels from the actual top of the screen.
#define SCREEN_T 16
#define SCREEN_B 144
// The left side of the *visible* screen is 8 pixels from the actual left side of the screen.
#define SCREEN_L 8
#define SCREEN_R 160
#define SCREEN_TILE_WIDTH 20
#define SCREEN_TILE_HEIGHT 18
#define SCREEN_SCROLL_WIDTH (SCREEN_TILE_WIDTH + 2)
#define ROW_WIDTH 32
#define COLUMN_HEIGHT 17
#define PIXELS_PER_COLUMN 8
#define COLUMNS_PER_SCREEN 20

/*
 * Tiles
 */
#define TILE_SIZE_PIXELS 8
#define TILE_DATA_SIZE_BYTES 16
#define TILE_COUNT(arr) (sizeof(arr) / TILE_DATA_SIZE_BYTES)
// Bit-shift by 5 is equivalent to multiply by 32, which is the ROW_WIDTH.
#define MAP_INDEX(row, column) ((((uint16_t)(row)) << 5) + (column))
#define MAP_INDEX_ROW_OFFSET(row) (((uint16_t)(row)) << 5)
// Like `MAP_INDEX()`, but only for the size of one screen (20x18 tiles).
#define SCREEN_MAP_INDEX(row, column) ((((uint16_t)(row)) * SCREEN_TILE_WIDTH) + (column))
// Note: Indexes 0x01-0x24 are used for font characters.
// clang-format off
#define EMPTY_TILE        0x00
#define WALL_BLOCK_TILE   0x25
#define MINE_TILE         0x27
#define CRATER_TILE       0x28
#define BOMB_READY_ICON   0x2A
#define SHIELD_TILE       0x2B
#define HEALTH_KIT_TILE   0x2C
#define HEALTH_BAR_START  0x33
#define LOCK_TILE         0x46
// clang-format on

#define INTRO_SCENE_OFFSET 0x80
#define INTRO_SCENE_STARS_OFFSET 0x8C
#define TITLE_SCREEN_OFFSET 0xA4

/*
 * Sprites
 */
#define PLAYER_SPRITE_ID 0
#define PLAYER_SHIELD_SPRITES_OFFSET 10
#define DEATH_SPRITE 9
#define BULLET_SPRITE 19

/*
 * Banks
 */
#define RAM_BANK0_ADDRESS 0xA000

/*
 * Tunable parameters
 */
#define MAX_BULLETS 3
#define BLOCK_HEALTH 2
#define MINE_HEALTH 1
#define POWERUP_RESERVED_IDS 235
#define SHIELD_ID 254
#define SHIELD_DURATION 160
#define HEALTH_KIT_ID 255
#define HEALTH_KIT_VALUE 5
// When the player is damaged enough, health kits provide more health to the player.
#define HEALTH_KIT_DAMAGED_VALUE (2 * HEALTH_KIT_VALUE)
#define HEALTH_KIT_CRITICALLY_DAMAGED_VALUE (4 * HEALTH_KIT_VALUE)
#define COLLISION_DAMAGE 5
#define IFRAMES_DURATION 60
#define IFRAMES_ANIMATION_CYCLE 5
// A BOMB_RADIUS of N will create square bomb explosions of (2*N+1, 2*N+1) size in tiles. The +1 is
// for the center row, which is centered on the ship.
#define BOMB_RADIUS 3
#define BOMB_LENGTH (BOMB_RADIUS * 2 + 1)
#define BOMB_COOLDOWN_FRAMES 420
#define BULLET_LIFESPAN 25
#define BULLET_DAMAGE 1
#define BULLET_COLLISION_X_OFFSET 0
#define BULLET_COLLISION_Y_OFFSET 2
#define PLAYER_START_X 0x1400
#define PLAYER_START_Y 0x5000
#define PLAYER_MAX_HEALTH 100
// The below damage thresholds affect health pickup effectiveness and the ship's sprite tile.
#define PLAYER_DAMAGED_THRESHOLD 50
#define PLAYER_CRITICALLY_DAMAGED_THRESHOLD 15
#define PLAYER_COLLISION_DAMAGE 2
#define SHIELD_COLLISION_DAMAGE 5
#define PLAYER_KNOCKBACK_SPEED 0x0400
// Important: PLAYER_KNOCKBACK_DURATION should be less than IFRAMES_DURATION.
#define PLAYER_KNOCKBACK_DURATION 2
// The speeds are 16-bit fixed point numbers, where the high 8 bits are the pixels
// (per frame) and the low 8 bits are the subpixels.
// clang-format off
#define PLAYER_SPEED_NORMAL    0x0100
#define PLAYER_SPEED_HARD      0x0140
#define PLAYER_SPEED_TURBO     0x0180
#define PLAYER_SPEED_TURBO_MAX 0x01C0
#define PLAYER_SPEED_INCREASE  0x0010
#define BULLET_SPEED           0x0400
#define SCROLL_SPEED_NORMAL    0x0100
#define SCROLL_SPEED_HARD      0x0200
#define SCROLL_SPEED_TURBO     0x0300
#define SCROLL_SPEED_TURBO_MAX 0x0400
#define SCROLL_SPEED_INCREASE  0x0040
// clang-format on
// Increase the difficulty after this many screens have been scrolled.
#define DIFFICULTY_INCREASE_SCREEN_COUNT 10
#define POINTS_PER_MINE 2
#define POINTS_PER_PICKUP 2
#define POINTS_PER_SCREEN_SCROLLED 5
// IMPORTANT: If you update these thresholds, you must update the unlock messages in score.h too.
#define HARD_MODE_UNLOCK_POINTS 2000
#define TURBO_MODE_UNLOCK_POINTS 1000
#define UPGRADE_SPRITE_UNLOCK_POINTS 500
// The below probabilities are out of 65,535 (uint16_t max).
// clang-format off
#define HEALTH_PICKUP_PROBABILITY  100
#define SHIELD_PICKUP_PROBABILITY  100
#define MINE_PROBABILITY_NARROW    2000
#define MINE_PROBABILITY_WIDE_OPEN 8000
// clang-format on

/*
 * Font
 */
#define CHAR_SPACE 0x00
#define CHAR_0 0x01
#define CHAR_1 0x02
#define CHAR_2 0x03
#define CHAR_3 0x04
#define CHAR_4 0x05
#define CHAR_5 0x06
#define CHAR_6 0x07
#define CHAR_7 0x08
#define CHAR_8 0x09
#define CHAR_9 0x0A
#define CHAR_A 0x0B
#define CHAR_B 0x0C
#define CHAR_C 0x0D
#define CHAR_D 0x0E
#define CHAR_E 0x0F
#define CHAR_F 0x10
#define CHAR_G 0x11
#define CHAR_H 0x12
#define CHAR_I 0x13
#define CHAR_J 0x14
#define CHAR_K 0x15
#define CHAR_L 0x16
#define CHAR_M 0x17
#define CHAR_N 0x18
#define CHAR_O 0x19
#define CHAR_P 0x1A
#define CHAR_Q 0x1B
#define CHAR_R 0x1C
#define CHAR_S 0x1D
#define CHAR_T 0x1E
#define CHAR_U 0x1F
#define CHAR_V 0x20
#define CHAR_W 0x21
#define CHAR_X 0x22
#define CHAR_Y 0x23
#define CHAR_Z 0x24
#define CHAR_COLON 0x39
#define CHAR_CURSOR 0x3A
#define CHAR_EXCLAMATION_MARK 0x3B
#define CHAR_COMMA 0x3C
#define CHAR_QUESTION_MARK 0x3D

/*
 * Math
 */
#define MOD2(n) ((n) & 0x1)
#define MOD4(n) ((n) & 0x3)
#define MOD8(n) ((n) & 0x7)
#define MOD16(n) ((n) & 0xF)
#define MOD32(n) ((n) & 0x1F)
#define MOD64(n) ((n) & 0x3F)

/*
 * Arrays
 */
#define UINT8_ARRARY_SIZE(arr) (sizeof(arr) / sizeof(uint8_t))

/*
 * Game modes
 */
enum GameMode {
  NORMAL = 0,
  HARD = 1,
  TURBO = 2,
  CLEAR_DATA = 3,
  TITLE_SCREEN = 4,
};

/*
 * Global variables
 */
extern struct Sprite player_sprite;
extern uint8_t collision_map[COLUMN_HEIGHT * ROW_WIDTH];
extern uint8_t background_map[COLUMN_HEIGHT * ROW_WIDTH];
// How many pixels per frame to scroll the screen (high 8 bits: pixels, low 8 bits: subpixels)
extern fixed scroll_speed;
extern uint16_t point_score;
extern enum GameMode game_mode;

#endif
