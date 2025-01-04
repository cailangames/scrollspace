#ifndef _INTRO_SCENE_H_
#define _INTRO_SCENE_H_

#include <gb/gb.h>

#include "common.h"
#include "player_sprites.h"
#include "intro_stars_tiles.h"
#include "intro_stars_map.h"
#include "intro_atmosphere_tiles.h"
#include "intro_atmosphere_map.h"
#include "title_screens.h"

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
  set_bkg_data(0, sizeof(intro_stars_tiles)/TILE_SIZE_BYTES, intro_stars_tiles);
  set_bkg_tiles(0, 0, 32, 18, intro_stars_map);
  
  // Load player sprite and set it to sprite 0
  set_sprite_data(0, 1, player_data);
  set_sprite_tile(0, 0);

  // Move sprite to initial position
  move_sprite(0, x, y);
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
  set_bkg_data(0xC, sizeof(intro_atmosphere_tiles)/TILE_SIZE_BYTES, intro_atmosphere_tiles);
  while (SCX_REG != 95){
    scroll_bkg(1,0);
    vsync();
  }

  // Load first 12 columns of atmosphere and scroll the bkg
  set_bkg_submap(0, 0, 12, 18, intro_atmosphere_map, 32);
  for (idx=0; idx < 12*8; idx++){
    scroll_bkg(1,0);
    vsync();
  }

  // Load next 12 columns of atmosphere and scroll the bkg
  set_bkg_submap(12, 0, 12, 18, intro_atmosphere_map, 32);
  for (idx=0; idx < 12*8; idx++){
    scroll_bkg(1,0);
    vsync();
  }

  // Load last 8 columns of atmosphere and scroll the bkg
  set_bkg_submap(24, 0, 8, 18, intro_atmosphere_map, 32);
  for (idx=0; idx < 8*8; idx++){
    scroll_bkg(1,0);
    vsync();
  }

  // Load first 12 columns of empty screen
  set_bkg_submap(0, 0, 12, 18, empty_screen, 32);
  for (idx=0; idx < 12*8; idx++){
    scroll_bkg(1,0);
    vsync();
  }

  // Load next 12 columns of empty screen
  set_bkg_submap(12, 0, 8, 18, empty_screen, 32);
  for (idx=0; idx < 8*8; idx++){
    scroll_bkg(1,0);
    vsync();
  }

  // // Load last 8 columns of empty screen
  // set_bkg_submap(24, 0, 8, 18, empty_screen, 32);
  // for (idx=0; idx < 8*8; idx++){
    // scroll_bkg(1,0);
    // vsync();
  // }

  // End of intro scene
  HIDE_SPRITES;
  move_bkg(0,0);
  return;
}

#endif
