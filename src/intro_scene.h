#ifndef _INTRO_SCENE_H_
#define _INTRO_SCENE_H_

#include <gb/gb.h>

#include "common.h"
#include "sprite_data.h"
#include "tile_data.h"

extern const hUGESong_t intro_song;
extern uint8_t background_map[COLUMN_HEIGHT*ROW_WIDTH];

// Shows the intro
static void show_intro(void){
  uint8_t x, y, idx;
  uint8_t row, col;
  
  // Fill the background map with empty tiles
  for (row=0; row < SCREEN_TILE_HEIGHT; ++row){
    for (col=0; col < ROW_WIDTH; ++col){
      background_map[row*ROW_WIDTH+col] = 0x98;
    }
  }

  x = 24;
  y = 72;

#if ENABLE_MUSIC
  hUGE_init(&intro_song);
  play_all_channels();
  add_VBL(hUGE_dosound);
#endif

  HIDE_BKG;
  // Load intro tiles and map into VRAM
  set_bkg_data(INTRO_SCENE_OFFSET, sizeof(intro_stars_tiles)/TILE_SIZE_BYTES, intro_stars_tiles);
  set_bkg_data(INTRO_SCENE_STARS_OFFSET, sizeof(intro_atmosphere_tiles)/TILE_SIZE_BYTES, intro_atmosphere_tiles);
  set_bkg_data(TITLE_SCREEN_OFFSET, sizeof(title_screen_tiles)/TILE_SIZE_BYTES, title_screen_tiles);
  set_bkg_tiles(0, 0, 32, 18, intro_stars_map);
  set_bkg_tiles(0, 18, 32, 14, intro_atmosphere_map);
  
  // Load player sprite and set it to sprite 0
  set_sprite_data(0, 1, player_sprites);
  set_sprite_data(1, 27, intro_player_heatshield_sprites_data);
  for (uint8_t i=0; i<28; i++){
    set_sprite_tile(i, i);
  }

  // Move sprite to initial position
  move_sprite(0, x, y);
  fade_in();
  
  SHOW_BKG;
  SHOW_SPRITES;

  /*
   * Horizontal movement through space
   */
  for (idx=0; idx < 90; idx++){
    vsync();
    scroll_bkg(4,0); 
  }  
  for (idx=0; idx < 36; idx++){
    vsync();
    scroll_bkg(3,0); 
  }  
  for (idx=0; idx < 22; idx++){
    vsync();
    scroll_bkg(2,0); 
  }  

  /*
   * Enter planet animation 
   * Move screen diagonally
   */
  // Align screen to the right
  while (SCY_REG < 112){
    scroll_bkg(1,1);
    vsync();
  }
  
  // set_bkg_tiles(0,0,20,14,empty_screen_map);
  // set_bkg_tiles(20,0,12,14,empty_screen_map);
  set_bkg_tiles(0,0,20,14,background_map);
  set_bkg_tiles(20,0,12,14,background_map);

  // Switch to heat shield sprites
  move_sprite(0,0,0);
  move_sprite(1, x-8, y-8);  // top left
  move_sprite(2, x, y-8);  // top middle
  move_sprite(3, x+8, y-8);  // top right
  move_sprite(4, x-8, y); // left of player
  move_sprite(5, x, y); // player sprite
  move_sprite(6, x+8, y); // right of player
  move_sprite(7, x-8, y+8);  // bottom left
  move_sprite(8, x, y+8);  // bottom middle
  move_sprite(9, x+8, y+8);  // bottom right

  hUGE_mute_channel(HT_CH4, HT_CH_MUTE);

  NR42_REG = 0x00;
  NR44_REG = 0x00;

  // Play sound effect.
  NR41_REG = 0x00;
  NR42_REG = 0xF7;
  NR43_REG = 0x81;
  NR44_REG = 0x80;

  idx = 0;
  while (SCY_REG < 176){
    scroll_bkg(1,1);
    vsync();
    ++idx;
    if ((idx % 8) == 0 ){
      OBP0_REG = 0x8C; // 0b1000 1100 - Dark gray, white, black, white
      idx = 0;
    }
    else {
      OBP0_REG = 0xE4; // 0b1101 0000 - Black, Dark Grey, Light gray, white
    }
  }
  OBP0_REG = 0xE4; // 0b1110 0100 - Black, Light gray, white, transparent

  // Switch to dissipated heat shield sprites
  for (uint8_t i=1; i < 10; i++){
    move_sprite(i,0,0);
  } 
  move_sprite(10, x-8, y-8);  // top left
  move_sprite(11, x, y-8);  // top middle
  move_sprite(12, x+8, y-8);  // top right
  move_sprite(13, x-8, y); // left of player
  move_sprite(14, x, y); // player sprite
  move_sprite(15, x+8, y); // right of player
  move_sprite(16, x-8, y+8);  // bottom left
  move_sprite(17, x, y+8);  // bottom middle
  move_sprite(18, x+8, y+8);  // bottom right

  idx = 0;
  while (SCY_REG < 208){
    scroll_bkg(1,1);
    vsync();
    ++idx;
    if ((idx % 8) == 0 ){
      OBP0_REG = 0x8C; // 0b00 1100 - Light gray, white, black, transparent
      idx = 0;
    }
    else {
      OBP0_REG = 0xE4; // 0b1101 0000 - Black, Light gray, white, transparent
    }
  }
  OBP0_REG = 0xE4; // 0b1110 0100 - Black, Light gray, white, transparent

  // Switch to almost gone dissipated heat shield sprites
  for (uint8_t i=10; i < 19; i++){
    move_sprite(i,0,0);
  } 
  move_sprite(19, x-8, y-8);  // top left
  move_sprite(20, x, y-8);  // top middle
  move_sprite(21, x+8, y-8);  // top right
  move_sprite(22, x-8, y); // left of player
  move_sprite(23, x, y); // player sprite
  move_sprite(24, x+8, y); // right of player
  move_sprite(25, x-8, y+8);  // bottom left
  move_sprite(26, x, y+8);  // bottom middle
  move_sprite(27, x+8, y+8);  // bottom right

  idx = 0;
  while (SCY_REG < 224){
    scroll_bkg(1,1);
    vsync();
    ++idx;
    if ((idx % 8) == 0 ){
      OBP0_REG = 0x8C; // 0b00 1100 - Light gray, white, black, transparent
      idx = 0;
    }
    else {
      OBP0_REG = 0xE4; // 0b1101 0000 - Black, Light gray, white, transparent
    }
  }
  OBP0_REG = 0xE4; // 0b1110 0100 - Black, Light gray, white, transparent

  // set_bkg_tiles(0,14,20,14,empty_screen_map);
  // set_bkg_tiles(20,14,12,14,empty_screen_map);
  set_bkg_tiles(0,14,20,14,background_map);
  set_bkg_tiles(20,14,12,14,background_map);
  
  while (SCY_REG < 240){
    scroll_bkg(1,1);
    vsync();
  }
  // Restart channel.
  hUGE_mute_channel(HT_CH4, HT_CH_PLAY);
  
  // Hide all sprites except the plane itself
  for (uint8_t i=1; i < 28; i++){
    move_sprite(i,0,0);
  } 
  move_sprite(0,x,y);

  // set_bkg_tiles(0,28,20,2,empty_screen_map);
  // set_bkg_tiles(20,28,12,2,empty_screen_map);
  set_bkg_tiles(0,28,20,2,background_map);
  set_bkg_tiles(20,28,12,2,background_map);

  while (SCX_REG != 0){
    scroll_bkg(1,1);
    vsync();
  }
  // Load title screen
  set_bkg_tiles(12,13,20,18,title_screen_map);
  // Load the bricks on the missing areas
  for (row=1; row<13; row++){
    for (col=20; col<=31; col++){
      set_bkg_tile_xy(col, row, 165);
    }
  }
  for (row=22; row<31; row++){
    for (col=3; col<12; col++){
      set_bkg_tile_xy(col, row, 165);
    }
  }

  // Set sprite priority so that bkg are drawn on top 
  set_sprite_prop(0,0x80);

  while (SCY_REG < 8){
    scroll_bkg(0,1);
    vsync();
  }

  while (SCX_REG < 96){
    scroll_bkg(1,1);
    vsync();
  }

  uint8_t count = 0;
  for (count=0; count < 12; count++){
    scroll_sprite(0,2,2);
    vsync();
  }

  // Fly off and spell PRESS START
  for (uint8_t i = 0; i < 116/4; i++){
    scroll_sprite(0,4,0);
    vsync();
  }

  // End of intro scene
  set_sprite_prop(0,0);  // Reset sprite priority to default
  move_sprite(0, 0, 0);
  HIDE_SPRITES;
  move_bkg(0,0);
  return;
}

#endif
