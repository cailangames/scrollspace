#pragma bank 1

#include <gb/gb.h>
#include <hUGEDriver.h>
#include "common.h"
#include "score.h"
#include "wait.h"
#include "display_effects.h"
#include "sound_effects.h"
#include "tile_data.h"
#include "songs.h"

// Shows the title screen.
void show_title_screen(bool restart_song) BANKED {
  HIDE_WIN;
  clear_window();
  // Copy title screen to background_map
  for (uint8_t i = 0; i < SCREEN_TILE_HEIGHT; ++i) {
    for (uint8_t j = 0; j < SCREEN_TILE_WIDTH; ++j) {
      background_map[i*SCREEN_TILE_WIDTH+j] = title_screen_map[i*SCREEN_TILE_WIDTH+j];
    }
  }
  
  // Add Press Start text
  background_map[10*SCREEN_TILE_WIDTH+6] = CHAR_P;
  background_map[10*SCREEN_TILE_WIDTH+7] = CHAR_R;
  background_map[10*SCREEN_TILE_WIDTH+8] = CHAR_E;
  background_map[10*SCREEN_TILE_WIDTH+9] = CHAR_S;
  background_map[10*SCREEN_TILE_WIDTH+10] = CHAR_S;

  background_map[11*SCREEN_TILE_WIDTH+8] = CHAR_S;
  background_map[11*SCREEN_TILE_WIDTH+9] = CHAR_T;
  background_map[11*SCREEN_TILE_WIDTH+10] = CHAR_A;
  background_map[11*SCREEN_TILE_WIDTH+11] = CHAR_R;
  background_map[11*SCREEN_TILE_WIDTH+12] = CHAR_T;

  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, background_map);
  
  if (restart_song){
    fade_in();
    SHOW_BKG;
  }
#if ENABLE_MUSIC
  if (restart_song){
    hUGE_init(&intro_song);
    play_all_channels();
    add_VBL(hUGE_dosound);
  }
#endif
  
  /*
   * Blink PRESS START while waiting for user input
   */
  uint8_t counter = 0;
  while (1){
    if (joypad() & J_START){
      wait_for_keys_released(J_START);
      return;
    }
    ++counter;
    if (counter == 15){
      // Add Press Start text
      background_map[10*SCREEN_TILE_WIDTH+6] = EMPTY_TILE;
      background_map[10*SCREEN_TILE_WIDTH+7] = EMPTY_TILE;
      background_map[10*SCREEN_TILE_WIDTH+8] = EMPTY_TILE;
      background_map[10*SCREEN_TILE_WIDTH+9] = EMPTY_TILE;
      background_map[10*SCREEN_TILE_WIDTH+10] = EMPTY_TILE;

      background_map[11*SCREEN_TILE_WIDTH+8] = EMPTY_TILE;
      background_map[11*SCREEN_TILE_WIDTH+9] = EMPTY_TILE;
      background_map[11*SCREEN_TILE_WIDTH+10] = EMPTY_TILE;
      background_map[11*SCREEN_TILE_WIDTH+11] = EMPTY_TILE;
      background_map[11*SCREEN_TILE_WIDTH+12] = EMPTY_TILE;
    }
    else if (counter == 30){
      // Add Press Start text
      background_map[10*SCREEN_TILE_WIDTH+6] = CHAR_P;
      background_map[10*SCREEN_TILE_WIDTH+7] = CHAR_R;
      background_map[10*SCREEN_TILE_WIDTH+8] = CHAR_E;
      background_map[10*SCREEN_TILE_WIDTH+9] = CHAR_S;
      background_map[10*SCREEN_TILE_WIDTH+10] = CHAR_S;

      background_map[11*SCREEN_TILE_WIDTH+8] = CHAR_S;
      background_map[11*SCREEN_TILE_WIDTH+9] = CHAR_T;
      background_map[11*SCREEN_TILE_WIDTH+10] = CHAR_A;
      background_map[11*SCREEN_TILE_WIDTH+11] = CHAR_R;
      background_map[11*SCREEN_TILE_WIDTH+12] = CHAR_T;
      counter = 0;
    }
    vsync();
    set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, background_map);
  }
}
