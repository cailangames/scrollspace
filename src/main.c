#include <gb/gb.h>
#include <gbdk/font.h>
#include <rand.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gbdk/bcd.h>

#include "hUGEDriver.h"
#include "common.h"
#include "procedural_generation.h"

#include "sprites.h"
#include "title_screens.h"
#include "player_sprites.h"
#include "player_shield_sprites.h"
#include "block_tiles.h"
#include "progressbar_tiles.h"
#include "projectiles_sprites.h"
#include "powerups_tiles.h"

void play_gun_sound(void){
  // Stop Channel before playing FX
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);

  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x4D;
  NR11_REG = 0XC1;
  NR12_REG = 0XF2;
  NR13_REG = 0X9B;
  NR14_REG = 0X87;

  // Restart channel
  hUGE_mute_channel(HT_CH1, HT_CH_PLAY);

}

void play_health_sound(void){
  // Stop Channel before playing FX
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);

  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x75;
  NR11_REG = 0X86;
  NR12_REG = 0X5F;
  NR13_REG = 0X62;
  NR14_REG = 0X86;

  // Restart channel
  hUGE_mute_channel(HT_CH1, HT_CH_PLAY);

}

void play_bomb_sound(void){
  // Stop Channel before playing FX
  hUGE_mute_channel(HT_CH4, HT_CH_MUTE);

  NR42_REG = 0x0;
  NR44_REG = 0x0;

  // Play FX
  NR41_REG = 0X00;
  NR42_REG = 0XF7;
  NR43_REG = 0X71;
  NR44_REG = 0X80;

  // Restart channel
  hUGE_mute_channel(HT_CH4, HT_CH_PLAY);

}

void wait(uint8_t n){
  uint8_t i;
  for (i=0; i < n; i++){
    wait_vbl_done();
  }
}

void fadeout(void){
  uint8_t i;

  for (i=0; i<4; i++){
    switch (i)
    {
    case 0:
      BGP_REG = 0xE4;
      break;

    case 1:
      BGP_REG = 0xF9;
      break;
    
    case 2:
      BGP_REG = 0xFE;
      break;
    
    case 3:
      BGP_REG = 0xFF;
      break;
    }
    wait(10);
  }
}

void fadein(void){
  uint8_t i;

  for (i=0; i<3; i++){
    switch (i)
    {
    case 0:
      BGP_REG = 0xFE;
      break;

    case 1:
      BGP_REG = 0xF9;
      break;
    
    case 2:
      BGP_REG = 0xE4;
      break;
    }
    wait(10);
  }
}

void score2tile(uint32_t score, uint8_t* score_tiles){
  uint8_t len = bcd2text(&score, 1, score_tiles);
  set_win_tiles(10, 1, len, 1, score_tiles);
  
  // uint8_t digit, i;  
  // uint8_t len = 10;

  // // For now, only support scores from 0-999
  // if (score < 1000){
  //   i = 0;
  //   digit = (score / 1000000000);
  //   score -= digit*1000000000;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = (score / 100000000);
  //   score -= digit*100000000;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = (score / 10000000);
  //   score -= digit*10000000;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = (score / 1000000);
  //   score -= digit*1000000;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = (score / 100000);
  //   score -= digit*100000;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = (score / 10000);
  //   score -= digit*10000;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = (score / 1000);
  //   score -= digit*1000;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = score / 100;
  //   score -= digit*100;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = score / 10;
  //   score -= digit*10;
  //   score_tiles[i] = digit + 0x01;

  //   i++;
  //   digit = score;
  //   score_tiles[i] = digit + 0x01;

  // }
  // else {
  //   score_tiles[0] = 0x01;
  //   score_tiles[1] = 0x01;
  //   score_tiles[2] = 0x01;
  //   score_tiles[3] = 0x01;
  // }
  // set_win_tiles(8, 1, len, 1, score_tiles);
}



void drop_bomb(struct Sprite *player, uint8_t *coll_map, uint8_t *bkg_map, uint8_t radius){
  /*
   * Grab the corners of the player and move radius tiles away and replace 
   * the coll_map and bkg_map entries with 0.
   * The bomb is dropped in front of the player
   */
  uint16_t cm_idx; 
  uint16_t bkg_idx; 
  uint16_t row_top, row_bot;
  uint16_t col_left, col_right;

  row_top = (player->y - 16)/8;
  row_bot = row_top + 1;
  col_left = ((SCX_REG + player->x - 8) % 256)/8;
  col_left += radius+1; // drop in front of player 
  col_right = col_left;

  if (radius <= row_top){
    row_top -= radius;
  }
  else {
    row_top = 0;
  }

  if (COLUMN_HEIGHT >= row_bot + radius){
    row_bot += radius;
  }
  else {
    row_bot = COLUMN_HEIGHT;
  }

  if (radius <= col_left){
    col_left -= radius;
  }
  else {
    col_left = 0;
  }

  if (32 >= col_right + radius){
    col_right += radius;
  }
  else {
    col_right = 32;
  }

  uint16_t row, col;
  for (row=row_top; row < row_bot; row++){
    for (col=col_left; col < col_right; col++){
      cm_idx = col*COLUMN_HEIGHT + row;
      bkg_idx = col + row*32;

      coll_map[cm_idx] = 0;
      bkg_map[bkg_idx] = CRATERBLOCK_IDX; // 0;  // Index 0 is the blank tile, index 4 is the crater tile
    }
  }

  // set_bkg_tiles(0, 0, 32, COLUMN_HEIGHT, bkg_map);
}

uint8_t check_collisions(struct Sprite *sprite, uint8_t *coll_map, uint8_t *bkg_map, uint8_t player_sprite){
  /*
   * The player sprite can collide with up to 4 tiles.
   * Check the collision map on the top_left, top_right
   * bottom_left, and bottom right corners.
   */

  uint16_t cm_idx_topl, cm_idx_topr; 
  uint16_t cm_idx_botl, cm_idx_botr; 
  uint16_t bkg_idx_topl, bkg_idx_topr; 
  uint16_t bkg_idx_botl, bkg_idx_botr; 
  uint16_t row_topl, row_topr;
  uint16_t row_botl, row_botr;
  uint16_t col_topl, col_topr;
  uint16_t col_botl, col_botr;
  uint8_t collision = 0;

  row_topl = (sprite->cb.y - 16) / 8;
  col_topl = ((SCX_REG + sprite->cb.x - 8) %256) / 8;
  cm_idx_topl = col_topl*COLUMN_HEIGHT + row_topl;
  bkg_idx_topl = col_topl + row_topl*32;

  row_topr = row_topl;
  col_topr = ((SCX_REG + sprite->cb.x + sprite->cb.w - 8)%256) / 8;
  cm_idx_topr = col_topr*COLUMN_HEIGHT + row_topr;
  bkg_idx_topr = col_topr + row_topr*32;

  row_botl = ((sprite->cb.y + sprite->cb.h - 16)%256) / 8;
  col_botl = col_topl; 
  cm_idx_botl = col_botl*COLUMN_HEIGHT + row_botl;
  bkg_idx_botl = col_botl + row_botl*32;

  row_botr = row_botl;
  col_botr = col_topr; 
  cm_idx_botr = col_botr*COLUMN_HEIGHT + row_botr;
  bkg_idx_botr = col_botr + row_botr*32;

  if (coll_map[cm_idx_topl] > 0){
    collision = coll_map[cm_idx_topl];
    if (collision == 1) {
      // Hit a wall
      if (sprite->dir & UP){
        // Top Left collision detected while sprite is moving up 
        sprite->y += 8;
      }
      
      if (sprite->dir & LEFT){
        // Top Left collision detected while sprite is moving up 
        sprite->x += 8;
      }
    }
    else if (collision < 10) {
      // Hit a block
      // Replace obstacle tile and remove collision
      bkg_map[bkg_idx_topl] = 0;
      coll_map[cm_idx_topl] = 0;
    }
    else if (player_sprite) {
      bkg_map[bkg_idx_topl] = 0;
      coll_map[cm_idx_topl] = 0;
    }
  }
  else if (coll_map[cm_idx_topr] > 0) {
    collision = coll_map[cm_idx_topr]; 
    if (collision == 1) {
      // Hit a wall
      if (sprite->dir & UP){
        // Top Right collision detected while sprite is moving up 
        sprite->y += 8;
      }

      if (sprite->dir & RIGHT){
        // Top Right collision detected while sprite is moving up 
        sprite->x -= 8;
      }
    }
    else if (collision < 10) {
      // Hit an obstacle
      // Replace obstacle tile and remove collision
      bkg_map[bkg_idx_topr] = 0;
      coll_map[cm_idx_topr] = 0;
    }
    else if (player_sprite) {
      bkg_map[bkg_idx_topr] = 0;
      coll_map[cm_idx_topr] = 0;
    }
  }
  else if(coll_map[cm_idx_botl] > 0){
    collision = coll_map[cm_idx_botl];
    if (collision == 1) {
      // Hit a wall
      if (sprite->dir & DOWN){
        // Bottom Left collision detected while sprite is moving down 
        sprite->y -= 8;
      }

      if (sprite->dir & LEFT){
        // Bottom Left collision detected while sprite is moving down 
        sprite->x += 8;
      }
    }
    else if (collision < 10) {
      // Hit an obstacle
      // Replace obstacle tile and remove collision
      bkg_map[bkg_idx_botl] = 0;
      coll_map[cm_idx_botl] = 0;
    }
    else if (player_sprite) {
      bkg_map[bkg_idx_botl] = 0;
      coll_map[bkg_idx_botl] = 0;
    }
  } 
  else if (coll_map[cm_idx_botr] > 0){
    collision = coll_map[cm_idx_botr];
    if (collision == 1) {
      // Hit a wall
      if (sprite->dir & DOWN){
        // Bottom Right collision detected while sprite is moving down 
        sprite->y -= 8;
      }

      if (sprite->dir & RIGHT){
        // Bottom Right collision detected while sprite is moving down 
        sprite->x -= 8;
      }
    }
    else if (collision < 10) {
      // Hit an obstacle
      // Replace obstacle tile and remove collision
      bkg_map[bkg_idx_botr] = 0;
      coll_map[cm_idx_botr] = 0;
    }
    else if (player_sprite) {
      bkg_map[bkg_idx_botr] = 0;
      coll_map[cm_idx_botr] = 0;
    }
  }
  
  if ((collision == 1) && (player_sprite)){
    // Move the player sprite after collision
    move_sprite(sprite->sprite_id, sprite->x, sprite->y);
  }

  return collision;
}

void update_health_bar(struct Sprite *player, uint8_t *progressbar_tiles, uint8_t *player_sprite_base_id, uint8_t progressbar_tilemap_offset){
  uint8_t i, idx;
  if (player->health == 100){
    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i = 1; i < 9; i++){
      progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
    }
    progressbar_tiles[9] = progressbar_tilemap_offset + 3; // right edge of bar
  }
  else if (player->health >= 90) {
    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i = 1; i < 9; i++){
      progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
    }
    progressbar_tiles[9] = progressbar_tilemap_offset + 6; // right edge of bar

  }
  else if ((player->health < 90) && (player->health >= 10)) {
    idx = (player->health + 10) / 10;
    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i=1; i < 9; i++){
      if (i < idx){
        progressbar_tiles[i] = progressbar_tilemap_offset + 2; // fill 
      }
      else {
        progressbar_tiles[i] = progressbar_tilemap_offset + 5; // clear
      }
    }
    progressbar_tiles[9] = progressbar_tilemap_offset + 6; // clear right edge of bar
  }
  else {
    progressbar_tiles[1] = progressbar_tilemap_offset + 5; // Clear bottom 2 tiles 
    progressbar_tiles[0] = progressbar_tilemap_offset + 4;
  }
  set_win_tiles(8, 0, 10, 1, progressbar_tiles);

  if (player->health > 50){
    *player_sprite_base_id = 0;
  }
  else if (player->health > 25){
    *player_sprite_base_id = 3;
  }
  else if (player->health > 0){
    *player_sprite_base_id = 6;
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
  uint8_t blocks_tilemap_offset = FONT_OFFSET;
  uint8_t powerups_tilemap_offset = blocks_tilemap_offset + 4;
  uint8_t progressbar_tilemap_offset = powerups_tilemap_offset + 8;
  set_bkg_data(FONT_OFFSET,4,block_tiles);
  set_bkg_data(powerups_tilemap_offset, 8, powerups_tiles);
  set_bkg_data(progressbar_tilemap_offset, 7, progressbar_tiles);

  // Load sprite data
  set_sprite_data(0,10,player_data);
  set_sprite_data(10,9,player_shield_data);
  set_sprite_data(19,3,projectiles_data);

  // Load title screen
  set_bkg_tiles(0,0,20,COLUMN_HEIGHT,game_titlescreen);

  // Load Window
  move_win(7,128);

  /**
   * Load music 
   */
  extern const hUGESong_t main_song;
  // hUGE_init(&main_song);

  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x33;

  // Start both channels
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
  hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
  
  // Add hUGE driver to VBL Interrupt handler
  add_VBL(hUGE_dosound);

  /*
   * Turn on display and show background 
   */
  DISPLAY_ON;
  SHOW_BKG;

  uint8_t progressbar_tiles[10];
  uint8_t powerups_top_tiles[5];
  uint8_t powerups_bot_tiles[5];
  uint8_t score_tiles[10];

  struct Sprite player;
  /**
   * Create bullet sprite array 
   */
  struct Sprite bullets[MAX_BULLETS];
  uint8_t bullets_arr_idx;
  uint8_t bullets_active;
  bool bullets_fired;
  bool bomb_dropped;
  bool shield_active;
  enum powerup active_powerup;
  uint8_t n_bullets;
  uint8_t n_bombs;
  uint8_t n_shields;
  uint8_t n_health;

  uint8_t gap_w_min;    // Minimum gap width
  uint8_t gap_w_start;
  uint8_t obs_w_max;  // Maximum width of obstacles
  uint8_t gap_row_idx;   // Start of gap
  uint8_t gap_w; // Gap width

  // Collision map. The first 18 elements correspond to the first col in background tile map
  uint8_t col_idx;   // First column to populate
  uint8_t *coll_map = calloc(COLUMN_HEIGHT*32, sizeof(uint8_t));  // 32x16
  uint8_t *bkg_map = calloc(COLUMN_HEIGHT*32, sizeof(uint8_t));  // 16x32
  uint8_t new_column[COLUMN_HEIGHT];  // Placeholder for new column array
  bool copy_bkgmap_to_vram; 

  uint8_t current_input;
  uint8_t old_input;
  uint8_t dx;
  uint8_t dy;
  uint8_t frame_count;
  uint8_t scroll_count;
  int16_t damage_animation_counter; // When hit, skip checking collisions for this long
  uint8_t scroll_thresh; // Scroll when this is set
  uint8_t col_count;  // counter for number of columns scrolled. Used to calculate screen_count
  uint16_t screen_count; // number of screens scrolled
  uint32_t score; 
  uint8_t player_collision;
  uint8_t bullet_collision;
  enum animation_state damage_animation_state; // Used during the damage recovery to toggle between showing and hidding the player sprite

  uint8_t player_sprite_base_id;
  uint8_t bullet_sprite_base_id;

  // Bullet sprites and pointers
  struct Sprite b;
  struct Sprite *b_ptr;

  // Temporary variables (loop counters, array indices, etc)
  uint8_t tmpx; 
  uint8_t tmpy; 
  uint8_t i, idx;
  uint16_t ii;

  // Banking variables
  uint8_t last_bank;

  while (1){
    // Load the window contents
    bullets_arr_idx = 0;
    n_bullets = MAX_BULLETS;
    n_bombs = MAX_BOMBS;
    n_shields = MAX_SHIELDS;
    n_health = MAX_HEALTH;

    powerups_top_tiles[0] = powerups_tilemap_offset + 4;  // Deselected ammo
    powerups_top_tiles[1] = n_bullets + 1;
    powerups_top_tiles[2] = 0;
    powerups_top_tiles[3] = powerups_tilemap_offset + 3;  // Deselected health kit
    powerups_top_tiles[4] = n_health + 1;

    powerups_bot_tiles[0] = powerups_tilemap_offset + 1;  // Deselected bomb
    powerups_bot_tiles[1] = n_bombs + 1;
    powerups_bot_tiles[2] = 0;
    powerups_bot_tiles[3] = powerups_tilemap_offset + 2;  // Deselected shield
    powerups_bot_tiles[4] = n_shields + 1;

    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i = 1; i < 9; i++){
      progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
    }
    progressbar_tiles[9] = progressbar_tilemap_offset + 3; // right edge of bar

    score = 0;
    score2tile(score, score_tiles);
    set_win_tiles(2, 0, 5, 1, powerups_top_tiles);
    set_win_tiles(2, 1, 5, 1, powerups_bot_tiles);
    set_win_tiles(8, 0, 10, 1, progressbar_tiles);

    /*
    * Create a player and display the sprite 
    */
    player.sprite_id = 0;
    player.sprite_tile_id = 0;
    player.x = 20;
    player.y = 72 + 8;
    player.speed = 1;
    player.dir = RIGHT;
    player.cb_x_offset = 1;
    player.cb_y_offset = 2;
    player.cb.x = player.x + player.cb_x_offset;
    player.cb.y = player.y + player.cb_y_offset;
    player.cb.w = 5;
    player.cb.h = 4;
    player.health = 100;
    player.active = true;
    player.type = PLAYER;
    move_sprite(player.sprite_id, player.x, player.y);

    /**
     * Create bullet sprite array 
     */
    bullets_arr_idx = 0;
    bullets_active = 0;
    bullets_fired = false;
    bomb_dropped = false;
    shield_active = false;
    active_powerup = GUN;

    /*
    * Initialize map by filling in the remaining tiles in the 
    * 32x32 memory space
    */
    gap_w_min = 6;    // Minimum gap width
    gap_w_start = 10;
    obs_w_max = update_obstacle_max_width(gap_w_min);  // Maximum width of obstacles
    gap_row_idx = 4;   // Start of gap
    gap_w = gap_w_start; // Gap width
    col_idx = 20;   // First column to populate
  
    // Clear collision map and background map
    for (ii=0; ii<32*COLUMN_HEIGHT;ii++){
      coll_map[ii] = 0;
      bkg_map[ii] = 0;
    }

    // Collision map. The first 18 elements correspond to the first col in background tile map
    for (i=0; i<COLUMN_HEIGHT;i++){
      if ((i >= gap_row_idx) & (i < gap_row_idx+gap_w)){
        new_column[i] = 0;
        bkg_map[col_idx + i*32] = 0;
        coll_map[col_idx*COLUMN_HEIGHT + i] = 0;
      }
      else{
        new_column[i] = MAPBLOCK_IDX;
        bkg_map[col_idx + i*32] = MAPBLOCK_IDX;
        coll_map[col_idx*COLUMN_HEIGHT + i] = 1;
      }
    }
    set_bkg_tiles(col_idx, 0, 1, COLUMN_HEIGHT, new_column);

    /*
    * Game Looop
    */
    current_input;
    old_input=0;
    dx = 0;
    dy = 0;
    frame_count = 0;
    scroll_count = 0;
    damage_animation_counter = 0; // When hit, skip checking collisions for this long
    scroll_thresh = 0x3; // Scroll when this is set
    col_count = 0;  // counter for number of columns scrolled. Used to calculate screen_count
    screen_count = 0; // number of screens scrolled
    score = 0; 
    player_collision = 0;
    bullet_collision = 0;
    damage_animation_state = HIDDEN;

    waitpad(J_START);
    waitpadup();
    hUGE_init(&main_song);
    hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
    hUGE_mute_channel(HT_CH2, HT_CH_PLAY);

    SHOW_SPRITES;
    SHOW_WIN;
    initrand(DIV_REG);

    last_bank = CURRENT_BANK;
    SWITCH_ROM(1);
    for (int i=0; i<11; i++){
      generate_new_column(&col_idx, &gap_row_idx, &gap_w, new_column, gap_w_min, obs_w_max, coll_map, bkg_map);
      set_bkg_tiles(col_idx, 0, 1, COLUMN_HEIGHT, new_column);
    }
    SWITCH_ROM(last_bank);

    player_sprite_base_id = 0;
    bullet_sprite_base_id = 19;

    copy_bkgmap_to_vram = false;

    while(1) {
      current_input = joypad();
      player_collision = 0;
      bullet_collision = 0;
      copy_bkgmap_to_vram = false;

      // D-PAD
      if (KEY_PRESSED(J_START)){
        hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
        hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
        waitpadup();
        waitpad(J_START);
        waitpadup();
        hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
        hUGE_mute_channel(HT_CH2, HT_CH_PLAY);
        damage_animation_state = SHOWN;
      }

      if (!KEY_PRESSED(J_UP) && !KEY_PRESSED(J_DOWN) && \
          !KEY_PRESSED(J_LEFT) && !KEY_PRESSED(J_RIGHT)) 
      {
        dx = 0;
        dy = 0;
        player.sprite_tile_id = player_sprite_base_id;
        player.cb_x_offset = 1;
        player.cb_y_offset = 2;
        player.cb.h = 4;
        player.dir = RIGHT;
      }
      else {
        // At least one key pressed
        if (KEY_PRESSED(J_RIGHT)){
          dx = player.speed;
          // dy = 0;  // Keep in case i want to disable diagonal movement
          player.sprite_tile_id = player_sprite_base_id;
          player.dir |= RIGHT;
        }
        if (KEY_PRESSED(J_LEFT)){
          dx = -player.speed;
          // dy = 0;  // Keep in case i want to disable diagonal movement
          player.sprite_tile_id = player_sprite_base_id;
          player.dir |= LEFT;
        }
        if (KEY_PRESSED(J_UP)){
          // dx = 0;  // Keep in case i want to disable diagonal movement
          dy = -player.speed;
          player.sprite_tile_id = player_sprite_base_id + 1;
          player.dir |= UP;
          
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
          player.sprite_tile_id = player_sprite_base_id + 2;
          player.dir |= DOWN;

          // Make collision box smaller when plane is "tilted"
          // 5 wide x 1 high
          player.cb_x_offset = 2;
          player.cb_y_offset = 4;
          player.cb.h = 1;
          player.cb.w = 3;
        }
      }

      if (KEY_FIRST_PRESS(J_A)){
        switch (active_powerup)
        {
          case GUN:
          {
            if (n_bullets > 0)
            {
              bullets_fired = true;
              n_bullets--;

              b.type = BULLET;
              b.sprite_id = bullets_arr_idx + 1; // +1 so we dont override the player (always sprite_id 0)
              b.speed = 4;
              b.x = player.x;
              b.y = player.y;

              // Collision box
              b.cb.x = b.x;
              b.cb.y = b.y;
              b.cb_x_offset = 0;
              b.cb_y_offset = 2;
              b.cb.h = 4;
              b.cb.w = 8;
              b.active = true;
              b.lifespan = 20;

              b.sprite_tile_id = bullet_sprite_base_id;
              set_sprite_tile(b.sprite_id, b.sprite_tile_id);
              move_sprite(b.sprite_id, b.x, b.y);

              *(bullets+bullets_arr_idx) = b;

              bullets_arr_idx++;
              bullets_active++;

              // Check if index exceeded the array size and reset to 0
              if (bullets_arr_idx >= MAX_BULLETS){
                bullets_arr_idx = 0;
              }
              play_gun_sound();
            }
          }
          break;
          
          case BOMB:
          {
            if (n_bombs > 0){
              bomb_dropped = true;
              n_bombs--;
            }
            play_bomb_sound();
          }
          break;

          case SHIELD:
          {
            if ((n_shields > 0) && (!shield_active)){
              shield_active = true;
              damage_animation_counter = 8*20;
              player_sprite_base_id += 10;
              n_shields--;
            }
          }
          break;

          case HEALTH:
          {
            if (n_health > 0){
              player.health += 10;
              if (player.health > 100){
                player.health = 100;
              }
              n_health--;
              update_health_bar(&player, progressbar_tiles, &player_sprite_base_id, progressbar_tilemap_offset);
              play_health_sound();
            }
          }
          break;
        }
      }
      if (KEY_FIRST_PRESS(J_B)){
        switch (active_powerup)
        {
          case GUN:
          {
            if (n_bombs > 0){
              // Deselect current tile
              powerups_top_tiles[0] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = BOMB;
              powerups_bot_tiles[0] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_shields > 0){
              // Deselect current tile
              powerups_top_tiles[0] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = SHIELD;
              powerups_bot_tiles[3] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_health > 0){
              // Deselect current tile
              powerups_top_tiles[0] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = HEALTH;
              powerups_top_tiles[3] = powerups_tilemap_offset + 4 + active_powerup;
            }
          }
          break;
          
          case BOMB:
          {
            if (n_shields > 0){
              // Deselect current tile
              powerups_bot_tiles[0] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = SHIELD;
              powerups_bot_tiles[3] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_health > 0){
              // Deselect current tile
              powerups_bot_tiles[0] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = HEALTH;
              powerups_top_tiles[3] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_bullets > 0){
              // Deselect current tile
              powerups_bot_tiles[0] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = GUN;
              powerups_top_tiles[0] = powerups_tilemap_offset + 4 + active_powerup;
            }
          }
          break;

          case SHIELD:
          {
            if (n_health > 0){
              // Deselect current tile
              powerups_bot_tiles[3] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = HEALTH;
              powerups_top_tiles[3] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_bullets > 0){
              // Deselect current tile
              powerups_bot_tiles[3] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = GUN;
              powerups_top_tiles[0] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_bombs > 0){
              // Deselect current tile
              powerups_bot_tiles[3] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = BOMB;
              powerups_bot_tiles[0] = powerups_tilemap_offset + 4 + active_powerup;
            }
          }
          break;

          case HEALTH:
          {
            if (n_bullets > 0){
              // Deselect current tile
              powerups_top_tiles[3] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = GUN;
              powerups_top_tiles[0] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_bombs > 0){
              // Deselect current tile
              powerups_top_tiles[3] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = BOMB;
              powerups_bot_tiles[0] = powerups_tilemap_offset + 4 + active_powerup;
            }
            else if (n_shields > 0){
              // Deselect current tile
              powerups_top_tiles[3] = powerups_tilemap_offset + active_powerup; 

              // Change active powerup 
              active_powerup = SHIELD;
              powerups_bot_tiles[3] = powerups_tilemap_offset + 4 + active_powerup;
            }
          }
          break;
        }
        set_win_tiles(2, 0, 5, 1, powerups_top_tiles);
        set_win_tiles(2, 1, 5, 1, powerups_bot_tiles);
      }

      old_input = current_input;

      // Update player position
      // Bound check
      tmpx = player.x + dx; 
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
      
      tmpy = player.y + dy; 
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
      
      /**
       * Update bullets, if fired
       */
      if (bullets_fired){
        // Update bullet positions
        b_ptr = bullets;
        for (i=0; i < MAX_BULLETS; i++){
          // If current bullet is not active, move to the next one
          if (!b_ptr->active){
            b_ptr++;
            continue;
          }

          b_ptr->x += b_ptr->speed;
          b_ptr->lifespan--;

          // Update collision box
          b_ptr->cb.x = b_ptr->x + b_ptr->cb_x_offset;
          b_ptr->cb.y = b_ptr->y + b_ptr->cb_y_offset;

          bullet_collision = check_collisions(b_ptr, coll_map, bkg_map, false);
          // Check that the bullet collided with something it can destroy
          if (bullet_collision == 2) {
            // Hit a block
            score += 2;
            // Force to 1 so the next || works
            bullet_collision = 1;
          }
          else if (bullet_collision > 2) {
            // Collided with something it can't destroy.
            // Force to 0 so the next || works
            bullet_collision = 0;
          }

          if ((b_ptr->x > SCREEN_R) || \
              bullet_collision || \
              (b_ptr->lifespan == 0))
          {
           // Hide sprite
            b_ptr->x = 0;
            b_ptr->y = 0;
            b_ptr->speed = 0;
            b_ptr->active = false;
            bullets_active--;
          }
          
          move_sprite(b_ptr->sprite_id, b_ptr->x, b_ptr->y);
          b_ptr++;
        }
        
        if (bullets_active == 0) 
        {
          // No active bullets
          bullets_fired = false;
        }
      }
      if (bomb_dropped){
        drop_bomb(&player, coll_map, bkg_map, 3);
        copy_bkgmap_to_vram = true;
        bomb_dropped = false;
      }

      /*
       * Continue processing 
       */

      if (damage_animation_counter == 0){
        // Damage animation or shield powerup expired.
        // Reset so that player is shown
        if (shield_active){
          shield_active = false;
          player_sprite_base_id -= 10;
        }

        if (damage_animation_state == HIDDEN){
          // SHOW_SPRITES;
          move_sprite(player.sprite_id, player.x, player.y);
          damage_animation_state = SHOWN;
        }

        player_collision = check_collisions(&player, coll_map, bkg_map, true);
        if (player_collision == 1) {
          player.health -= 10;
          damage_animation_counter = 16;
        }
        else if (player_collision == 2) {
          player.health -= 5;
          damage_animation_counter = 16;
          player_collision = 1;
        }
        else if (player_collision == 11) {
          player.health += 10;
          player_collision = 1;
          play_health_sound();
        }
        else if (player_collision == 10) {
          shield_active = true;
          damage_animation_counter = 8*20;
          player_sprite_base_id += 10;
          player_collision = 0;
        }

        if (player.health <= 0){
          // End game in the future
          // For now, reset to full health
          hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
          hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
          player.sprite_tile_id = 9;
          set_sprite_tile(player.sprite_id, player.sprite_tile_id);
          wait(10);
          HIDE_BKG;
          fadeout();
          HIDE_SPRITES;
          HIDE_WIN;
          // Load title screen
          set_bkg_tiles(0,0,20,COLUMN_HEIGHT,game_titlescreen);
          move_bkg(0,0);
          player.sprite_tile_id = player_sprite_base_id;
          set_sprite_tile(player.sprite_id, player.sprite_tile_id);
          SHOW_BKG;
          wait(60);
          fadein();
          break;
        }

        // Update health bar after a collision
        if (player_collision){
          update_health_bar(&player, progressbar_tiles, &player_sprite_base_id, progressbar_tilemap_offset);
        }
      }
      else {
        damage_animation_counter--;
        if (damage_animation_counter < 0){
          damage_animation_counter = 0;
        }
        if (!shield_active){
          if (damage_animation_state == HIDDEN){
            // SHOW_SPRITES;
            move_sprite(player.sprite_id, player.x, player.y);
            damage_animation_state = SHOWN;
          }
          else{
            // HIDE_SPRITES;
            move_sprite(player.sprite_id, 0, 0);
            damage_animation_state = HIDDEN;
          }
        }
      }

      if ((frame_count & scroll_thresh)){
        scroll_bkg(1,0);
        scroll_count++;

        if (scroll_count == 8){
          scroll_count = 0;
          last_bank = CURRENT_BANK;
          SWITCH_ROM(1);
          generate_new_column(&col_idx, &gap_row_idx, &gap_w, new_column, gap_w_min, obs_w_max, coll_map, bkg_map);
          SWITCH_ROM(last_bank);
          copy_bkgmap_to_vram = true;

          col_count++;
          if (col_count == 20){
            col_count = 0;
            screen_count++;

            // Increment score at full screen
            score += 5;

            // score2tile(score, score_tiles);

            // if (screen_count == (scroll_thresh*20)){
            if (screen_count == 20){
              screen_count = 0;
              // Increase speed every 20 screens
              // This logic is incorrect. FIX
              // scroll_thresh = (scroll_thresh << 1) + 1;

              // Add one to every powerup
              if (n_bullets < MAX_BULLETS){
                n_bullets++;
              }
              if (n_bombs < MAX_BOMBS){
                n_bombs++;
              }
              if (n_shields < MAX_SHIELDS){
                n_shields++;
              }
              if (n_health < MAX_HEALTH){
                n_health++;
              }
            }
            else if (screen_count == 5){
              // Add a bullet every 5 screens
              if (n_bullets < MAX_BULLETS){
                n_bullets++;
              }
            }
            else if (screen_count == 10){
              // Add a bomb every 10 screens
              if (n_bombs < MAX_BOMBS){
                n_bombs++;
              }
            }
          }
          else if (col_count == 10){
            // Increment score at half screen
            score += 2;
          }
        }
      }
      else if ((frame_count & 0x3) == 0) { // %4
        // Update HUD
        if (n_bullets > 0){
          powerups_top_tiles[1] = n_bullets + 1;  
        }
        else{
          powerups_top_tiles[1] = 1;  
        }

        if (n_health > 0){
          powerups_top_tiles[4] = n_health + 1;  
        }
        else{
          powerups_top_tiles[4] = 1;  
        }
        set_win_tiles(2, 0, 5, 1, powerups_top_tiles);

        if (n_bombs > 0){
          powerups_bot_tiles[1] = n_bombs + 1;  
        }
        else{
          powerups_bot_tiles[1] = 1;  
        }

        if (n_shields > 0){
          powerups_bot_tiles[4] = n_shields + 1;  
        }
        else{
          powerups_bot_tiles[4] = 1;  
        }
        set_win_tiles(2, 1, 5, 1, powerups_bot_tiles);
      }
      
      frame_count++;
      if (frame_count >= 255){
        frame_count = 0;
      }

      if ((player_collision) || (bullet_collision)){
        // Update bkg_map if there are collisions
        copy_bkgmap_to_vram = true;
      }

      // Wait for frame to finish drawing
      vsync();
      
      score2tile(score, score_tiles);
      if (copy_bkgmap_to_vram){
        // Write the entire map to VRAM
        set_bkg_tiles(0, 0, 32, COLUMN_HEIGHT, bkg_map);
      }
    }
  }
}
