#ifndef _COMMON_H_
#define _COMMON_H_

#define KEY_PRESSED(K) (current_input & K)
#define KEY_HELD(K) ((current_input & K) && (old_input & K))
#define KEY_FIRST_PRESS(K) ((current_input & K) && !(old_input & K))
#define FONT_OFFSET 37
#define SCREEN_T  16
#define SCREEN_B 144
#define SCREEN_L 8
#define SCREEN_R 160
#define SCREEN_TILE_WIDTH 20
#define ROW_WIDTH 32
#define COLUMN_HEIGHT 17
#define BULLET_ARR_SIZE 9
#define MAX_BULLETS 5 // Cannot exceed BULLET_ARR_SIZE
#define MAX_BOMBS 1

#define EMPTY_TILE_IDX 0
#define MAPBLOCK_IDX  FONT_OFFSET
#define MINE_IDX MAPBLOCK_IDX+2
#define CRATERBLOCK_IDX MAPBLOCK_IDX+3
#define SHIELD_TILE MAPBLOCK_IDX+6
#define HEALTH_KIT_TILE MAPBLOCK_IDX+7

#define BLOCK_HEALTH 4
#define MINE_HEALTH 2
#define SHIELD_ID 254
#define SHIELD_DURATION 8*20
#define HELATH_KIT_ID 255
#define HEALTH_KIT_VALUE 5
#define COLLISION_DAMAGE 3
#define COLLISION_TIMEOUT 24

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