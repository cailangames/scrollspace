#include <gb/gb.h>
#include <gbdk/font.h>
#include <rand.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "player.h"
#include "title_screens.h"
#include "player_py.h"
#include "block_tiles_py.h"

#define KEY_PRESSED(K) (current_input & (K))
#define FONT_OFFSET 36
#define MAPBLOCK_IDX  FONT_OFFSET
#define SCREEN_T  16
#define SCREEN_B 152
#define SCREEN_L 8
#define SCREEN_R 160

void wait(uint8_t n){
  uint8_t i;
  for (i=0; i < n; i++){
    wait_vbl_done();
  }
}

uint8_t update_obstacle_max_width(uint8_t new_gap_w_min){
  uint8_t obstacle_max = new_gap_w_min / 3;
  if (obstacle_max == 0){
    obstacle_max = 1;
  }

  return obstacle_max;
}

int8_t new_gap_row_idx_offset(void){
  // initrand(DIV_REG);
  uint8_t n = rand();
  int8_t idx_offset;
  
  if (n < 24) {
    idx_offset = 0;
  }
  else if (n < 140){
    idx_offset = 1;
  }
  else {
    idx_offset = -1;
  }

  return idx_offset;
}

int8_t new_w_offset(void){
  // initrand(DIV_REG);
  uint8_t n = rand();
  int8_t w_offset;
  
  if (n < 24) {
    w_offset = 1;
  }
  else if (n < 140){
    w_offset = -1;
  }
  else {
    w_offset = 0;
  }

  return w_offset;
}

uint8_t new_obstacle_idx(uint8_t gap_w, uint8_t gap_row_idx){
  // initrand(DIV_REG);
  // uint8_t n = rand() / 14;
  uint16_t n = (gap_w * rand())/255 + gap_row_idx;

  return (uint8_t) n;
}

void generate_new_column(uint8_t *col_idx, uint8_t *gap_row_idx, 
                         uint8_t *gap_w, uint8_t *new_column, 
                         uint8_t gap_w_min, uint8_t obs_w_max,
                         uint8_t *coll_map){
  // initrand(DIV_REG);
  uint8_t n;  // random number
  uint8_t i;  // Loop counter
  int8_t tmp_gap_row_idx;
  int8_t idx_offset = 0;
  int8_t w_offset = 0;
  
  // Generate a random number to determine if we are going to 
  // update both idx and w, or just one of them
  n = rand();

  if (n < 24) {
    idx_offset = new_gap_row_idx_offset();
    w_offset = new_w_offset();
  }
  else if (n < 140) {
    idx_offset = new_gap_row_idx_offset();
  }
  else {
    w_offset = new_w_offset();
  }
  
  // Update the gap_row_idx and the gap_w
  tmp_gap_row_idx = *gap_row_idx + idx_offset;
  *gap_w = *gap_w + w_offset;

  // Bound checking
  if (tmp_gap_row_idx < 1){
    *gap_row_idx = 1; // leave 1 row at top
  }
  else if (tmp_gap_row_idx > (16 - gap_w_min)) {
    *gap_row_idx = (16 - gap_w_min);  // leave 1 row at bottom
  }
  else {
    *gap_row_idx = (uint8_t) tmp_gap_row_idx;
  }

  if (*gap_w < gap_w_min){
    *gap_w = gap_w_min;
  }
  else if (*gap_w > 18) {
    *gap_w = 18;
  }
  obs_w_max = update_obstacle_max_width(*gap_w);

  // Generate new column
  for (i=0; i<18;i++){
    if ((i >= *gap_row_idx) && (i < *gap_row_idx+*gap_w)){
      new_column[i] = 0;
      coll_map[(*col_idx)*18 + i] = 0;
    }
    else{
      new_column[i] = MAPBLOCK_IDX;
      coll_map[(*col_idx)*18 + i] = 1;
    }
  }

  // Get another random number to see if we should add
  // Obstacles in the gap
  n = rand();
  if (n < 96){
    uint8_t idx = new_obstacle_idx(*gap_w, *gap_row_idx);
    if ((idx > *gap_row_idx) && (idx < *gap_row_idx + *gap_w)){
      for (i=0; i<obs_w_max; i++){
        new_column[idx+i] = MAPBLOCK_IDX; // + 1;
        coll_map[(*col_idx)*18 + idx + i] = 2;
      }
    }
  }

  // Write new column to tile map
  set_bkg_tiles(*col_idx, 0, 1, 18, new_column);

  // Increment col_idx
  *col_idx = *col_idx + 1;
  if (*col_idx > 31){
    *col_idx = 0;
  }
}

bool check_collisions(struct Player player, uint8_t *coll_map){
  /*
   * The player sprite can collide with up to 4 tiles.
   * Check the collision map on the top_left, top_right
   * bottom_left, and bottom right corners.
   */

  uint16_t cm_idx_topl, cm_idx_topr; 
  uint16_t cm_idx_botl, cm_idx_botr; 
  uint8_t row_topl, row_topr;
  uint8_t row_botl, row_botr;
  uint8_t col_topl, col_topr;
  uint8_t col_botl, col_botr;

  row_topl = (player.cb.y - 16) / 8;
  col_topl = (SCX_REG + player.cb.x - 8) / 8;
  cm_idx_topl = col_topl*18 + row_topl;

  row_topr = (player.cb.y - 16) / 8;
  col_topr = (SCX_REG + player.cb.x + player.cb.w - 8) / 8;
  cm_idx_topr = col_topr*18 + row_topr;

  row_botl = (player.cb.y + player.cb.h - 16) / 8;
  col_botl = (SCX_REG + player.cb.x - 8) / 8;
  cm_idx_botl = col_botl*18 + row_botl;

  row_botr = (player.cb.y + player.cb.h - 16) / 8;
  col_botr = (SCX_REG + player.cb.x + player.cb.w - 8) / 8;
  cm_idx_botr = col_botr*18 + row_botr;

  if ((coll_map[cm_idx_topl] > 0) || (coll_map[cm_idx_topr] > 0) || \
      (coll_map[cm_idx_botl] > 0) || (coll_map[cm_idx_botr] > 0)){
    return true;
  }
  else{
    return false;
  }
}

void main(void){
  /*
   * Load background and sprite data
   */

  // Load font tiles to background map
  font_t min_font;
  font_init();
  min_font = font_load(font_min);
  font_set(min_font);

  // Load background tiles
  set_bkg_data(FONT_OFFSET,1,block_tiles);

  // Load sprite data
  set_sprite_data(0,3,player_data);

  // Load title screen
  set_bkg_tiles(0,0,20,18,game_titlescreen);

  /*
   * Create a player and display the sprite 
   */
  struct Player player;
  player.sprite_id = 0;
  player.sprite_tile_id = 0;
  player.x = 20;
  player.y = 72 + 8;
  player.speed = 1;
  player.dir = LEFT;
  player.cb_x_offset = 1;
  player.cb_y_offset = 2;
  player.cb.x = player.x + player.cb_x_offset;
  player.cb.y = player.y + player.cb_y_offset;
  player.cb.w = 5;
  player.cb.h = 4;
  move_sprite(player.sprite_id, player.x, player.y);

  /*
   * Turn on display and show sprites and background 
   */
  DISPLAY_ON;
  SHOW_SPRITES;
  SHOW_BKG;

  /*
   * Initialize map by filling in the remaining tiles in the 
   * 32x32 memory space
   */
  uint8_t gap_w_min = 4;    // Minimum gap width
  uint8_t gap_w_start = 8;
  uint8_t obs_w_max = update_obstacle_max_width(gap_w_min);  // Maximum width of obstacles
  uint8_t gap_row_idx = 4;   // Start of gap
  uint8_t gap_w = gap_w_start; // Gap width
  uint8_t col_idx = 20;   // First column to populate
 
  // Collision map. The first 18 elements correspond to the first col in background tile map
  uint8_t *coll_map = calloc(18*32, sizeof(uint8_t));
  uint8_t new_column[18];  // Placeholder for new column array
  for (int i=0; i<18;i++){
    if ((i >= gap_row_idx) & (i < gap_row_idx+gap_w)){
      new_column[i] = 0;
      coll_map[col_idx*18 + i] = 0;
    }
    else{
      new_column[i] = MAPBLOCK_IDX;
      coll_map[col_idx*18 + i] = 1;
    }
  }
  set_bkg_tiles(col_idx, 0, 1, 18, new_column);

  /*
   * Game Looop
   */
  uint8_t current_input;
  uint8_t dx = 0;
  uint8_t dy = 0;
  uint8_t frame_count = 0;
  uint8_t scroll_count = 0;
  uint8_t damage_reecovery_count = 0; // When hit, skip checking collisions for this long
  uint8_t scroll_thresh = 0x3; // Scroll when this is set
  uint8_t col_count = 0;  // counter for number of columns scrolled. Used to calculate screen_count
  uint16_t screen_count = 0; // number of screens scrolled
  bool damage_hidden = false; // Used during the damage recovery to toggle between showing and hidding the player sprite

  waitpad(J_START);
  waitpadup();
  initrand(DIV_REG);
  for (int i=0; i<12; i++){
    generate_new_column(&col_idx, &gap_row_idx, &gap_w, new_column, gap_w_min, obs_w_max, coll_map);
  }

  while(1) {
    current_input = joypad();

    // D-PAD

    if (!KEY_PRESSED(J_UP) && !KEY_PRESSED(J_DOWN) && \
        !KEY_PRESSED(J_LEFT) && !KEY_PRESSED(J_RIGHT)) 
    {
      dx = 0;
      dy = 0;
      player.sprite_tile_id = 0;
      player.cb_x_offset = 1;
      player.cb_y_offset = 2;
      player.cb.h = 4;
    }
    else {
      // At least one key pressed
      if (KEY_PRESSED(J_RIGHT)){
        dx = player.speed;
        // dy = 0;  // Keep in case i want to disable diagonal movement
        player.sprite_tile_id = 0;
      }
      if (KEY_PRESSED(J_LEFT)){
        dx = -player.speed;
        // dy = 0;  // Keep in case i want to disable diagonal movement
        player.sprite_tile_id = 0;
      }
      if (KEY_PRESSED(J_UP)){
        // dx = 0;  // Keep in case i want to disable diagonal movement
        dy = -player.speed;
        player.sprite_tile_id = 1;
        
        // Make collision box smaller when plane is "tilted"
        // 3 wide x 1 high
        player.cb_x_offset = 2;
        player.cb_y_offset = 3;
        player.cb.h = 1;
        player.cb.w = 3;
      }
      if (KEY_PRESSED(J_DOWN)){
        // dx = 0;  // Keep in case i want to disable diagonal movement
        dy = player.speed;
        player.sprite_tile_id = 2;

        // Make collision box smaller when plane is "tilted"
        // 5 wide x 1 high
        player.cb_x_offset = 2;
        player.cb_y_offset = 4;
        player.cb.h = 1;
        player.cb.w = 3;
      }
    }
    

    // Update player position
    // Bound check
    uint8_t tmpx = player.x + dx; 
    if (tmpx <= SCREEN_L){
      player.x = SCREEN_L;
      // dx = (player.x - SCREEN_L);
    }
    else if (tmpx >= SCREEN_R){
      player.x = SCREEN_R;
      // dx = (SCREEN_R - player.x);
    }
    else{
      player.x += dx;
    }
    
    uint8_t tmpy = player.y + dy; 
    if (tmpy <= SCREEN_T){
      player.y = SCREEN_T;
      // dy = (player.y - SCREEN_T);
    }
    else if (tmpy >= SCREEN_B){
      player.y = SCREEN_B;
      // dy = (SCREEN_B - player.y);
    }
    else{
      player.y += dy;
    }

    // Update collision box
    player.cb.x = player.x + player.cb_x_offset;
    player.cb.y = player.y + player.cb_y_offset;

    set_sprite_tile(player.sprite_id, player.sprite_tile_id);
    move_sprite(player.sprite_id, player.x, player.y);

    if (damage_reecovery_count == 0){
      if (damage_hidden){
        SHOW_SPRITES;
        damage_hidden = false;
      }
      if (check_collisions(player, coll_map)){
        damage_reecovery_count = 16;
      }
    }
    else{
      damage_reecovery_count--;
      if (damage_hidden){
        SHOW_SPRITES;
        damage_hidden = false;
      }
      else{
        HIDE_SPRITES;
        damage_hidden = true;
      }
    }

    if ((frame_count & scroll_thresh)){
      scroll_bkg(1,0);
      scroll_count++;
    }

    if (scroll_count == 8){
      scroll_count = 0;
      generate_new_column(&col_idx, &gap_row_idx, &gap_w, new_column, gap_w_min, obs_w_max, coll_map);
      col_count++;
      if (col_count > 20){
        col_count = 0;
        screen_count++;
      }
    }
    frame_count++;

    if (frame_count >= 255){
      frame_count = 0;
    }

    if (screen_count == (scroll_thresh*20)){
      screen_count = 0;
      scroll_thresh = (scroll_thresh << 1) + 1;
    }

    // Wait for frame to finish drawing
    wait_vbl_done();
  }
}
