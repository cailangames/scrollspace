#pragma bank 1

#include "title_screen.h"

#include <gb/gb.h>
#include <hUGEDriver.h>
#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "display_effects.h"
#include "score.h"
#include "songs.h"
#include "sound_effects.h"
#include "text_data.h"
#include "tile_data.h"
#include "wait.h"

// Shows the title screen.
void show_title_screen(bool restart_song) BANKED {
  HIDE_WIN;
  clear_window();

  // Set the background tiles for the title screen.
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, title_screen_map);

  // Add the "PRESS START" text.
  set_bkg_tiles(7, 11, UINT8_ARRARY_SIZE(press_text), 1, press_text);
  set_bkg_tiles(10, 12, UINT8_ARRARY_SIZE(start_text), 1, start_text);

  if (restart_song) {
    fade_in();
    SHOW_BKG;

    __critical {
      hUGE_init(&intro_song);
      add_VBL(hUGE_dosound);
    }
    play_all_channels();
  }

  for (uint8_t i = 0; i < 5; ++i) {
    background_map[i] = EMPTY_TILE;
  }

  // Blink "PRESS START" while waiting for user input.
  uint8_t counter = 0;
  while (true) {
    uint8_t input = joypad();
    if (KEY_PRESSED(input, J_START | J_A)) {
      wait_for_keys_released(J_START | J_A);
      return;
    }

    vsync();
    ++counter;
    if (counter == 20) {
      // Clear the "PRESS START" text.
      set_bkg_tiles(7, 11, 5, 1, background_map);
      set_bkg_tiles(10, 12, 5, 1, background_map);
    } else if (counter == 40) {
      // Add the "PRESS START" text.
      set_bkg_tiles(7, 11, UINT8_ARRARY_SIZE(press_text), 1, press_text);
      set_bkg_tiles(10, 12, UINT8_ARRARY_SIZE(start_text), 1, start_text);
      counter = 0;
    }
  }
}
