#ifndef _COMMON_H_
#define _COMMON_H_

#define KEY_PRESSED(K) (current_input & K)
#define KEY_HELD(K) ((current_input & K) && (old_input & K))
#define KEY_FIRST_PRESS(K) ((current_input & K) && !(old_input & K))
#define FONT_OFFSET 36
#define MAPBLOCK_IDX  FONT_OFFSET
#define CRATERBLOCK_IDX MAPBLOCK_IDX+3
#define SCREEN_T  16
#define SCREEN_B 144
#define SCREEN_L 8
#define SCREEN_R 160
#define COLUMN_HEIGHT 16
#define MAX_BULLETS 2
#define MAX_BOMBS 1

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

extern uint8_t update_obstacle_max_width(uint8_t new_gap_w_min) BANKED;

extern int8_t new_gap_row_idx_offset(void) BANKED;

extern int8_t new_w_offset(void) BANKED;

extern uint8_t new_obstacle_idx(uint8_t gap_w, uint8_t gap_row_idx) BANKED;

extern void generate_new_column(uint8_t *col_idx, uint8_t *gap_row_idx, 
                         uint8_t *gap_w, uint8_t *new_column, 
                         uint8_t gap_w_min, uint8_t obs_w_max,
                         uint8_t *coll_map, uint8_t *bkg_map) BANKED;

#endif