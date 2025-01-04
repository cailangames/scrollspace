#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <gb/gb.h>
#include <gbdk/bcd.h>
#include <gbdk/font.h>
#include <hUGEDriver.h>
#include <rand.h>

#include "collision.h"
#include "common.h"
#include "player.h"
#include "procedural_generation.h"
#include "score.h"
#include "sound_effects.h"
#include "sprites.h"
#include "weapons.h"

// Sprite data
#include "logo_cursor_sprites.h"
#include "player_shield_sprites.h"
#include "player_sprites.h"
#include "projectiles_sprites.h"

// Tile data
#include "block_tiles.h"
#include "cailan_games_logo_map.h"
#include "cailan_games_logo_tiles.h"
#include "font_extras_tiles.h"
#include "lock_tiles.h"
#include "powerups_tiles.h"
#include "progressbar_tiles.h"
#include "title_screens.h"
#include "tutorial_screen_map.h"
#include "tutorial_screen_tiles.h"

// The following are used for performance testing of individual components.
#define ENABLE_SCORING 1
#define ENABLE_MUSIC 1
#define ENABLE_WEAPONS 1
#define ENABLE_COLLISIONS 1

extern const hUGESong_t intro_song;
extern const hUGESong_t main_song;
extern const hUGESong_t main_song_fast;

struct Sprite player_sprite;
uint8_t collision_map[COLUMN_HEIGHT*ROW_WIDTH];
uint8_t background_map[COLUMN_HEIGHT*ROW_WIDTH];
uint16_t point_score = 0;

static const uint8_t blank_win_tiles[SCREEN_TILE_WIDTH] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static bool game_paused = true;
// Whether or not to show the timer-based score. If false, the points-based score is shown instead.
// Note: The value of this variable is kept between runs of the game.
static bool show_time = true;

#if ENABLE_SCORING
static void increment_timer_score_isr(void) {
  if (game_paused) {
    return;
  }
  increment_timer_score();
}
#endif

static void wait(uint8_t num_frames) {
  for (uint8_t i = 0; i < num_frames; ++i) {
    vsync();
  }
}

static void fade_out(void) {
  for (uint8_t i = 0; i < 4; ++i) {
    switch (i) {
    case 0:
      BGP_REG = 0xE4;
      break;
    case 1:
      BGP_REG = 0xF9;
      break;
    case 2:
      BGP_REG = 0xFE;
      break;
    case 3:
      BGP_REG = 0xFF;
      break;
    }
    wait(10);
  }
}

static void fade_in(void) {
  for (uint8_t i = 0; i < 3; ++i) {
    switch (i) {
    case 0:
      BGP_REG = 0xFE;
      break;
    case 1:
      BGP_REG = 0xF9;
      break;
    case 2:
      BGP_REG = 0xE4;
      break;
    }
    wait(10);
  }
}

// Loads sprite, tile, and font data.
static void load_data(void) {
  // Load font tiles to background map.
  font_init();
  font_t min_font = font_load(font_min);
  font_set(min_font);

  // Load background tiles.
  uint8_t tile_index = MAPBLOCK_IDX;
  set_bkg_data(tile_index, sizeof(block_tiles)/TILE_SIZE_BYTES, block_tiles);
  tile_index += sizeof(block_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(powerups_tiles)/TILE_SIZE_BYTES, powerups_tiles);
  tile_index += sizeof(powerups_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(progressbar_tiles)/TILE_SIZE_BYTES, progressbar_tiles);
  tile_index += sizeof(progressbar_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(font_extras_tiles)/TILE_SIZE_BYTES, font_extras_tiles);
  tile_index += sizeof(font_extras_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(tutorial_screen_tiles)/TILE_SIZE_BYTES, tutorial_screen_tiles);
  tile_index += sizeof(tutorial_screen_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(lock_tiles)/TILE_SIZE_BYTES, lock_tiles);
  tile_index += sizeof(lock_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(cailan_games_logo_tiles)/TILE_SIZE_BYTES, cailan_games_logo_tiles);

  // Load sprite data.
  set_sprite_data(0, 10, player_data);
  set_sprite_data(10, 9, player_shield_data);
  set_sprite_data(19, 3, projectiles_data);
  set_sprite_data(22, 1, logo_cursor_data);
}

// Shows the logo screen.
static void show_logo_screen(void) {
  HIDE_BKG;
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT+1, cailan_games_logo_map);
  play_all_channels();
  wait(60);
  fade_in();
  SHOW_BKG;

  // Initialize the CBT SFX and add it to the VBL
  CBTFX_PLAY_SFX_02;
  add_VBL(CBTFX_update);

  // The cursor has two sprites: A top half and a bottom half. The below code controls both halves
  // and blinks them while the logo screen is playing.
  set_sprite_tile(0, CURSOR_SPRITE);
  set_sprite_tile(1, CURSOR_SPRITE);
  move_sprite(0, 119+8, 75+17);
  move_sprite(1, 119+8, 83+17);
  SHOW_SPRITES;
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
  remove_VBL(CBTFX_update);
  mute_all_channels();

  fade_out();
  HIDE_SPRITES;
}

// Shows the title screen.
static void show_title_screen(void) {
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT, game_titlescreen);
  wait(30);
  fade_in();
  SHOW_BKG;
  display_highscores();
  SHOW_WIN;
#if ENABLE_MUSIC
  hUGE_init(&intro_song);
  play_all_channels();
#endif
  // Wait for the player to press start before going to the next screen.
  waitpad(J_START);
  waitpadup();
}

// Shows the speed selection screen and returns the speed chosen by the player.
static uint8_t show_speed_selection_screen(void) {
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT, speed_titlescreen);
  set_sprite_tile(PLAYER_SPRITE_ID, 0);
  move_sprite(PLAYER_SPRITE_ID, 32, 72);
  SHOW_SPRITES;
  uint8_t scroll_speed = SCROLL_SPEED_NORMAL;
  uint8_t prev_input = 0;
  while (true) {
    uint8_t input = joypad();
    if (KEY_FIRST_PRESS(input, prev_input, J_UP)) {
      if (scroll_speed == SCROLL_SPEED_NORMAL) {
        move_sprite(PLAYER_SPRITE_ID, 32, 104);
        scroll_speed = SCROLL_SPEED_TURBO;
      }
      else if (scroll_speed == SCROLL_SPEED_HARD) {
        move_sprite(PLAYER_SPRITE_ID, 32, 72);
        scroll_speed = SCROLL_SPEED_NORMAL;
      }
      else {
        move_sprite(PLAYER_SPRITE_ID, 32, 88);
        scroll_speed = SCROLL_SPEED_HARD;
      }
    }
    else if (KEY_FIRST_PRESS(input, prev_input, J_DOWN)) {
      if (scroll_speed == SCROLL_SPEED_NORMAL) {
        move_sprite(PLAYER_SPRITE_ID, 32, 88);
        scroll_speed = SCROLL_SPEED_HARD;
      }
      else if (scroll_speed == SCROLL_SPEED_HARD) {
        move_sprite(PLAYER_SPRITE_ID, 32, 104);
        scroll_speed = SCROLL_SPEED_TURBO;
      }
      else {
        move_sprite(PLAYER_SPRITE_ID, 32, 72);
        scroll_speed = SCROLL_SPEED_NORMAL;
      }
    }
    else if (KEY_FIRST_PRESS(input, prev_input, J_START) || KEY_FIRST_PRESS(input, prev_input, J_A) || KEY_FIRST_PRESS(input, prev_input, J_B)) {
      break;
    }
    prev_input = input;
    vsync();
  }
  HIDE_SPRITES;
  return scroll_speed;
}

#if ENABLE_COLLISIONS
static void show_gameover_screen(void) {
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT+1, gameover_titlescreen);
  move_bkg(0, 0);

  display_gameover_scores();

  SHOW_BKG;
  fade_in();
  waitpad(J_START);
  waitpadup();
  fade_out();
  HIDE_BKG;
}

static void handle_gameover(void) {
  game_paused = true;
  mute_all_channels();
  set_sprite_tile(PLAYER_SPRITE_ID, DEATH_SPRITE);
  wait(10);
  HIDE_BKG;
  play_gameover_sound();
  fade_out();
  HIDE_SPRITES;

  // Hide the bullet sprites so that they don't appear later in the title screens.
  hide_bullet_sprites();
  update_health_bar(0);

  // Show gameover screen, then transition to the title screen.
  show_gameover_screen();
  show_title_screen();
}
#endif

void main(void) {
  /*
   * SETUP
   */
  load_data();
  // Load Window.
  move_win(7, 136);
  // Initialize high scores.
  init_highscores();
  // Make sure ROM bank 1 is loaded (in addition to the always-loaded ROM bank 0).
  SWITCH_ROM(1);

#if ENABLE_MUSIC
  // Load music.
  // Enable sound playback.
  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x33;

  // Mute all channels.
  mute_all_channels();

  // Add hUGE driver to VBL interrupt handler.
  add_VBL(hUGE_dosound);
#endif

#if ENABLE_SCORING
  // Add timer score incrementer to VBL interrupt handler.
  add_VBL(increment_timer_score_isr);
#endif

  /*
   * LOGO AND TITLE SCREENS
   */
  show_logo_screen();
  show_title_screen();

  uint8_t scroll_pixels_per_frame;  // How many pixels to scroll each frame
  uint8_t input;  // The joypad input of this frame
  uint8_t prev_input;  // The joypad input of the previous frame
  uint8_t scroll_count;  // How many pixels have been scrolled in the current column
  uint8_t col_count;  // How many columns have been scrolled in the current screen
  uint8_t left_col_index;  // The index of the leftmost column
  bool bomb_dropped;  // Whether or not the bomb was dropped this frame
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
    left_col_index = 0;
    bomb_dropped = false;
    generated_column = false;
    generated_column_idx = 0;
    update_window_score = false;

    /*
     * SPEED SELECTION SCREEN
     */
    scroll_pixels_per_frame = show_speed_selection_screen();

    // Copy tutorial screen tiles to the background.
    for (uint8_t i = 0; i < COLUMN_HEIGHT; ++i) {
      for (uint8_t j = 0; j < SCREEN_TILE_WIDTH; ++j) {
        background_map[i*ROW_WIDTH+j] = tutorial_screen_map[i*SCREEN_TILE_WIDTH+j];
      }
    }
    set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, background_map);

    // Reset the window tiles.
    set_win_tiles(0, 0, SCREEN_TILE_WIDTH, 1, blank_win_tiles);

    init_player();
    init_weapons();

#if ENABLE_MUSIC
    mute_all_channels();
#endif

    wait(10);

#if ENABLE_MUSIC
    if (scroll_pixels_per_frame == SCROLL_SPEED_NORMAL) {
      hUGE_init(&main_song);
    }
    else {
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

    wait(15);

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
        waitpadup();
        waitpad(J_START);
        waitpadup();
#if ENABLE_MUSIC
        play_all_channels();
#endif
        game_paused = false;
        continue;
      }

      if (KEY_FIRST_PRESS(input, prev_input, J_SELECT)) {
        show_time = !show_time;
        score_update_needed = true;
      }

      // Update the player sprite and weapons based on the input.
      move_player(input);
#if ENABLE_WEAPONS
      bomb_dropped = update_weapons(input, prev_input);
#endif
#if ENABLE_COLLISIONS
      if (handle_player_collisions()) {
        if (player_sprite.health <= 0) {
          handle_gameover();
          break;
        }
      }
#endif

      prev_input = input;

      // Update scroll counts and generate a new column, if necessary.
      scroll_count += scroll_pixels_per_frame;
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
        if (show_time) {
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
      scroll_bkg(scroll_pixels_per_frame, 0);

      // Update tiles for background objects that sprites collided with.
      if (player_sprite.collided) {
        set_bkg_tile_xy(player_sprite.collided_col, player_sprite.collided_row, background_map[MAP_ARRAY_INDEX_ROW_OFFSET(player_sprite.collided_row) + player_sprite.collided_col]);
        player_sprite.collided = false;
      }
      struct Sprite* b = bullet_sprites;
      for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
        if (b->collided) {
          set_bkg_tile_xy(b->collided_col, b->collided_row, background_map[MAP_ARRAY_INDEX_ROW_OFFSET(b->collided_row) + b->collided_col]);
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

#if ENABLE_SCORING
      // Write the score tiles to the window layer, if necessary.
      if (update_window_score) {
        write_score_to_window();
        update_window_score = false;
      }
#endif
    }
  }
}
