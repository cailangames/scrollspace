// Main game loop for Scroll Space

#include <gb/gb.h>
#include <gbdk/bcd.h>
#include <gbdk/font.h>
#include <hUGEDriver.h>
#include <rand.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "collision.h"
#include "common.h"
#include "display_effects.h"
#include "gameover.h"
#include "intro_scene.h"
#include "logo_screen.h"
#include "mapgen.h"
#include "player.h"
#include "score.h"
#include "selection_screen.h"
#include "songs.h"
#include "sound_effects.h"
#include "sprite_data.h"
#include "sprites.h"
#include "text_data.h"
#include "tile_data.h"
#include "title_screen.h"
#include "wait.h"
#include "weapons.h"

// Option to enable/disable intro for development.
#define ENABLE_INTRO 1

enum GameMode game_mode = NORMAL;
struct Sprite player_sprite;
uint8_t collision_map[COLUMN_HEIGHT * ROW_WIDTH];
uint8_t background_map[COLUMN_HEIGHT * ROW_WIDTH];
fixed scroll_speed;
uint16_t point_score = 0;
bool hard_mode_unlocked = false;
bool turbo_mode_unlocked = false;
bool upgrade_sprite_unlocked = false;
bool using_upgrade_sprite = false;

static bool rand_initialized = false;
static bool game_paused = true;
// Whether or not to show the timer-based score. If false, the point-based score is shown instead.
// Note: The value of this variable is kept between runs of the game.
static bool show_timer_score = false;
static bool timer_score_changed = false;

// Increments the timer score. To be called during an ISR (interrupt service routine).
static void increment_timer_score_isr(void) {
  if (game_paused) {
    return;
  }
  timer_score_changed = increment_timer_score();
}

// Loads the title screen tiles.
static void load_title_screen(void) {
  set_bkg_data(TITLE_SCREEN_OFFSET, TILE_COUNT(title_screen_tiles), title_screen_tiles);
}

// Loads font data.
static void load_font(void) {
  // Load font tiles to background map.
  font_init();
  font_t min_font = font_load(font_min);
  font_set(min_font);
}

// Loads sprite data.
// Note: The player's ship sprites are loaded elsewhere.
static void load_sprite_data(void) {
  set_sprite_data(BULLET_SPRITE, TILE_COUNT(projectiles_sprites), projectiles_sprites);
}

// Loads background tile data.
static void load_tile_data(void) {
  uint8_t tile_index = WALL_BLOCK_TILE;
  set_bkg_data(tile_index, TILE_COUNT(block_tiles), block_tiles);
  tile_index += TILE_COUNT(block_tiles);
  set_bkg_data(tile_index, TILE_COUNT(powerup_tiles), powerup_tiles);
  tile_index += TILE_COUNT(powerup_tiles);
  set_bkg_data(tile_index, TILE_COUNT(health_bar_tiles), health_bar_tiles);
  tile_index += TILE_COUNT(health_bar_tiles);
  set_bkg_data(tile_index, TILE_COUNT(font_extras_tiles), font_extras_tiles);
  tile_index += TILE_COUNT(font_extras_tiles);
  set_bkg_data(tile_index, TILE_COUNT(tutorial_screen_tiles), tutorial_screen_tiles);
  tile_index += TILE_COUNT(tutorial_screen_tiles);
  set_bkg_data(tile_index, TILE_COUNT(lock_tiles), lock_tiles);
  tile_index += TILE_COUNT(lock_tiles);
}

// Incrementally increases the difficulty of the game, e.g. by increasing the scroll speed.
static void increase_difficulty(void) {
  scroll_speed.w += SCROLL_SPEED_INCREASE;
  player_sprite.speed.w += PLAYER_SPEED_INCREASE;
  if (game_mode == NORMAL) {
    if (scroll_speed.w > SCROLL_SPEED_HARD) {
      scroll_speed.w = SCROLL_SPEED_HARD;
    }
    if ((!upgrade_sprite_unlocked) && (player_sprite.speed.w > PLAYER_SPEED_HARD)) {
      player_sprite.speed.w = PLAYER_SPEED_HARD;
    }
    else if ((upgrade_sprite_unlocked) && (player_sprite.speed.w > PLAYER_SPEED_HARD_XENO)) {
      player_sprite.speed.w = PLAYER_SPEED_HARD_XENO;
    }
  } else if (game_mode == HARD) {
    if (scroll_speed.w > SCROLL_SPEED_TURBO) {
      scroll_speed.w = SCROLL_SPEED_TURBO;
    }
    if ((!upgrade_sprite_unlocked) && (player_sprite.speed.w > PLAYER_SPEED_TURBO)) {
      player_sprite.speed.w = PLAYER_SPEED_TURBO;
    }
    else if ((upgrade_sprite_unlocked) && (player_sprite.speed.w > PLAYER_SPEED_TURBO_XENO)) {
      player_sprite.speed.w = PLAYER_SPEED_TURBO_XENO;
    }
  } else {
    if (scroll_speed.w > SCROLL_SPEED_TURBO_MAX) {
      scroll_speed.w = SCROLL_SPEED_TURBO_MAX;
    }
    if ((!upgrade_sprite_unlocked) && (player_sprite.speed.w > PLAYER_SPEED_TURBO_MAX)) {
      player_sprite.speed.w = PLAYER_SPEED_TURBO_MAX;
    }
    else if ((upgrade_sprite_unlocked) && (player_sprite.speed.w > PLAYER_SPEED_TURBO_MAX_XENO)) {
      player_sprite.speed.w = PLAYER_SPEED_TURBO_MAX_XENO;
    }
  }
  decrease_pickup_probability();
}

void main(void) {
  // Enable sound playback.
  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x33;
  // Mute all channels.
  mute_all_channels();

#if ENABLE_INTRO
  /*
   * LOGO SCREEN
   */
  // The logo screen code is in ROM bank 2.
  SWITCH_ROM(2);
  show_logo_screen();

  /*
   * INTRO SCENE
   */
  // From here on, we only use ROM banks 0 and 1.
  SWITCH_ROM(1);
  show_intro_scene();
#endif

  /*
   * SETUP GAME ASSETS
   */
  load_font();
  load_sprite_data();
  load_tile_data();
  load_title_screen();
  // Load Window.
  move_win(7, 136);
  // Initialize high scores.
  init_highscores();
  update_unlocks();
  if (upgrade_sprite_unlocked) {
    using_upgrade_sprite = true;
  }
  // Set palette 1 colors (for bullets).
  OBP1_REG = 0xE4;  // 0b1110 0100 - black, dark gray, light gray, white

  // Add timer score incrementer to VBL interrupt handler.
  add_VBL(increment_timer_score_isr);

  /*
   * TITLE SCREEN
   */
  show_title_screen(false);

  // Get the high byte of the random seed right after the user has pressed START to continue past
  // the title screen.
  uint16_t rand_seed = DIV_REG;
  rand_seed <<= 8;

  fixed screen_pos_x;             // The x position of the screen (high 8 bits: pixels, low 8 bits: subpixels)
  uint8_t input;                  // The joypad input of this frame
  uint8_t prev_input;             // The joypad input of the previous frame
  uint8_t prev_left_column;       // The left-most column on the screen in the previous frame. Used to track when the background has scrolled a full column's worth of pixels.
  uint8_t column_count;           // How many columns have been scrolled in the current screen
  uint16_t difficulty_countdown;  // A count-down of the number of frames before a difficulty increase
  bool generated_column;          // Whether or not a column was generated this frame
  uint8_t generated_column_idx;   // The index of the generated column
  bool update_window_score;       // Whether or not the score in the window needs to be updated
  uint16_t prev_point_score;      // The `point_score` of the previous frame. Used to determine whether or not to update the window score tiles.

  while (true) {
    /*
     * Set up for main game loop.
     */
    // Initialize variables.
    screen_pos_x.w = 0x0000;
    input = 0;
    prev_input = 0;
    prev_left_column = 0;
    column_count = 0;
    difficulty_countdown = DIFFICULTY_INCREASE_CYCLE;
    generated_column = false;
    generated_column_idx = ROW_WIDTH - 1;
    update_window_score = false;
    prev_point_score = UINT16_MAX;

    /*
     * MODE SELECTION SCREEN
     */
    show_mode_selection_screen();
    if (game_mode == NORMAL) {
      scroll_speed.w = SCROLL_SPEED_NORMAL;
    } else if (game_mode == HARD) {
      scroll_speed.w = SCROLL_SPEED_HARD;
    } else if (game_mode == TURBO) {
      scroll_speed.w = SCROLL_SPEED_TURBO;
    } else {
      show_title_screen(false);
      continue;
    }

    fade_out();
    // initrand() needs to be called with a 16-bit random seed. We produce this seed by polling the
    // 8-bit DIV_REG register at two different, non-deterministic times: Once after the title screen,
    // and once after the mode selection screen. Both of these are non-deterministic because they
    // require the user to press a button in order to continue.
    if (!rand_initialized) {
      rand_seed |= DIV_REG;
      initrand(rand_seed);
      rand_initialized = true;
    }

    // Reset the procedural generation state. (Needs to be called after initrand() has been called.)
    reset_generation_state();

    // Generate the tutorial of the game map.
    generate_tutorial();

    // Reset the window tiles.
    clear_window();

    init_player();
    write_health_bar_to_window();
    init_weapons();
    update_bomb_ready_icon();

    mute_all_channels();

    wait_frames(10);

    const hUGESong_t* song = (game_mode == NORMAL) ? &main_song : &main_song_fast;
    __critical {
      hUGE_init(song);
    }
    play_all_channels();

    wait_frames(15);

    // Reset scores and window tiles.
    reset_scores();
    write_score_to_window();

    SHOW_SPRITES;
    SHOW_WIN;
    fade_in();

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
        // Pause the game.
        game_paused = true;
        mute_all_channels();
        wait_for_keys_released(J_START);
        wait_for_keys_pressed(J_START);
        wait_for_keys_released(J_START);
        play_all_channels();
        game_paused = false;
        continue;
      }

      if (KEY_FIRST_PRESS(input, prev_input, J_SELECT)) {
        // Toggle between point-based score and timer-based score.
        show_timer_score = !show_timer_score;
        // Make sure that the score in the window is updated.
        timer_score_changed = true;
        prev_point_score = point_score - 1;
      }

      // Update the player sprite and weapons based on the input.
      move_player(input);
      update_weapons(input, prev_input);
      bool health_changed = handle_player_collisions();
      if (health_changed) {
        if (player_sprite.health <= 0) {
          game_paused = true;
          handle_gameover();
          break;
        }
      }

      prev_input = input;

      // Update screen position and generate a new column, if necessary.
      screen_pos_x.w += scroll_speed.w;
      uint8_t left_column = screen_pos_x.h >> 3;
      if (left_column != prev_left_column) {
        prev_left_column = left_column;
        generated_column_idx = MOD32(generated_column_idx + 1);  // MOD32 is for screen wrap-around.
        generate_column(generated_column_idx);
        generated_column = true;

        ++column_count;
        if (column_count == COLUMNS_PER_SCREEN) {
          column_count = 0;
          point_score += POINTS_PER_SCREEN_SCROLLED;
        }
      }

      // Increase difficulty, if necessary.
      --difficulty_countdown;
      if (difficulty_countdown == 0) {
        increase_difficulty();
        difficulty_countdown = DIFFICULTY_INCREASE_CYCLE;
      }

      // Update the score tiles, if necessary.
      if (show_timer_score) {
        if (timer_score_changed) {
          update_timer_score_tiles();
          update_window_score = true;
          timer_score_changed = false;
        }
      } else {
        if (point_score != prev_point_score) {
          update_point_score_tiles();
          update_window_score = true;
          prev_point_score = point_score;
        }
      }

      // Wait for the frame to finish drawing.
      vsync();

      /*
       * COPY TO VRAM
       */
      // Scroll the background.
      SCX_REG = screen_pos_x.h;

      // Update tiles for background objects that sprites collided with.
      if (player_sprite.collided) {
        set_bkg_tile_xy(player_sprite.collided_col, player_sprite.collided_row, background_map[MAP_INDEX(player_sprite.collided_row, player_sprite.collided_col)]);
        player_sprite.collided = false;
      }
      update_tiles_hit_by_weapons();

      // Update background tiles when a new column is generated.
      if (generated_column) {
        set_bkg_submap(generated_column_idx, 0, 1, COLUMN_HEIGHT, background_map, ROW_WIDTH);
        generated_column = false;
      }

      // Write health bar tiles to the window layer, if necessary.
      if (health_changed) {
        write_health_bar_to_window();
      }

      // Write the score tiles to the window layer, if necessary.
      if (update_window_score) {
        write_score_to_window();
        update_window_score = false;
      }

      // Update the "bomb ready" icon in the window layer, if necessary.
      update_bomb_ready_icon();
    }
  }
}
