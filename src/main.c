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

#include "font_extras_tiles.h"
#include "sprites.h"
#include "title_screens.h"
#include "tutorial_screen_tiles.h"
#include "tutorial_screen_map.h"
#include "player_sprites.h"
#include "player_shield_sprites.h"
#include "block_tiles.h"
#include "progressbar_tiles.h"
#include "projectiles_sprites.h"
#include "powerups_tiles.h"

// Score variables
uint8_t isr_frame_counter;
uint8_t playtime_seconds;
uint8_t playtime_minutes;
uint8_t playtime_hours;
uint8_t game_paused;
uint16_t score;

void increment_score_isr(void){
  if (game_paused){
    return;
  }

  isr_frame_counter++;

  if (isr_frame_counter == 60){
    playtime_seconds++;
    isr_frame_counter = 0;

    if (playtime_seconds == 60){
      playtime_minutes++;
      playtime_seconds = 0;

      if (playtime_minutes == 60){
        playtime_hours++;
        playtime_minutes = 0;
      }
    }
  }
}

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

void play_gameover_sound(void){
  // Stop Channel before playing FX
  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play FX
  NR10_REG = 0x1C;
  NR11_REG = 0X89;
  NR12_REG = 0XF7;
  NR13_REG = 0X75;
  NR14_REG = 0X86;
}

void play_shield_sound(void){
  // Stop Channel before playing FX
  hUGE_mute_channel(HT_CH4, HT_CH_MUTE);

  NR42_REG = 0x0;
  NR44_REG = 0x0;

  // Play FX
  NR41_REG = 0X3F;
  NR42_REG = 0X74;
  NR43_REG = 0X2C;
  NR44_REG = 0XC0;

  // Restart channel
  hUGE_mute_channel(HT_CH4, HT_CH_PLAY);

}

void wait(uint8_t n){
  uint8_t i;
  for (i=0; i < n; i++){
    wait_vbl_done();
  }
}

void gameover_screen(uint8_t* score_time_tiles, uint8_t colon_offset, uint16_t *high_score_ptr){
  uint8_t last_playtime_seconds;
  uint8_t last_playtime_minutes;
  uint8_t last_playtime_hours;
  uint16_t last_score;
  uint8_t new_highscore;

  uint8_t tenk;
  uint8_t onek;
  uint8_t oneh;
  uint8_t tens;
  uint8_t single;

  set_bkg_tiles(0,0,20,18,gameover_titlescreen);
  move_bkg(0,0);

  // Last Best Score
  last_score = high_score_ptr[0];
  last_playtime_hours = high_score_ptr[1] >> 8;
  last_playtime_minutes = high_score_ptr[1] & 0xFF;
  last_playtime_seconds = high_score_ptr[2] >> 8;

  // Current Score
  // Seconds
  tens = playtime_seconds/10;
  score_time_tiles[7] = (playtime_seconds - 10*tens) + 0x01;
  score_time_tiles[6] = tens + 0x01; 
  score_time_tiles[5] = colon_offset;
  
  // Minutes
  tens = playtime_minutes/10;
  score_time_tiles[4] = (playtime_minutes - 10*tens) + 0x01;
  score_time_tiles[3] = tens + 0x01; 
  
  // Hours
  tens = playtime_hours/10;
  score_time_tiles[2] = colon_offset;
  score_time_tiles[1] = (playtime_hours - 10*tens) + 0x01;
  score_time_tiles[0] = tens + 0x01; 

  set_bkg_tiles(6, 5, 8, 1, score_time_tiles);

  new_highscore = 0;
  if (last_playtime_hours < playtime_hours){
    new_highscore = 1;
  }
  else if (last_playtime_minutes < playtime_minutes){
    new_highscore = 1;
  }
  else if (last_playtime_seconds < playtime_seconds){
    new_highscore = 1;
  }

  if (new_highscore == 1){
    //Write the current score as the new high score
    set_bkg_tiles(6, 10, 8, 1, score_time_tiles);

    // Update in RAM
    high_score_ptr[1] = playtime_hours << 8 | playtime_minutes;
    high_score_ptr[2] = playtime_seconds << 8;
  }
  else{
    // Write the score from RAM
    // Seconds
    tens = last_playtime_seconds/10;
    score_time_tiles[7] = (last_playtime_seconds - 10*tens) + 0x01;
    score_time_tiles[6] = tens + 0x01; 
    score_time_tiles[5] = colon_offset;
    
    // Minutes
    tens = last_playtime_minutes/10;
    score_time_tiles[4] = (last_playtime_minutes - 10*tens) + 0x01;
    score_time_tiles[3] = tens + 0x01; 
    
    // Hours
    tens = last_playtime_hours/10;
    score_time_tiles[2] = colon_offset;
    score_time_tiles[1] = (last_playtime_hours - 10*tens) + 0x01;
    score_time_tiles[0] = tens + 0x01; 

    set_bkg_tiles(6, 10, 8, 1, score_time_tiles);
  }

  tenk = score/10000;
  onek = (score - tenk*10000) / 1000;
  oneh = (score - tenk*10000 - onek*1000) / 100;
  tens = (score - tenk*10000 - onek*1000 - oneh*100) / 10;
  single = (score - tenk*10000 - onek*1000 - oneh*100 - tens*10);

  score_time_tiles[0] = 0;
  score_time_tiles[1] = 0;
  score_time_tiles[2] = 0;
  score_time_tiles[3] = tenk + 0x01;
  score_time_tiles[4] = onek + 0x1;
  score_time_tiles[5] = oneh + 0x1;
  score_time_tiles[6] = tens + 0x1;
  score_time_tiles[7] = single + 0x1;
  
  set_bkg_tiles(6, 6, 8, 1, score_time_tiles);

  if (last_score < score){
    //Write the current score as the new high score
    set_bkg_tiles(6, 11, 8, 1, score_time_tiles);

    // Update in RAM
    high_score_ptr[0] = score;
  }
  else{
    // Write the score from RAM
    tenk = last_score/10000;
    onek = (last_score - tenk*10000) / 1000;
    oneh = (last_score - tenk*10000 - onek*1000) / 100;
    tens = (last_score - tenk*10000 - onek*1000 - oneh*100) / 10;
    single = (last_score - tenk*10000 - onek*1000 - oneh*100 - tens*10);

    score_time_tiles[0] = 0;
    score_time_tiles[1] = 0;
    score_time_tiles[2] = 0;
    score_time_tiles[3] = tenk + 0x01;
    score_time_tiles[4] = onek + 0x1;
    score_time_tiles[5] = oneh + 0x1;
    score_time_tiles[6] = tens + 0x1;
    score_time_tiles[7] = single + 0x1;
    
    set_bkg_tiles(6, 11, 8, 1, score_time_tiles);

  }
  SHOW_BKG;
  fadein();
  waitpad(J_START);
  waitpadup();
  fadeout();
  HIDE_BKG;
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

void highscore2tile(uint8_t* high_score_tiles, uint16_t *high_score_ptr){
  uint8_t tenk;
  uint8_t onek;
  uint8_t oneh;
  uint8_t tens;
  uint8_t single;

  // Best Score from RAM
  score = high_score_ptr[0];
  playtime_hours = high_score_ptr[1] >> 8;
  playtime_minutes = high_score_ptr[1] & 0xFF;
  playtime_seconds = high_score_ptr[2] >> 8;

  // Write out BEST
  high_score_tiles[0] = 0xC;
  high_score_tiles[1] = 0xF;
  high_score_tiles[2] = 0x1D;
  high_score_tiles[3] = 0x1E;
  high_score_tiles[4] = 0x0;

  // Seconds
  tens = playtime_seconds/10;
  high_score_tiles[12] = (playtime_seconds - 10*tens) + 0x01;
  high_score_tiles[11] = tens + 0x01; 
  high_score_tiles[10] = 0x38;
  
  // Minutes
  tens = playtime_minutes/10;
  high_score_tiles[9] = (playtime_minutes - 10*tens) + 0x01;
  high_score_tiles[8] = tens + 0x01; 
  
  // Hours
  tens = playtime_hours/10;
  high_score_tiles[7] = 0x38;
  high_score_tiles[6] = (playtime_hours - 10*tens) + 0x01;
  high_score_tiles[5] = tens + 0x01; 

  tenk = score/10000;
  onek = (score - tenk*10000) / 1000;
  oneh = (score - tenk*10000 - onek*1000) / 100;
  tens = (score - tenk*10000 - onek*1000 - oneh*100) / 10;
  single = (score - tenk*10000 - onek*1000 - oneh*100 - tens*10);

  high_score_tiles[13] = 0;
  high_score_tiles[14] = tenk + 0x01;
  high_score_tiles[15] = onek + 0x1;
  high_score_tiles[16] = oneh + 0x1;
  high_score_tiles[17] = tens + 0x1;
  high_score_tiles[18] = single + 0x1;
  high_score_tiles[19] = 0;
  
  set_win_tiles(0, 0, 20, 1, high_score_tiles);

}

void score2tile(uint8_t* score_time_tiles, uint8_t colon_offset, uint8_t show_time){
  uint8_t tens;

  if (show_time){
    // Seconds
    tens = playtime_seconds/10;
    score_time_tiles[7] = (playtime_seconds - 10*tens) + 0x01;
    score_time_tiles[6] = tens + 0x01; 
    score_time_tiles[5] = colon_offset;
    
    // Minutes
    tens = playtime_minutes/10;
    score_time_tiles[4] = (playtime_minutes - 10*tens) + 0x01;
    score_time_tiles[3] = tens + 0x01; 
    
    // Hours
    tens = playtime_hours/10;
    score_time_tiles[2] = colon_offset;
    score_time_tiles[1] = (playtime_hours - 10*tens) + 0x01;
    score_time_tiles[0] = tens + 0x01; 
  }
  else {
    uint8_t tenk = score/10000;
    uint8_t onek = (score - tenk*10000) / 1000;
    uint8_t oneh = (score - tenk*10000 - onek*1000) / 100;
    tens = (score - tenk*10000 - onek*1000 - oneh*100) / 10;
    uint8_t single = (score - tenk*10000 - onek*1000 - oneh*100 - tens*10);

    score_time_tiles[0] = 0;
    score_time_tiles[1] = 0;
    score_time_tiles[2] = 0;
    score_time_tiles[3] = tenk + 0x01;
    score_time_tiles[4] = onek + 0x1;
    score_time_tiles[5] = oneh + 0x1;
    score_time_tiles[6] = tens + 0x1;
    score_time_tiles[7] = single + 0x1;
  }
  
  set_win_tiles(12, 0, 8, 1, score_time_tiles);
}

void drop_bomb(struct Sprite *player, uint8_t *coll_map, uint8_t *bkg_map, uint8_t radius){
  /*
   * Grab the corners of the player and move radius tiles away and replace 
   * the coll_map and bkg_map entries with 0.
   * The bomb is dropped in front of the player
   */
  uint16_t idx; 
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
      idx = col + row*32;

      if ((bkg_map[idx] == MAPBLOCK_IDX) || (bkg_map[idx] == (MAPBLOCK_IDX+2))){
        // If we are destroying a wall or a mine, increment score
        score += 1;
      }
      coll_map[idx] = 0;
      bkg_map[idx] = CRATERBLOCK_IDX; // 0;  // Index 0 is the blank tile, index 4 is the crater tile
    }
  }
}

uint8_t check_collisions(struct Sprite *sprite, uint8_t *coll_map, uint8_t *bkg_map, uint8_t player_sprite){
  /*
   * The player sprite can collide with up to 4 tiles.
   * Check the collision map on the top_left, top_right
   * bottom_left, and bottom right corners.
   */

  uint16_t idx_topl, idx_topr; 
  uint16_t idx_botl, idx_botr; 
  uint16_t row_topl, row_topr;
  uint16_t row_botl, row_botr;
  uint16_t col_topl, col_topr;
  uint16_t col_botl, col_botr;
  uint8_t collision = 0;
  uint8_t collision_block = 0;
  int8_t block_health = 0;

  row_topl = (sprite->cb.y - 16) / 8;
  col_topl = ((SCX_REG + sprite->cb.x - 8) %256) / 8;
  idx_topl = col_topl + row_topl*32;

  row_topr = row_topl;
  col_topr = ((SCX_REG + sprite->cb.x + sprite->cb.w - 8)%256) / 8;
  idx_topr = col_topr + row_topr*32;

  row_botl = ((sprite->cb.y + sprite->cb.h - 16)%256) / 8;
  col_botl = col_topl; 
  idx_botl = col_botl + row_botl*32;

  row_botr = row_botl;
  col_botr = col_topr; 
  idx_botr = col_botr + row_botr*32;

  if (coll_map[idx_topr] > 0) {
    collision = coll_map[idx_topr]; 
    collision_block = bkg_map[idx_topr];

    // Check if it is not a power up
    if (collision < 235){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_topr] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_topr] - 1;
      }

      if (block_health > 0) {
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

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_topr] = 0;
        coll_map[idx_topr] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          score += 2;
        }
      }
      else {
        coll_map[idx_topr] = block_health;
      }
    }
    else if (player_sprite) {
      // Pick up power up
      bkg_map[idx_topr] = 0;
      coll_map[idx_topr] = 0;
    }
  }
  else if (coll_map[idx_botr] > 0){
    collision = coll_map[idx_botr];
    collision_block = bkg_map[idx_botr];

    // Check if it is not a power up
    if (collision < 235){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_botl] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_botl] - 1;
      }

      if (block_health > 0) {
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

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_botr] = 0;
        coll_map[idx_botr] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          score += 2;
        }
      }
      else {
        coll_map[idx_botr] = block_health;
      }
    }
    else if (player_sprite) {
      // Pick up power up
      bkg_map[idx_botr] = 0;
      coll_map[idx_botr] = 0;
    }
  }
  else if ((player_sprite) && (coll_map[idx_topl] > 0)){
    // Only the player has left side collisions
    collision = coll_map[idx_topl];
    collision_block = bkg_map[idx_topl];

    // Check if it is not a power up
    if (collision < 235){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_topl] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_topl] - 1;
      }

      if (block_health > 0) {
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

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_topl] = 0;
        coll_map[idx_topl] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          score += 2;
        }
      }
      else {
        coll_map[idx_topl] = block_health;
      }
    }
    else if (player_sprite) {
      // Pick up power up
      bkg_map[idx_topl] = 0;
      coll_map[idx_topl] = 0;
    }
  }
  else if ((player_sprite) && (coll_map[idx_botl] > 0)){
    // Only the player has left side collisions
    collision = coll_map[idx_botl];
    collision_block = bkg_map[idx_botl];

    // Check if it is not a power up
    if (collision < 235){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_botl] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_botl] - 1;
      }

      if (block_health > 0) {
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

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_botl] = 0;
        coll_map[idx_botl] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          score += 2;
        }
      }
      else {
        coll_map[idx_botl] = block_health;
      }
    }
    else if (player_sprite) {
      // Pick up power up
      bkg_map[idx_botl] = 0;
      coll_map[idx_botl] = 0;
    }
  } 
  
  if ((block_health > 2) && (player_sprite)){
    // Move the player sprite after collision
    move_sprite(sprite->sprite_id, sprite->x, sprite->y);
  }

  return collision;
}

void update_health_bar(struct Sprite *player, uint8_t *progressbar_tiles, uint8_t *player_sprite_base_id, uint8_t progressbar_tilemap_offset){
  uint8_t i, idx;
  if (player->health == 100){
    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i = 1; i < 7; i++){
      progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
    }
    progressbar_tiles[7] = progressbar_tilemap_offset + 3; // right edge of bar
  }
  else if (player->health >= 88) {
    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i = 1; i < 7; i++){
      progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
    }
    progressbar_tiles[7] = progressbar_tilemap_offset + 6; // right edge of bar
  }
  else if (player->health >= 16) {
    idx = player->health / 12;
    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i=1; i < 7; i++){
      if (i < idx){
        progressbar_tiles[i] = progressbar_tilemap_offset + 2; // fill 
      }
      else {
        progressbar_tiles[i] = progressbar_tilemap_offset + 5; // clear
      }
    }
    progressbar_tiles[7] = progressbar_tilemap_offset + 6; // clear right edge of bar
  }
  else if (player->health > 0){
    progressbar_tiles[1] = progressbar_tilemap_offset + 5; 
    progressbar_tiles[0] = progressbar_tilemap_offset + 1;
  }
  else{
    progressbar_tiles[1] = progressbar_tilemap_offset + 5; // Clear bottom 2 tiles 
    progressbar_tiles[0] = progressbar_tilemap_offset + 4;
  }
  set_win_tiles(0, 0, 8, 1, progressbar_tiles);

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
  uint8_t tutorialscreen_tilemap_offset = progressbar_tilemap_offset + 8;
  set_bkg_data(blocks_tilemap_offset, 4, block_tiles);
  set_bkg_data(powerups_tilemap_offset, 8, powerups_tiles);
  set_bkg_data(progressbar_tilemap_offset, 7, progressbar_tiles);
  set_bkg_data(tutorialscreen_tilemap_offset, tutorial_screen_ntiles, tutorial_screen_tiles);

  // Load extra font characters
  uint8_t extra_font_offset = progressbar_tilemap_offset + 7;
  set_bkg_data(extra_font_offset,1,font_extras_tiles);

  // Load sprite data
  set_sprite_data(0,10,player_data);
  set_sprite_data(10,9,player_shield_data);
  set_sprite_data(19,3,projectiles_data);

  // Load title screen
  set_bkg_tiles(0,0,20,COLUMN_HEIGHT,game_titlescreen);

  // Load Window
  move_win(7,136);

  /**
   * Load music 
   */
  extern const hUGESong_t intro_song;
  extern const hUGESong_t main_song;
  extern const hUGESong_t main_song_fast;

  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x33;

  // Start both channels
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
  hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
  hUGE_mute_channel(HT_CH4, HT_CH_MUTE);
  
  // Add hUGE driver to VBL Interrupt handler
  add_VBL(hUGE_dosound);

  // Add score incrementer to VBL interrupt handler.
  add_VBL(increment_score_isr);

  /*
   * Turn on display and show background 
   */
  DISPLAY_ON;
  SHOW_BKG;

  uint8_t bomb_tiles[2];
  uint8_t progressbar_tiles[8];
  uint8_t *score_time_tiles = calloc(8, sizeof(uint8_t));
  uint8_t *high_score_tiles = calloc(20, sizeof(uint8_t));
  const uint8_t blank_win_tiles[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint8_t show_time;

  /*
   * Turn on RAM and grab high score
   */
  uint16_t *high_score_ptr = 0xA000; // RAM BANK 0 Address
  ENABLE_RAM;
  SWITCH_RAM(0);
  if (high_score_ptr[0] == 0xFFFF){
    high_score_ptr[0] = 0;
    high_score_ptr[1] = 0;
    high_score_ptr[2] = 0;
  }
  else{
    set_win_tiles(0, 0, 20, 1, blank_win_tiles); 
    highscore2tile(high_score_tiles, high_score_ptr);
    SHOW_WIN;
  }

  struct Sprite player;
  /**
   * Create bullet sprite array 
   */
  struct Sprite bullets[BULLET_ARR_SIZE];
  uint8_t bullets_arr_idx;
  uint8_t bullets_active;
  bool bullets_fired;
  bool bomb_dropped;
  bool shield_active;
  enum powerup active_powerup;
  uint8_t n_bullets;
  uint8_t n_bombs;

  // Collision map. The first 18 elements correspond to the first col in background tile map
  uint8_t *coll_map = calloc(COLUMN_HEIGHT*32, sizeof(uint8_t));  // 32x16
  uint8_t *bkg_map = calloc(COLUMN_HEIGHT*32, sizeof(uint8_t));  // 32x16
  bool copy_bkgmap_to_vram; 

  // Map generation variables.
  struct GenerationState gen_state;
  uint8_t gen_column_index;  // The index of the next column to generate.

  uint8_t current_input;
  uint8_t old_input;
  uint8_t dx;
  uint8_t dy;
  uint8_t frame_count;
  uint8_t scroll_count;
  int16_t damage_animation_counter; // When hit, skip checking collisions for this long
  uint8_t col_count;  // counter for number of columns scrolled. Used to calculate screen_count
  uint16_t screen_count; // number of screens scrolled
  uint8_t scroll_frames_per_pixel;  // Slower scrolling: How many frames to wait before scrolling one pixel
  uint8_t scroll_pixels_per_frame;  // Faster scrolling: How many pixels to scroll each frame
  uint8_t scroll_frames_count;  // Counts frames for scroll_frames_per_pixel scrolling
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
  uint8_t i, j;
  uint16_t ii;

  // Banking variables
  uint8_t last_bank;

  while (1){
    // Load the window contents
    show_time = 1;
    bullets_arr_idx = 0;
    n_bullets = MAX_BULLETS;
    n_bombs = MAX_BOMBS;

    bomb_tiles[0] = powerups_tilemap_offset + 1;  // Deselected bomb
    bomb_tiles[1] = n_bombs + 1;

    progressbar_tiles[0] = progressbar_tilemap_offset + 1; // left edge of bar
    for (i = 1; i < 7; i++){
      progressbar_tiles[i] = progressbar_tilemap_offset + 2; // center of bar
    }
    progressbar_tiles[7] = progressbar_tilemap_offset + 3; // right edge of bar

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
  
    // Clear collision map and background map
    for (ii=0; ii<32*COLUMN_HEIGHT; ++ii) {
      coll_map[ii] = 0;
      bkg_map[ii] = 0;
    }

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
    col_count = 0;  // counter for number of columns scrolled. Used to calculate screen_count
    screen_count = 0; // number of screens scrolled
    scroll_frames_per_pixel = 1;
    scroll_pixels_per_frame = 0;
    scroll_frames_count = 0;
    player_collision = 0;
    bullet_collision = 0;
    damage_animation_state = HIDDEN;
    gen_state.biome_id = 0;
    gen_state.biome_column_index = 0;
    gen_column_index = 20;

    hUGE_init(&intro_song);
    hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
    hUGE_mute_channel(HT_CH2, HT_CH_PLAY);
    hUGE_mute_channel(HT_CH4, HT_CH_PLAY);
    waitpad(J_START);
    waitpadup();
    
    for (i=0; i<COLUMN_HEIGHT; i++){
      for (j=0; j<SCREEN_TILE_WIDTH; j++){
        bkg_map[i*ROW_WIDTH+j] = tutorial_screen_map[i*SCREEN_TILE_WIDTH+j];
      }
    }
    set_bkg_tiles(0, 0, 32, COLUMN_HEIGHT, bkg_map);

    set_win_tiles(0, 0, 20, 1, blank_win_tiles); 
    set_win_tiles(9, 0, 2, 1, bomb_tiles);
    set_win_tiles(0, 0, 8, 1, progressbar_tiles);
    hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
    hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
    hUGE_mute_channel(HT_CH4, HT_CH_MUTE);

    wait(10);
    
    hUGE_init(&main_song);
    hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
    hUGE_mute_channel(HT_CH2, HT_CH_PLAY);
    hUGE_mute_channel(HT_CH4, HT_CH_PLAY);

    SHOW_SPRITES;
    SHOW_WIN;
    initrand(DIV_REG);

    last_bank = CURRENT_BANK;
    SWITCH_ROM(1);
    for (uint8_t i = 0; i < ROW_WIDTH - SCREEN_TILE_WIDTH; ++i) {
      generate_next_column(&gen_state, coll_map+gen_column_index, bkg_map+gen_column_index);
      if (++gen_column_index >= ROW_WIDTH) {
        gen_column_index = 0;
      }
    }
    set_bkg_tiles(0, 0, 32, COLUMN_HEIGHT, bkg_map);
    SWITCH_ROM(last_bank);

    player_sprite_base_id = 0;
    bullet_sprite_base_id = 19;

    copy_bkgmap_to_vram = false;

    // Reset score
    isr_frame_counter = 0;
    playtime_seconds = 0;
    playtime_minutes = 0;
    playtime_hours = 0;
    game_paused = 0;
    
    // Reset score tiles
    for (int i=0; i < 8; i++){
      score_time_tiles[i] = 0;
    }
    score = 0;

    score2tile(score_time_tiles, extra_font_offset, show_time);

    while(1) {
      current_input = joypad();
      player_collision = 0;
      bullet_collision = 0;
      copy_bkgmap_to_vram = false;


      // D-PAD
      if (KEY_PRESSED(J_START)){
        game_paused = 1;
        hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
        hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
        hUGE_mute_channel(HT_CH4, HT_CH_MUTE);
        waitpadup();
        waitpad(J_START);
        waitpadup();
        hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
        hUGE_mute_channel(HT_CH2, HT_CH_PLAY);
        hUGE_mute_channel(HT_CH4, HT_CH_PLAY);
        damage_animation_state = SHOWN;
        game_paused = 0;
      }

      if (KEY_PRESSED(J_SELECT)){
        waitpadup();
        show_time = ~(show_time) & 0x1;
      }

      if (!KEY_PRESSED(J_UP) && !KEY_PRESSED(J_DOWN) && \
          !KEY_PRESSED(J_LEFT) && !KEY_PRESSED(J_RIGHT)) 
      {
        dx = 0;
        dy = 0;
        player.sprite_tile_id = player_sprite_base_id;

        // Reset player collision box to default
        player.cb_x_offset = 1;
        player.cb_y_offset = 2;
        player.cb.h = 4;
        player.cb.w = 5;
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

      if (KEY_FIRST_PRESS(J_A) && n_bullets > 0) {
        bullets_fired = true;
        n_bullets--;

        b.type = BULLET;
        b.sprite_id = bullets_arr_idx + 1; // +1 so we dont override the player (always sprite_id 0)
        b.speed = 4;
        b.x = player.x;
        b.y = player.y;

        // Collision box
        b.cb_x_offset = 0;
        b.cb_y_offset = 2;
        b.cb.x = b.x + b.cb_x_offset;
        b.cb.y = b.y + b.cb_y_offset;
        b.cb.h = 4;
        b.cb.w = 4;
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

      if (KEY_FIRST_PRESS(J_B) && n_bombs > 0) {
        bomb_dropped = true;
        n_bombs--;
        play_bomb_sound();
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
          if ((bullet_collision > 0) && (bullet_collision < 235)) {
            // Hit something
            // Force to 1 so the next || works
            bullet_collision = 1;
          }
          else {
            // Collided with a pickup (can't destroy)
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
        if ((player_collision > 0) && (player_collision < 235)) {
          player.health -= COLLISION_DAMAGE;
          damage_animation_counter = COLLISION_TIMEOUT;
          player_collision = 1;
        }
        else if (player_collision == HELATH_KIT_ID) {
          if (player.health < 100){
            // Prevent health overflow (int8 maxes at 128)
            if ((100 - player.health) >= HEALTH_KIT_VALUE){
              if(player.health < 50){
                player.health += 2*HEALTH_KIT_VALUE;
              }
              else if (player.health < 20){
                player.health += 4*HEALTH_KIT_VALUE;
              }
              else{
                player.health += HEALTH_KIT_VALUE;
              }
            }
            else {
              player.health += 100 - player.health;
            }
          }
          player_collision = 1;
          play_health_sound();
        }
        else if (player_collision == SHIELD_ID) {
          shield_active = true;
          damage_animation_counter = SHIELD_DURATION;
          player_sprite_base_id += 10;
          player_collision = 0;
          play_shield_sound();
        }

        if (player.health <= 0){
          game_paused = 1;

          hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
          hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
          hUGE_mute_channel(HT_CH4, HT_CH_MUTE);
          player.sprite_tile_id = 9;
          set_sprite_tile(player.sprite_id, player.sprite_tile_id);
          wait(10);
          HIDE_BKG;
          play_gameover_sound();
          fadeout();
          HIDE_SPRITES;

          update_health_bar(&player, progressbar_tiles, &player_sprite_base_id, progressbar_tilemap_offset);
          gameover_screen(score_time_tiles, extra_font_offset, high_score_ptr);

          // Load title screen
          set_bkg_tiles(0,0,20,COLUMN_HEIGHT,game_titlescreen);
          move_bkg(0,0);
          player_sprite_base_id = 0; // Reset to initial sprite
          player.sprite_tile_id = player_sprite_base_id;
          set_sprite_tile(player.sprite_id, player.sprite_tile_id);
          set_win_tiles(0, 0, 20, 1, blank_win_tiles); 
          highscore2tile(high_score_tiles, high_score_ptr);
          SHOW_WIN;
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

      // Scroll the screen.
      if (scroll_frames_per_pixel != 0) {
        ++scroll_frames_count;
        if (scroll_frames_count == scroll_frames_per_pixel) {
          scroll_frames_count = 0;
          scroll_bkg(1, 0);
          ++scroll_count;
        }
      } else {
        scroll_bkg(scroll_pixels_per_frame, 0);
        scroll_count += scroll_pixels_per_frame;
      }

      if (scroll_count >= 8){
        scroll_count = 0;
        last_bank = CURRENT_BANK;
        SWITCH_ROM(1);
        generate_next_column(&gen_state, coll_map+gen_column_index, bkg_map+gen_column_index);
        if (++gen_column_index >= ROW_WIDTH) {
          gen_column_index = 0;
        }
        SWITCH_ROM(last_bank);
        copy_bkgmap_to_vram = true;

        ++col_count;

        // Add a bullet every few columns we scroll.
        if (col_count % 2 == 0 && n_bullets < MAX_BULLETS) {
          ++n_bullets;
        }

        if (col_count == 20) {
          col_count = 0;
          ++screen_count;
          score += 5;

          // Add a bomb every few screens we scroll.
          if (screen_count % 3 == 0 && n_bombs < MAX_BOMBS) {
            ++n_bombs;
          }

          // Increase scroll speed after some time.
          if (screen_count == 3) {
            scroll_frames_per_pixel = 0;
            scroll_frames_count = 0;
            scroll_pixels_per_frame = 1;
          } else if (screen_count == 400) {
            hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
            hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
            hUGE_mute_channel(HT_CH4, HT_CH_MUTE);
            hUGE_init(&main_song_fast);
            scroll_pixels_per_frame = 2;
            player.speed = 2;
            hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
            hUGE_mute_channel(HT_CH2, HT_CH_PLAY);
            hUGE_mute_channel(HT_CH4, HT_CH_PLAY);
            
          }
        }
      }

      if ((frame_count & 0x3) == 0) { // %4
        // Update HUD
        if (n_bombs > 0) {
          bomb_tiles[1] = n_bombs + 1;
        } else {
          bomb_tiles[1] = 1;
        }
        set_win_tiles(9, 0, 2, 1, bomb_tiles);
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
      
      score2tile(score_time_tiles, extra_font_offset, show_time);
      if (copy_bkgmap_to_vram){
        // Write the entire map to VRAM
        set_bkg_tiles(0, 0, 32, COLUMN_HEIGHT, bkg_map);
      }
    }
  }
}
