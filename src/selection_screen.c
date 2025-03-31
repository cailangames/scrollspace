#include "selection_screen.h"

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "score.h"
#include "sound_effects.h"
#include "sprite_data.h"
#include "text_data.h"
#include "wait.h"

// Displays the given message in the window.
static void display_window_message(const uint8_t* text) {
  set_win_tiles(0, 0, SCREEN_TILE_WIDTH, 1, text);
}

// Displays a confirmation prompt to the player to confirm whether or not they want to perform an
// action. Returns true if the player confirmed the action, false otherwise.
static bool confirm_action(void) {
  vsync();
  set_win_tiles(0, 0, UINT8_ARRARY_SIZE(confirmation_prompt_text), 1, confirmation_prompt_text);
  set_win_tiles(0, 1, UINT8_ARRARY_SIZE(yes_or_no_text), 1, yes_or_no_text);
  move_win(7, 128);  // Need 2 rows in the window for the confirmation prompt.
  wait_for_keys_released(J_START | J_A);

  bool yes_highlighted = false;
  bool update_cursor = false;
  uint8_t prev_input = 0;
  while (true) {
    vsync();
    if (update_cursor) {
      if (yes_highlighted) {
        set_win_tile_xy(3, 1, CHAR_CURSOR);
        set_win_tile_xy(12, 1, EMPTY_TILE);
      } else {
        set_win_tile_xy(3, 1, EMPTY_TILE);
        set_win_tile_xy(12, 1, CHAR_CURSOR);
      }
      update_cursor = false;
    }

    uint8_t input = joypad();
    if (KEY_FIRST_PRESS(input, prev_input, J_RIGHT) && yes_highlighted) {
      yes_highlighted = false;
      update_cursor = true;
    } else if (KEY_FIRST_PRESS(input, prev_input, J_LEFT) && !yes_highlighted) {
      yes_highlighted = true;
      update_cursor = true;
    } else if (KEY_FIRST_PRESS(input, prev_input, J_START) || KEY_FIRST_PRESS(input, prev_input, J_A)) {
      move_win(7, 136);
      return yes_highlighted;
    }
    prev_input = input;
  }
}

void show_mode_selection_screen(void) {
  // Add walls at the top and bottom of the screen. The very bottom row is under the window, so we
  // clear it out too.
  memset(background_map, EMPTY_TILE, SCREEN_TILE_WIDTH * SCREEN_TILE_HEIGHT);
  memset(background_map, WALL_BLOCK_TILE, SCREEN_TILE_WIDTH);
  memset(background_map + (SCREEN_TILE_WIDTH * (COLUMN_HEIGHT - 1)), WALL_BLOCK_TILE, SCREEN_TILE_WIDTH);

  // Draw the background tiles first.
  SHOW_WIN;  // Show the window here to cover the bottom-most row.
  vsync();
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, background_map);
  set_bkg_tiles(7, 3, UINT8_ARRARY_SIZE(normal_text), 1, normal_text);
  set_bkg_tiles(7, 6, UINT8_ARRARY_SIZE(hard_text), 1, hard_text);
  set_bkg_tiles(7, 9, UINT8_ARRARY_SIZE(turbo_text), 1, turbo_text);
  set_bkg_tiles(7, 14, UINT8_ARRARY_SIZE(clear_data_text), 1, clear_data_text);

  // Then draw the player sprite.
  if (upgrade_sprite_unlocked) {
    set_sprite_data(PLAYER_BASE_SPRITE, TILE_COUNT(player_upgrade_sprites), player_upgrade_sprites);
    set_sprite_data(PLAYER_BASE_SPRITE + PLAYER_SHIELD_SPRITES_OFFSET, TILE_COUNT(player_upgrade_shield_sprites), player_upgrade_shield_sprites);
  }
  if (game_mode == TITLE_SCREEN) {
    game_mode = NORMAL;
  }
  // Note that the previous value of `game_mode` is used here. For convenience, we want to remember
  // which mode the player selected last and default the cursor to that mode.
  uint8_t y = (game_mode == NORMAL) ? 40 : (game_mode == HARD) ? 64
                                                               : 88;
  set_sprite_tile(PLAYER_SPRITE_ID, PLAYER_BASE_SPRITE);
  move_sprite(PLAYER_SPRITE_ID, 32, y);
  SHOW_SPRITES;

  uint8_t prev_input = 0;
  uint8_t prev_y = 0;  // Note that y != prev_y for the first frame so that the window is updated properly.
  bool update_locks = true;
  while (true) {
    vsync();
    // Update displays.
    if (update_locks) {
      set_bkg_tile_xy(5, 6, hard_mode_unlocked ? EMPTY_TILE : LOCK_TILE);
      set_bkg_tile_xy(5, 9, turbo_mode_unlocked ? EMPTY_TILE : LOCK_TILE);
      update_locks = false;
    }
    if (y != prev_y) {
      // Update the window message.
      if (game_mode == CLEAR_DATA) {
        display_window_message(clear_data_description_text);
      } else if (game_mode == HARD && !hard_mode_unlocked) {
        display_window_message(unlock_hard_mode_text);
      } else if (game_mode == TURBO && !turbo_mode_unlocked) {
        display_window_message(unlock_turbo_mode_text);
      } else {
        display_highscores();
      }
      prev_y = y;
    }

    // Handle input.
    uint8_t input = joypad();
    if (KEY_PRESSED(input, J_SELECT)) {
      // It's cheatin' time!
      if (KEY_PRESSED(input, J_UP) && KEY_PRESSED(input, J_A) && (!hard_mode_unlocked || !turbo_mode_unlocked)) {
        // Unlock hard and turbo modes. Note that the upgrade sprite is intentionally *not*
        // unlocked by this.
        hard_mode_unlocked = true;
        turbo_mode_unlocked = true;
        update_locks = true;
        prev_y = 0;  // Updates the window.
        play_bomb_sound();
      }
    } else if (KEY_FIRST_PRESS(input, prev_input, J_UP)) {
      if (game_mode == HARD) {
        game_mode = NORMAL;
        y = 40;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else if (game_mode == TURBO) {
        game_mode = HARD;
        y = 64;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else if (game_mode == CLEAR_DATA) {
        game_mode = TURBO;
        y = 88;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      }
      play_mode_selection_sound();
    } else if (KEY_FIRST_PRESS(input, prev_input, J_DOWN)) {
      if (game_mode == NORMAL) {
        game_mode = HARD;
        y = 64;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else if (game_mode == HARD) {
        game_mode = TURBO;
        y = 88;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else if (game_mode == TURBO) {
        game_mode = CLEAR_DATA;
        y = 128;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      }
      play_mode_selection_sound();
    } else if (KEY_FIRST_PRESS(input, prev_input, J_START) || KEY_FIRST_PRESS(input, prev_input, J_A)) {
      if (game_mode == CLEAR_DATA) {
        if (confirm_action()) {
          play_bomb_sound();
          clear_score_data();
          set_sprite_data(PLAYER_BASE_SPRITE, TILE_COUNT(player_sprites), player_sprites);
          set_sprite_data(PLAYER_BASE_SPRITE + PLAYER_SHIELD_SPRITES_OFFSET, TILE_COUNT(player_shield_sprites), player_shield_sprites);
          hard_mode_unlocked = false;
          turbo_mode_unlocked = false;
          upgrade_sprite_unlocked = false;
          update_locks = true;
        }
        prev_y = 0;                    // Updates the window.
        prev_input = (J_START | J_A);  // Makes sure these keys are released before triggering this again.
        continue;
      } else if ((game_mode == HARD && !hard_mode_unlocked) || (game_mode == TURBO && !turbo_mode_unlocked)) {
        // Mode not unlocked yet. Let the player know by shaking the ship and playing a sound.
        play_collision_sound();
        for (uint8_t i = 0; i < 2; ++i) {
          move_sprite(PLAYER_SPRITE_ID, 34, y);
          wait_frames(2);
          move_sprite(PLAYER_SPRITE_ID, 32, y);
          wait_frames(2);
          move_sprite(PLAYER_SPRITE_ID, 30, y);
          wait_frames(2);
          move_sprite(PLAYER_SPRITE_ID, 32, y);
          wait_frames(2);
        }
      } else {
        play_flyaway_sound();
        // Make sprite fly off the screen
        for (uint8_t i = 0; i < 144 / 8; i++) {
          scroll_sprite(PLAYER_SPRITE_ID, 8, 0);
          vsync();
        }
        break;
      }
    } else if (KEY_FIRST_PRESS(input, prev_input, J_B)) {
      game_mode = TITLE_SCREEN;
      break;
    }

    prev_input = input;
  }
  HIDE_SPRITES;
}
