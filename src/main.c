#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <gb/gb.h>
#include <gbdk/bcd.h>
#include <gbdk/font.h>
#include <hUGEDriver.h>
#include <rand.h>

#include "cailan_games_logo.h"
#include "collision.h"
#include "common.h"
#include "display_effects.h"
#include "intro_scene.h"
#include "player.h"
#include "procedural_generation.h"
#include "score.h"
#include "sound_effects.h"
#include "sprites.h"
#include "weapons.h"

// Sprite data
#include "sprite_data.h"

// Tile data
#include "tile_data.h"

extern const hUGESong_t intro_song;
extern const hUGESong_t main_song;
extern const hUGESong_t main_song_fast;

enum GameMode game_mode = NORMAL;
struct Sprite player_sprite;
uint8_t collision_map[COLUMN_HEIGHT*ROW_WIDTH];
uint8_t background_map[COLUMN_HEIGHT*ROW_WIDTH];
uint16_t point_score = 0;

static bool hard_mode_unlocked = false;
static bool turbo_mode_unlocked = false;
static bool game_paused = true;
// Whether or not to show the timer-based score. If false, the points-based score is shown instead.
// Note: The value of this variable is kept between runs of the game.
static bool show_timer_score = false;

#if ENABLE_SCORING
static void increment_timer_score_isr(void) {
  if (game_paused) {
    return;
  }
  increment_timer_score();
}
#endif

void wait_frames(uint8_t n) {
  for (uint8_t i = 0; i < n; ++i) {
    vsync();
  }
}

void wait_for_keypress(uint8_t key){
  waitpadup();
  while (1){
    if (joypad() & key){
      waitpadup();
      return;
    }
    vsync();
  }
}

// Loads sprite, tile, and font data.
static void load_font(void){
  // Load font tiles to background map.
  font_init();
  font_t min_font = font_load(font_min);
  font_set(min_font);
}

static void load_sprite_data(void){
  // Load sprite data.
  set_sprite_data(0, 10, player_sprites);
  set_sprite_data(10, 9, player_shield_sprites);
  set_sprite_data(19, 3, projectile_sprites);
}

static void load_data(void) {
  // Load background tiles.
  uint8_t tile_index = WALL_BLOCK_TILE;
  set_bkg_data(tile_index, sizeof(block_tiles)/TILE_SIZE_BYTES, block_tiles);
  tile_index += sizeof(block_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(powerup_tiles)/TILE_SIZE_BYTES, powerup_tiles);
  tile_index += sizeof(powerup_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(health_bar_tiles)/TILE_SIZE_BYTES, health_bar_tiles);
  tile_index += sizeof(health_bar_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(font_extras_tiles)/TILE_SIZE_BYTES, font_extras_tiles);
  tile_index += sizeof(font_extras_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(tutorial_screen_tiles)/TILE_SIZE_BYTES, tutorial_screen_tiles);
  tile_index += sizeof(tutorial_screen_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(lock_tiles)/TILE_SIZE_BYTES, lock_tiles);
  tile_index += sizeof(lock_tiles)/TILE_SIZE_BYTES;
}

// Shows the title screen.
static void show_title_screen(uint8_t restart_song) {
  clear_window();
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT, title_screen_map);
  wait_frames(30);
  if (restart_song){
    fade_in();
    SHOW_BKG;
  }
  SHOW_WIN;
#if ENABLE_MUSIC
  if (restart_song){
    hUGE_init(&intro_song);
    play_all_channels();
    add_VBL(hUGE_dosound);
  }
#endif
  // Wait for the player to press start before going to the next screen.
  wait_for_keypress(J_START);
}

// Shows the mode selection screen and stores the mode chosen by the player in `game_mode`.
static void show_mode_selection_screen(void) {
  // Write "NORMAL".
  background_map[MAP_INDEX(6, 7)] = CHAR_N;
  background_map[MAP_INDEX(6, 8)] = CHAR_O;
  background_map[MAP_INDEX(6, 9)] = CHAR_R;
  background_map[MAP_INDEX(6, 10)] = CHAR_M;
  background_map[MAP_INDEX(6, 11)] = CHAR_A;
  background_map[MAP_INDEX(6, 12)] = CHAR_L;

  // Write "HARD".
  background_map[MAP_INDEX(8, 7)] = CHAR_H;
  background_map[MAP_INDEX(8, 8)] = CHAR_A;
  background_map[MAP_INDEX(8, 9)] = CHAR_R;
  background_map[MAP_INDEX(8, 10)] = CHAR_D;

  // Write "TURBO".
  background_map[MAP_INDEX(10, 7)] = CHAR_T;
  background_map[MAP_INDEX(10, 8)] = CHAR_U;
  background_map[MAP_INDEX(10, 9)] = CHAR_R;
  background_map[MAP_INDEX(10, 10)] = CHAR_B;
  background_map[MAP_INDEX(10, 11)] = CHAR_O;

  // Note that the previous value of `game_mode` is used here. For convenience, we want to remember
  // which mode the player selected last and default the cursor to that mode.
  uint8_t y = (game_mode == NORMAL) ? 64 : (game_mode == HARD) ? 80 : 96;

  vsync();
  set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, background_map);
  set_sprite_tile(PLAYER_SPRITE_ID, 0);
  move_sprite(PLAYER_SPRITE_ID, 32, y);
  SHOW_SPRITES;

  uint8_t prev_input = 0;
  uint8_t prev_y = 0;  // Note that y != prev_y for the first frame so that the window is updated properly.
  bool update_locks = true;
  while (true) {
    vsync();
    // Update displays.
    if (update_locks) {
      set_bkg_tile_xy(5, 8, hard_mode_unlocked ? EMPTY_TILE : LOCK_TILE);
      set_bkg_tile_xy(5, 10, turbo_mode_unlocked ? EMPTY_TILE : LOCK_TILE);
      update_locks = false;
    }
    if (y != prev_y) {
      // Update the window message.
      if (game_mode == HARD && !hard_mode_unlocked) {
        display_hardmode_unlock_msg();
      } else if (game_mode == TURBO && !turbo_mode_unlocked) {
        display_turbomode_unlock_msg();
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
        // Unlock hard and turbo modes.
        hard_mode_unlocked = true;
        turbo_mode_unlocked = true;
        update_locks = true;
        prev_y = 0;  // Updates the window.
        play_bomb_sound();
      }
    }
    else if (KEY_FIRST_PRESS(input, prev_input, J_UP)) {
      if (game_mode == NORMAL) {
        game_mode = TURBO;
        y = 96;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else if (game_mode == HARD) {
        game_mode = NORMAL;
        y = 64;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else {
        game_mode = HARD;
        y = 80;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      }
    }
    else if (KEY_FIRST_PRESS(input, prev_input, J_DOWN)) {
      if (game_mode == NORMAL) {
        game_mode = HARD;
        y = 80;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else if (game_mode == HARD) {
        game_mode = TURBO;
        y = 96;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      } else {
        game_mode = NORMAL;
        y = 64;
        move_sprite(PLAYER_SPRITE_ID, 32, y);
      }
    }
    else if (KEY_FIRST_PRESS(input, prev_input, J_START) || KEY_FIRST_PRESS(input, prev_input, J_A) || KEY_FIRST_PRESS(input, prev_input, J_B)) {
      if ((game_mode == HARD && !hard_mode_unlocked) || (game_mode == TURBO && !turbo_mode_unlocked)) {
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
        break;
      }
    }

    prev_input = input;
  }
  HIDE_SPRITES;
}

#if ENABLE_COLLISIONS
static void show_gameover_screen(void) {
  clear_window();
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT, gameover_screen_map);
  move_bkg(0, 0);

  display_gameover_scores();
  update_modes_unlocked(&hard_mode_unlocked, &turbo_mode_unlocked);

  SHOW_BKG;
  fade_in();
  wait_for_keypress(J_START | J_A | J_B);
  fade_out();
  HIDE_BKG;
}

static void handle_gameover(void) {
  game_paused = true;
  mute_all_channels();
  remove_VBL(hUGE_dosound);
  
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

  // Show gameover screen, then transition to the title screen.
  show_gameover_screen();
  show_title_screen(1);
}
#endif

void main(void) {
#if ENABLE_MUSIC
  // Load music.
  // Enable sound playback.
  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x33;

  // Mute all channels.
  mute_all_channels();

#endif
  
#if ENABLE_INTRO
  /* 
   * Display Logo 
   */
  show_logo_screen();

  /** 
   * Display intro scene
   */
  show_intro();
#endif
  /*
   * SETUP GAME ASSETS
   */
  load_font();
  load_sprite_data();
  load_data();
  // Load Window.
  move_win(7, 136);
  // Initialize high scores.
  init_highscores();
  update_modes_unlocked(&hard_mode_unlocked, &turbo_mode_unlocked);
  // Make sure ROM bank 1 is loaded (in addition to the always-loaded ROM bank 0).
  SWITCH_ROM(1);

#if ENABLE_SCORING
  // Add timer score incrementer to VBL interrupt handler.
  add_VBL(increment_timer_score_isr);
#endif

  /*
   * LOGO AND TITLE SCREENS
   */
  show_title_screen(0);

  uint8_t scroll_speed_ppf;  // How many pixels per frame to scroll the screen
  uint8_t input;  // The joypad input of this frame
  uint8_t prev_input;  // The joypad input of the previous frame
  uint8_t scroll_count;  // How many pixels have been scrolled in the current column
  uint8_t col_count;  // How many columns have been scrolled in the current screen
  bool generated_column;  // Whether or not a column was generated this frame
  uint8_t generated_column_idx;  // The index of the generated column
  bool update_window_score;  // Whether or not the score in the window needs to be updated

  while (true) {
    /*
     * Set up for main game loop.
     */
    initrand(DIV_REG);

    // Clear collision and background maps.
    for (uint16_t ii = 0; ii < COLUMN_HEIGHT*ROW_WIDTH; ++ii) {
      collision_map[ii] = 0;
      background_map[ii] = 0;
    }

    // Reset the procedural generation state.
    reset_generation_state();

    // Initialize other variables.
    input = 0;
    prev_input = 0;
    scroll_count = 0;
    col_count = 0;
    generated_column = false;
    generated_column_idx = 0;
    update_window_score = false;

    /*
     * MODE SELECTION SCREEN
     */
    show_mode_selection_screen();
    if (game_mode == NORMAL) {
      scroll_speed_ppf = SCROLL_SPEED_NORMAL;
    } else if (game_mode == HARD) {
      scroll_speed_ppf = SCROLL_SPEED_HARD;
    } else {
      scroll_speed_ppf = SCROLL_SPEED_TURBO;
    }

    // Copy tutorial screen tiles to the background.
    for (uint8_t i = 0; i < COLUMN_HEIGHT; ++i) {
      for (uint8_t j = 0; j < SCREEN_TILE_WIDTH; ++j) {
        background_map[i*ROW_WIDTH+j] = tutorial_screen_map[i*SCREEN_TILE_WIDTH+j];
      }
    }
    set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, background_map);

    // Reset the window tiles.
    clear_window();

    init_player();
    write_health_bar_to_window();
    init_weapons();
    write_bomb_icon_to_window();

#if ENABLE_MUSIC
    mute_all_channels();
#endif

    wait_frames(10);

#if ENABLE_MUSIC
    if (game_mode == NORMAL) {
      hUGE_init(&main_song);
    } else {
      hUGE_init(&main_song_fast);
    }
    play_all_channels();
#endif

    SHOW_SPRITES;
    SHOW_WIN;

    generated_column_idx = SCREEN_TILE_WIDTH - 1;
    for (uint8_t i = 0; i < ROW_WIDTH - SCREEN_TILE_WIDTH; ++i) {
      generated_column_idx = MOD32(generated_column_idx + 1);  // MOD32 is for screen wrap-around.
      generate_column(generated_column_idx);
    }
    set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, background_map);

    wait_frames(15);

    // Reset scores and window tiles.
    reset_scores();
    write_score_to_window();

    // Unpausing the game turns on the timer, so this should be done as close as possible to the
    // main game loop.
    game_paused = false;

    /*
     * MAIN GAME LOOP
     */
    while (true) {
      /*
       * HANDLE INPUT
       */
      input = joypad();

      if (KEY_PRESSED(input, J_START)) {
        game_paused = true;
#if ENABLE_MUSIC
        mute_all_channels();
#endif
        wait_for_keypress(J_START);
#if ENABLE_MUSIC
        play_all_channels();
#endif
        game_paused = false;
        continue;
      }

      if (KEY_FIRST_PRESS(input, prev_input, J_SELECT)) {
        show_timer_score = !show_timer_score;
        score_update_needed = true;
      }

      // Update the player sprite and weapons based on the input.
      move_player(input);
#if ENABLE_WEAPONS
      bool bomb_dropped = update_weapons(input, prev_input);
#endif
#if ENABLE_COLLISIONS
      bool health_changed = handle_player_collisions();
      if (health_changed) {
        if (player_sprite.health <= 0) {
          handle_gameover();
          break;
        }
      }
#endif

      prev_input = input;

      // Update scroll counts and generate a new column, if necessary.
      scroll_count += scroll_speed_ppf;
      if (scroll_count >= PIXELS_PER_COLUMN) {
        scroll_count = MOD8(scroll_count);
        generated_column_idx = MOD32(generated_column_idx + 1);  // MOD32 is for screen wrap-around.
        generate_column(generated_column_idx);
        generated_column = true;

        ++col_count;
        if (col_count == COLUMNS_PER_SCREEN) {
          col_count = 0;
          point_score += POINTS_PER_SCREEN_SCROLLED;
        }
      }

#if ENABLE_SCORING
      // Update the score tiles, if necessary.
      if (score_update_needed) {
        if (show_timer_score) {
          update_timer_score_tiles();
        } else {
          update_point_score_tiles();
        }
        score_update_needed = false;
        update_window_score = true;
      }
#endif

      // Wait for the frame to finish drawing.
      vsync();

      /*
       * COPY TO VRAM
       */
      // Scroll the background.
      scroll_bkg(scroll_speed_ppf, 0);

      // Update tiles for background objects that sprites collided with.
      if (player_sprite.collided) {
        set_bkg_tile_xy(player_sprite.collided_col, player_sprite.collided_row, background_map[MAP_INDEX_ROW_OFFSET(player_sprite.collided_row) + player_sprite.collided_col]);
        player_sprite.collided = false;
      }
      struct Sprite* b = bullet_sprites;
      for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
        if (b->collided) {
          set_bkg_tile_xy(b->collided_col, b->collided_row, background_map[MAP_INDEX_ROW_OFFSET(b->collided_row) + b->collided_col]);
          b->collided = false;
        }
        ++b;
      }

      // Update background tiles when a bomb is dropped.
      if (bomb_dropped) {
        if (bombed_col_left + BOMB_LENGTH <= ROW_WIDTH) {
          set_bkg_submap(bombed_col_left, bombed_row_top, BOMB_LENGTH, bombed_height, background_map, ROW_WIDTH);
        } else {
          // The screen wraps around to the start of the background map, so we need to take that
          // into account here by writing to VRAM in two batches.
          uint8_t first_batch_width = ROW_WIDTH - bombed_col_left;
          set_bkg_submap(bombed_col_left, bombed_row_top, first_batch_width, bombed_height, background_map, ROW_WIDTH);
          set_bkg_submap(0, bombed_row_top, BOMB_LENGTH-first_batch_width, bombed_height, background_map, ROW_WIDTH);
        }
      }

      // Update background tiles when a new column is generated.
      if (generated_column) {
        set_bkg_submap(generated_column_idx, 0, 1, COLUMN_HEIGHT, background_map, ROW_WIDTH);
        generated_column = false;
      }

      // Write health bar tiles to the window layer, if necessary.
      if (health_changed) {
        write_health_bar_to_window();
      }

#if ENABLE_SCORING
      // Write the score tiles to the window layer, if necessary.
      if (update_window_score) {
        write_score_to_window();
        update_window_score = false;
      }
#endif

      // Write bomb icon tiles to the window layer, if necessary.
      if (bomb_icon_update_needed) {
        write_bomb_icon_to_window();
        bomb_icon_update_needed = false;
      }
    }
  }
}
