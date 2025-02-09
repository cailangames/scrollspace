#ifndef _INTRO_SCENE_H_
#define _INTRO_SCENE_H_

#include <gb/gb.h>

#include "common.h"
#include "sprite_data.h"
#include "tile_data.h"

extern const hUGESong_t intro_song;

/**
 * Generated using  
 * -(36*np.sin(2*np.pi/256*t)).astype(int) + 72
 */
const int8_t sin_lut[] = 
  {72,  72,  71,  70,  69,  68,  67,  66,  65,  65,  64,  63,  62,
   61,  60,  60,  59,  58,  57,  56,  56,  55,  54,  53,  52,  52,
   51,  50,  50,  49,  48,  48,  47,  46,  46,  45,  45,  44,  44,
   43,  43,  42,  42,  41,  41,  40,  40,  40,  39,  39,  39,  38,
   38,  38,  38,  37,  37,  37,  37,  37,  37,  37,  37,  37,  36,
   37,  37,  37,  37,  37,  37,  37,  37,  37,  38,  38,  38,  38,
   39,  39,  39,  40,  40,  40,  41,  41,  42,  42,  43,  43,  44,
   44,  45,  45,  46,  46,  47,  48,  48,  49,  50,  50,  51,  52,
   52,  53,  54,  55,  56,  56,  57,  58,  59,  60,  60,  61,  62,
   63,  64,  65,  65,  66,  67,  68,  69,  70,  71,  72,  72,  72,
   73,  74,  75,  76,  77,  78,  79,  79,  80,  81,  82,  83,  84,
   84,  85,  86,  87,  88,  88,  89,  90,  91,  92,  92,  93,  94,
   94,  95,  96,  96,  97,  98,  98,  99,  99, 100, 100, 101, 101,
   102, 102, 103, 103, 104, 104, 104, 105, 105, 105, 106, 106, 106,
   106, 107, 107, 107, 107, 107, 107, 107, 107, 107, 108, 107, 107,
   107, 107, 107, 107, 107, 107, 107, 106, 106, 106, 106, 105, 105,
   105, 104, 104, 104, 103, 103, 102, 102, 101, 101, 100, 100,  99,
   99,  98,  98,  97,  96,  96,  95,  94,  94,  93,  92,  92,  91,
   90,  89,  88,  88,  87,  86,  85,  84,  84,  83,  82,  81,  80,
   79,  79,  78,  77,  76,  75,  74,  73,  72};

// Shows the intro
static void show_intro(void){
  uint8_t x, y, idx;
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
  for (idx=0; idx < 255; idx++){
    if (idx < 128){
      x += 1;
    }
    else {
      x -= 1;
    }
    y = sin_lut[idx];
    move_sprite(0, x, y);
    vsync();
    scroll_bkg(2,0); 
  }  
  // One more scroll to get the screen back at 0,0
  vsync();
  scroll_bkg(2,0); 

  waitpad(J_START);
  waitpadup();

  /*
   * Enter planet animation 
   * Move screen diagonally
   */
  // Align screen to the right
  while (SCY_REG < 112){
    scroll_bkg(1,1);
    vsync();
  }

  set_bkg_tiles(0,0,20,14,empty_screen_map);
  set_bkg_tiles(20,0,12,14,empty_screen_map);

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

  while (SCY_REG < 176){
    scroll_bkg(1,1);
    vsync();
  }

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

  while (SCY_REG < 208){
    scroll_bkg(1,1);
    vsync();
  }
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

  while (SCY_REG < 224){
    scroll_bkg(1,1);
    vsync();
  }
  set_bkg_tiles(0,14,20,14,empty_screen_map);
  set_bkg_tiles(20,14,12,14,empty_screen_map);
  
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

  set_bkg_tiles(0,28,20,2,empty_screen_map);
  set_bkg_tiles(20,28,12,2,empty_screen_map);

  while (SCX_REG != 0){
    scroll_bkg(1,1);
    vsync();
  }
  // Load title screen
  set_bkg_tiles(12,13,20,18,title_screen_map);
  // for (uint8_t row=22; row<31; row++){
    // for (uint8_t col=0; col<12; col++){
      // set_bkg_tile_xy(col, row, 165);
    // }
  // }

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
  move_sprite(0, 0, 0);
  HIDE_SPRITES;
  move_bkg(0,0);
  return;
}

#endif
