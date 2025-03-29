#include "gameover.h"

#include <gb/gb.h>
#include <hUGEDriver.h>
#include <rand.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "display_effects.h"
#include "player.h"
#include "score.h"
#include "sound_effects.h"
#include "text_data.h"
#include "title_screen.h"
#include "wait.h"
#include "weapons.h"

// Shows the game over screen.
static void show_gameover_screen(void) {
  HIDE_WIN;
  clear_window();

  // Pick random tip.
  uint8_t tip_index = MOD8(rand());
  const uint8_t* tip = tip_messages + (tip_index * TIP_MESSAGE_LENGTH);

  // Add walls at the top and bottom of the screen.
  memset(background_map, WALL_BLOCK_TILE, SCREEN_TILE_WIDTH);
  memset(background_map + SCREEN_TILE_WIDTH, EMPTY_TILE, SCREEN_TILE_WIDTH * (SCREEN_TILE_HEIGHT - 2));
  memset(background_map + (SCREEN_TILE_WIDTH * (SCREEN_TILE_HEIGHT - 1)), WALL_BLOCK_TILE, SCREEN_TILE_WIDTH);

  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, background_map);
  set_bkg_tiles(5, 3, UINT8_ARRARY_SIZE(game_over_text), 1, game_over_text);
  set_bkg_tiles(2, 5, UINT8_ARRARY_SIZE(score_text), 1, score_text);
  set_bkg_tiles(3, 8, UINT8_ARRARY_SIZE(best_text), 1, best_text);
  set_bkg_tiles(2, 13, UINT8_ARRARY_SIZE(tip_text), 1, tip_text);
  set_bkg_tiles(3, 14, TIP_WRAP_LENGTH, 1, tip);
  set_bkg_tiles(3, 15, TIP_MESSAGE_LENGTH - TIP_WRAP_LENGTH, 1, tip + TIP_WRAP_LENGTH);
  move_bkg(0, 0);

  display_gameover_scores();

  SHOW_BKG;
  fade_in();
  wait_for_keys_pressed(J_START | J_A | J_B);
  wait_for_keys_released(J_START | J_A | J_B);

  bool new_unlock = update_unlocks();
  if (new_unlock) {
    show_reward_screen();
  }

  fade_out();
  HIDE_BKG;
  SHOW_WIN;
}

void handle_gameover(void) {
  mute_all_channels();
  __critical {
    remove_VBL(hUGE_dosound);
  }

  set_sprite_tile(PLAYER_SPRITE_ID, DEATH_SPRITE);
  wait_frames(10);
  HIDE_BKG;
  play_gameover_sound();
  fade_out();
  HIDE_SPRITES;

  // Hide the bullet sprites so that they don't appear later in the title screens.
  hide_bullet_sprites();
  update_health_bar_tiles(0);
  write_health_bar_to_window();

  // Show game over screen, then transition to the title screen.
  show_gameover_screen();
  show_title_screen(true);
}
