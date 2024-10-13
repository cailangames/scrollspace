#ifndef _COMMON_H_
#define _COMMON_H_

#define KEY_PRESSED(K) (current_input & K)
#define KEY_HELD(K) ((current_input & K) && (old_input & K))
#define KEY_FIRST_PRESS(K) ((current_input & K) && !(old_input & K))
#define FONT_OFFSET 37
// The top of the *visible* screen is 16 pixels from the actual top of the screen.
#define SCREEN_T  16
#define SCREEN_B 144
// The left side of the *visible* screen is 8 pixels from the actual left side of the screen.
#define SCREEN_L 8
#define SCREEN_R 160
#define SCREEN_TILE_WIDTH 20
#define ROW_WIDTH 32
#define COLUMN_HEIGHT 17
#define BULLET_ARR_SIZE 9
// MAX_BULLETS can't exceed BULLET_ARR_SIZE.
#define MAX_BULLETS 5
#define MAX_BOMBS 1

// Bit-shift by 5 is equivalent to multiply by 32, which is the ROW_WIDTH.
#define MAP_ARRAY_INDEX_ROW_OFFSET(ROW_IDX) ((ROW_IDX) << 5)
#define EMPTY_TILE_IDX 0
#define MAPBLOCK_IDX FONT_OFFSET
#define MINE_IDX (MAPBLOCK_IDX+2)
#define CRATERBLOCK_IDX (MAPBLOCK_IDX+3)
#define SHIELD_TILE (MAPBLOCK_IDX+6)
#define HEALTH_KIT_TILE (MAPBLOCK_IDX+7)

#define BLOCK_HEALTH 4
#define MINE_HEALTH 2
#define SHIELD_ID 254
#define SHIELD_DURATION 8*20
#define HEALTH_KIT_ID 255
#define HEALTH_KIT_VALUE 5
#define COLLISION_DAMAGE 3
#define COLLISION_TIMEOUT 24
// A BOMB_RADIUS of N will create square bomb explosions of (2*N+1, 2*N+1) size in tiles.
#define BOMB_RADIUS 2

enum powerup{
  GUN=0,
  BOMB=1,
  SHIELD=2,
  HEALTH=3
};

enum animation_state{
  HIDDEN=0,
  SHOWN=1
};

void fadein(void);
void fadeout(void);
void wait(uint8_t);

#endif