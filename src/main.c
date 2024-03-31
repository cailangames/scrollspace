#include <gb/gb.h>
#include <gbdk/font.h>
#include <rand.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "player.h"
#include "player_py.h"
#include "block_tiles_py.h"

#define KEY_PRESSED(K) (current_input & (K))
#define FONT_OFFSET 36
#define MAPBLOCK_IDX  FONT_OFFSET

void wait(uint8_t n){
  uint8_t i;
  for (i=0; i < 10; i++){
    wait_vbl_done();
  }
}

void generate_column(uint8_t *col_idx, uint8_t *prev_col){
  uint8_t i;
  uint8_t n;
  initrand(DIV_REG);

  uint8_t *map_col = calloc(18, sizeof(uint8_t));
  for (i = 0; i < 18; i++){
    n = rand();
    n = rand();

    if (n < 32){
      map_col[i] = 0;
      map_col[i+1] = 0;
      i++;
    }
    else {
      map_col[i] = MAPBLOCK_IDX;
    }
  }
  set_bkg_tiles(*col_idx, 0, 1, 18, map_col);
  // memcpy(prev_col, map_col, 18);
  // *col_idx = *col_idx+1;
  // *col_idx = (*col_idx+1) & 0x1F;
}

bool check_collisions(struct Player *player){
  uint8_t tile_idx = get_bkg_tile_xy(player->x, player->y);

  return tile_idx >= MAPBLOCK_IDX;
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

  uint8_t *empty_col = calloc(18, sizeof(uint8_t));
  uint8_t col_idx = 20;
  uint8_t prev_col[18];
  generate_column(&col_idx, prev_col);

  /*
   * Create a player and display the sprite 
   */
  struct Player player;
  player.sprite_id = 0;
  player.sprite_tile_id = 0;
  player.x = 20;
  player.y = 77;
  player.speed = 2;
  player.dir = LEFT;
  move_sprite(player.sprite_id, player.x, player.y);

  /*
   * Turn on display and show sprites and background 
   */
  DISPLAY_ON;
  SHOW_SPRITES;
  SHOW_BKG;

  /*
   * Game Looop
   */
  uint8_t i;
  uint8_t current_input;
  uint8_t dx,dy;
  uint16_t frame_count = 0;
  uint8_t scroll_count = 0;
  uint8_t old_scx = SCX_REG % 8;
  initrand(DIV_REG);

  while(1) {
    current_input = joypad();

    // D-PAD
    if (KEY_PRESSED(J_UP)){
      dx = 0;
      dy = -player.speed;
      player.sprite_tile_id = 1;
    }
    else if (KEY_PRESSED(J_DOWN)){
      dx = 0;
      dy = player.speed;
      player.sprite_tile_id = 2;
    }
    else if (KEY_PRESSED(J_RIGHT)){
      dx = player.speed;
      dy = 0;
      player.sprite_tile_id = 0;
    }
    else if (KEY_PRESSED(J_LEFT)){
      dx = -player.speed;
      dy = 0;
      player.sprite_tile_id = 0;
    }
    else{
      dx = 0;
      dy = 0;
      player.sprite_tile_id = 0;
    }

    // Update player position
    player.x += dx;
    player.y += dy;
    set_sprite_tile(player.sprite_id, player.sprite_tile_id);
    move_sprite(player.sprite_id, player.x, player.y);

    frame_count += 1;
    if ((frame_count % 120) == 0){
      generate_column(&col_idx, prev_col);
    }
    
    if ((frame_count % 4) == 0){
      scroll_bkg(1,0);
      if (scroll_count == 7){
        col_idx = (col_idx+1) & 0x1F;
        scroll_count = 0;

        // Clear the column outside of the camera view
        set_bkg_tiles(old_scx, 0, 1, 18, empty_col);
        old_scx = (old_scx+1) & 0x1F;
      }
      else {
        scroll_count++;
      }
    }

    // if (check_collisions(&player)){
      // for (i=0; i<3;i++){
        // HIDE_SPRITES;
        // wait(10);
        // SHOW_SPRITES;
        // wait(10);
      // }
    // }

    // Wait for frame to finish drawing
    wait_vbl_done();
  }
}
