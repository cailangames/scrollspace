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
#include "sprite_data.h"
#include "text_data.h"
#include "title_screen.h"
#include "wait.h"
#include "weapons.h"

// Shows the screen for giving the player a reward when they've earned a high enough score.
static void show_reward_screen(void) {
  // Clear the background map and add borders (top, bottom, right).
  memset(background_map, WALL_BLOCK_TILE, SCREEN_TILE_WIDTH);
  memset(background_map + SCREEN_TILE_WIDTH, EMPTY_TILE, SCREEN_TILE_WIDTH * (SCREEN_TILE_HEIGHT - 2));
  memset(background_map + (SCREEN_TILE_WIDTH * (SCREEN_TILE_HEIGHT - 1)), WALL_BLOCK_TILE, SCREEN_TILE_WIDTH);
  uint8_t* bkg_map = background_map + (SCREEN_TILE_WIDTH * 2) - 1;
  for (uint8_t i = 0; i < SCREEN_TILE_HEIGHT - 2; ++i) {
    *bkg_map = WALL_BLOCK_TILE;
    bkg_map += SCREEN_TILE_WIDTH;
  }
  vsync();
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, background_map);

  // Write "CONGRATULATIONS!"
  set_bkg_tiles(2, 6, UINT8_ARRARY_SIZE(congratulations_text), 1, congratulations_text);
  play_collision_sound();
  wait_frames(60);

  // Write "YOU UNLOCKED"
  set_bkg_tiles(4, 8, UINT8_ARRARY_SIZE(you_unlocked_text), 1, you_unlocked_text);
  play_collision_sound();
  wait_frames(60);

  // Write the reward's name.
  if (game_mode == NORMAL) {
    // "HARD MODE"
    set_bkg_tiles(5, 11, UINT8_ARRARY_SIZE(hard_mode_text), 1, hard_mode_text);
  } else if (game_mode == HARD) {
    // "TURBO MODE"
    set_bkg_tiles(4, 11, UINT8_ARRARY_SIZE(turbo_mode_text), 1, turbo_mode_text);
  } else {
    // "NEW SHIP"
    set_bkg_tiles(6, 11, UINT8_ARRARY_SIZE(new_ship_text), 1, new_ship_text);
  }
  play_health_sound();

  if (game_mode == TURBO) {
    set_sprite_data(0, 1, player_sprites);
    set_sprite_data(1, 1, player_upgrade_sprites);
    set_sprite_tile(0, 0);
    move_sprite(0, 9 * TILE_SIZE_PIXELS + SCREEN_L, 13 * TILE_SIZE_PIXELS + SCREEN_T);

    wait_frames(60);
    SHOW_SPRITES;
    play_gameover_sound();
    for (uint8_t i = 0; i < 5; ++i) {
      wait_frames(10);
      set_sprite_tile(0, 1);
      wait_frames(10);
      set_sprite_tile(0, 0);
    }
    play_health_sound();
    set_sprite_tile(0, 1);
    using_upgrade_sprite = true;
  }

  wait_for_keys_pressed(J_START | J_A | J_B);
  wait_for_keys_released(J_START | J_A | J_B);
  HIDE_SPRITES;
}

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
