#ifndef _CAILAN_GAMES_LOGO_H_
#define _CAILAN_GAMES_LOGO_H_

#include <gb/gb.h>

#include "cailan_games_logo_map.h"
#include "cailan_games_logo_tiles.h"
#include "common.h"
#include "display_effects.h"
#include "logo_cursor_sprites.h"
#include "sound_effects.h"

// Shows the logo screen.
static void show_logo_screen(void) {
  HIDE_BKG;

  // Load logo tiles and map into VRAM 
  set_bkg_data(0, sizeof(cailan_games_logo_tiles)/TILE_SIZE_BYTES, cailan_games_logo_tiles);
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, cailan_games_logo_map);
  
  // Load the cursor sprite into VRAM
  set_sprite_data(0, 1, logo_cursor_data);
  set_sprite_tile(0, 0);
  set_sprite_tile(1, 0);
  
  // Enable music
  play_all_channels();
  wait_frames(60);
  fade_in();
  SHOW_BKG;

  // Initialize the CBT SFX and add it to the VBL
  // CBTFX_PLAY_SFX_02;
  // add_VBL(CBTFX_update);

  // The cursor has two sprites: A top half and a bottom half. The below code controls both halves
  // and blinks them while the logo screen is playing.
  move_sprite(0, 119+8, 75+17);
  move_sprite(1, 119+8, 83+17);
  SHOW_SPRITES;
  play_health_sound();
  for (uint8_t i=0; i<150; i++){
    if ((i == 30) || (i == 90) || (i == 149)){
      move_sprite(0, 0, 0);
      move_sprite(1, 0, 0);
    }
    else if ((i == 60) || (i == 120)) {
      move_sprite(0, 119+8, 75+17);
      move_sprite(1, 119+8, 83+17);
    }
    vsync();
  }

  // Remove the CBT SFX from the VBL
  vsync();
  // remove_VBL(CBTFX_update);
  mute_all_channels();

  fade_out();
  HIDE_SPRITES;
}

#endif
