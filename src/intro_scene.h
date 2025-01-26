#ifndef _INTRO_SCENE_H_
#define _INTRO_SCENE_H_

#include <gb/gb.h>

#include "common.h"
#include "sprite_data.h"
#include "tile_data.h"

extern const hUGESong_t intro_song;

// Shows the intro
static void show_intro(void){
  uint8_t x, y, idx;
  uint16_t t;
  int8_t dy[] = {-1, -1, -1, 1, 1, 1, 1, 1, 1, -1, -1, -1,  0,  0,  0,  0,  0,  0, 0};
  int8_t dx[] = { 1,  1,  1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  x = 40;
  y = 80;

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
  set_sprite_data(0, 4, player_large_sprites);
  set_sprite_tile(0, 0);
  set_sprite_tile(1, 1);
  set_sprite_tile(2, 2);
  set_sprite_tile(3, 3);

  // Move sprite to initial position
  move_sprite(0, x, y);
  move_sprite(1, x+8, y);
  move_sprite(2, x, y+8);
  move_sprite(3, x+8, y+8);
  fade_in();
  
  SHOW_BKG;
  SHOW_SPRITES;

  t = 0;
  idx = 0;
  while(1){
    y += dy[idx];
    x += dx[idx];
    ++t;
    move_sprite(0, x, y);
    move_sprite(1, x+8, y);
    move_sprite(2, x, y+8);
    move_sprite(3, x+8, y+8);
    vsync();
    if (idx < 12){
      scroll_bkg(2, 0);
    }
    else {
      // Slow down
      scroll_bkg(1, 0);
    }
    if (t == 10){
      ++idx;
      t = 0;
      if (idx == sizeof(dy)) {
        // End of sprite animation
        break;
      }
    }
  }

  /* Enter planet animation */
  // Align screen to the right
  while (SCX_REG != 96){
    scroll_bkg(1,1);
    vsync();
  }

  //set_bkg_submap(0, 0, 32, 4, intro_atmosphere_map+14*32, 32);
  set_bkg_tiles(0,0,20,4,empty_screen_map);
  set_bkg_tiles(20,0,12,4,empty_screen_map);
  while (SCY_REG != 112){
    scroll_bkg(1,1);
    vsync();
  }

  set_bkg_tiles(0,4,20,10,empty_screen_map);
  set_bkg_tiles(20,4,12,10,empty_screen_map);

  while (SCX_REG != 0){
    scroll_bkg(1,1);
    vsync();
  }

  set_bkg_tiles(0,14,20,11,empty_screen_map);
  set_bkg_tiles(20,14,12,11,empty_screen_map);

  // Fully descend
  while (SCY_REG != 240){
    scroll_bkg(1,1);
    vsync();
  }

  while (SCX_REG != 40){
    scroll_bkg(1,0);
    vsync();
  }
  set_bkg_tiles(12,16,20,16,title_screen_map);
  set_bkg_tiles(0,26,12,4,empty_screen_map);
  move_bkg(0,24);

  for (uint8_t i=0; i < 8; i++){
    scroll_bkg(0,1);
    vsync();
  }  

  uint8_t count = 0;
  while (SCX_REG != 96){
    scroll_bkg(1,1);
    ++count;
    if (count == 6){
      scroll_sprite(0,1,0);
      scroll_sprite(1,1,0);
      scroll_sprite(2,1,0);
      scroll_sprite(3,1,0);
    }
    else if (count == 12){
      scroll_sprite(0,1,1);
      scroll_sprite(1,1,1);
      scroll_sprite(2,1,1);
      scroll_sprite(3,1,1);
      count = 0;
    }
    vsync();
  }
  set_bkg_tiles(12,30,20,2, title_screen_map+14*SCREEN_TILE_WIDTH);
  set_bkg_tiles(12,0,20,2, title_screen_map+16*SCREEN_TILE_WIDTH);

  // Fly off and spell PRESS START
  for (uint8_t i = 0; i < 130/4; i++){
    scroll_sprite(0,4,0);
    scroll_sprite(1,4,0);
    scroll_sprite(2,4,0);
    scroll_sprite(3,4,0);
    vsync();
  }

  // End of intro scene
  move_sprite(0, 0, 0);
  move_sprite(1, 0, 0);
  move_sprite(2, 0, 0);
  move_sprite(3, 0, 0);
  HIDE_SPRITES;
  move_bkg(0,0);
  return;
}

#endif
