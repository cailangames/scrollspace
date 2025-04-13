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
  // Prep the bakground map for reward screen scroll
  memset(background_map, WALL_BLOCK_TILE, 1);
  memset(background_map + 1, EMPTY_TILE, SCREEN_TILE_HEIGHT - 2);
  memset(background_map + SCREEN_TILE_HEIGHT - 1, WALL_BLOCK_TILE, 1);

  // Scroll into reward screen
  for (uint16_t i = 0; i < 19 * 8; ++i) {
    scroll_bkg(1, 0);
    if (MOD8(i) == 0) {
      set_bkg_tiles(i >> 3, 0, 1, SCREEN_TILE_HEIGHT, background_map);
    }
    vsync();
  }

  memset(background_map, WALL_BLOCK_TILE, SCREEN_TILE_HEIGHT);
  set_bkg_tiles(7, 0, 1, SCREEN_TILE_HEIGHT, background_map);
  vsync();

  for (uint8_t i = 0; i < 8; ++i) {
    scroll_bkg(1, 0);
    vsync();
  }

  // Write "CONGRATULATIONS!"
  wait_frames(20);
  set_bkg_tiles(22, 6, UINT8_ARRARY_SIZE(congratulations_text), 1, congratulations_text);
  play_collision_sound();
  wait_frames(60);

  // Write "YOU UNLOCKED"
  set_bkg_tiles(24, 8, UINT8_ARRARY_SIZE(you_unlocked_text), 1, you_unlocked_text);
  play_collision_sound();
  wait_frames(60);

  // Show the reward.
  const uint8_t* text;
  uint8_t size;
  uint8_t text_start_x = 25;
  uint8_t text_start_y = 11;
  if (game_mode == TURBO) {
    // For turbo mode's reward, show the ship being upgraded.
    set_sprite_data(0, 1, player_sprites);
    set_sprite_data(1, 1, player_upgrade_sprites);
    set_sprite_tile(0, 0);
    move_sprite(0, 9 * TILE_SIZE_PIXELS + SCREEN_L + 4, 10 * TILE_SIZE_PIXELS + SCREEN_T + 4);
    vsync();
    SHOW_SPRITES;
    wait_frames(60);
    play_shield_sound();
    for (uint8_t i = 0; i < 18; ++i) {
      wait_frames(5);
      set_sprite_tile(0, 1);
      wait_frames(5);
      set_sprite_tile(0, 0);
    }
    set_sprite_tile(0, 1);
    using_upgrade_sprite = true;

    text = the_xenobird_text;
    size = UINT8_ARRARY_SIZE(the_xenobird_text);
    text_start_x = 24;
    text_start_y = 13;
  } else if (game_mode == NORMAL) {
    text = hard_mode_text;
    size = UINT8_ARRARY_SIZE(hard_mode_text);
  } else {
    text = turbo_mode_text;
    size = UINT8_ARRARY_SIZE(turbo_mode_text);
  }
  set_bkg_tiles(text_start_x, text_start_y, size, 1, text);
  play_health_sound();
  wait_frames(120);

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
  memset(background_map, WALL_BLOCK_TILE, ROW_WIDTH);
  set_bkg_tiles(0, 0, ROW_WIDTH, 1, background_map);
  set_bkg_tiles(0, 17, ROW_WIDTH, 1, background_map);
  memset(background_map, EMPTY_TILE, ROW_WIDTH * (SCREEN_TILE_HEIGHT - 2));
  set_bkg_tiles(0, 1, ROW_WIDTH, SCREEN_TILE_HEIGHT - 2, background_map);

  // Fill in the game over text.
  const uint8_t* text;
  uint8_t size;
  switch (game_mode) {
    case NORMAL:
      text = normal_mode_text;
      size = UINT8_ARRARY_SIZE(normal_mode_text);
      break;
    case HARD:
      text = hard_mode_text;
      size = UINT8_ARRARY_SIZE(hard_mode_text);
      break;
    default:
      text = turbo_mode_text;
      size = UINT8_ARRARY_SIZE(turbo_mode_text);
      break;
  }
  set_bkg_tiles(5, 2, UINT8_ARRARY_SIZE(game_over_text), 1, game_over_text);
  set_bkg_tiles(5, 4, size, 1, text);
  set_bkg_tiles(2, 6, UINT8_ARRARY_SIZE(score_text), 1, score_text);
  set_bkg_tiles(3, 9, UINT8_ARRARY_SIZE(best_text), 1, best_text);
  set_bkg_tiles(1, 13, UINT8_ARRARY_SIZE(tip_text), 1, tip_text);
  set_bkg_tiles(2, 14, TIP_WRAP_LENGTH, 1, tip);
  set_bkg_tiles(2, 15, TIP_MESSAGE_LENGTH - TIP_WRAP_LENGTH, 1, tip + TIP_WRAP_LENGTH);
  move_bkg(0, 0);

  // Show the player's ship sprite.
  set_sprite_tile(PLAYER_SPRITE_ID, PLAYER_BASE_SPRITE);
  move_sprite(PLAYER_SPRITE_ID, 3 * TILE_SIZE_PIXELS + SCREEN_L, 4 * TILE_SIZE_PIXELS + SCREEN_T);

  display_gameover_scores();

  vsync();
  SHOW_BKG;
  SHOW_SPRITES;
  fade_in();
  wait_for_keys_pressed(J_START | J_A | J_B);
  wait_for_keys_released(J_START | J_A | J_B);
  HIDE_SPRITES;

  bool new_unlock = update_unlocks();
  if (new_unlock) {
    show_reward_screen();
  }

  fade_out();
  move_bkg(0, 0);
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
