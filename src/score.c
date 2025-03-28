// Code that handles scores, including the high scores.
//
// There are two scores:
//   1. A point-based score that is incremented whenever the player destroys objects or clears screens.
//   2. A timer-based score that is incremented every frame.
//
// There are also high scores, which are persisted across restarts via (battery backed) external RAM
// at bank 0 (starting address: 0xA000). These scores are stored in the following format:
//   * Bytes 0 and 1: High points score
//   * Byte 2: High timer score (hours)
//   * Byte 3: High timer score (minutes)
//   * Byte 4: High timer score (seconds)

#include "score.h"

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "sound_effects.h"
#include "sprite_data.h"
#include "text_data.h"
#include "wait.h"

struct HighScore {
  uint16_t points;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

static const uint8_t ram_signature[] = {CHAR_P, CHAR_G, CHAR_I, CHAR_S};
#define HIGH_SCORE_ADDRESS (RAM_BANK0_ADDRESS + UINT8_ARRARY_SIZE(ram_signature))

static uint8_t timer_frames = 0;
static uint8_t timer_seconds = 0;
static uint8_t timer_minutes = 0;
static uint8_t timer_hours = 0;

static uint8_t score_tiles[SCREEN_TILE_WIDTH];

// Writes the timer-based high score to the window.
static void write_timer_high_score_to_window(const struct HighScore* highscore) {
  // Hours
  uint8_t* tiles = score_tiles;
  uint8_t tenths = highscore->hours / 10;
  *tiles++ = tenths + 0x01;
  *tiles++ = (highscore->hours - 10 * tenths) + 0x01;
  *tiles++ = CHAR_COLON;

  // Minutes
  tenths = highscore->minutes / 10;
  *tiles++ = tenths + 0x01;
  *tiles++ = (highscore->minutes - 10 * tenths) + 0x01;
  *tiles++ = CHAR_COLON;

  // Seconds
  tenths = highscore->seconds / 10;
  *tiles++ = tenths + 0x01;
  *tiles = (highscore->seconds - 10 * tenths) + 0x01;

  set_win_tiles(11, 0, 8, 1, score_tiles);
}

// Writes the point-based high score to the window.
static void write_point_high_score_to_window(const struct HighScore* highscore) {
  // Points
  uint8_t tenk = highscore->points / 10000;
  uint8_t onek = (highscore->points - tenk * 10000) / 1000;
  uint8_t oneh = (highscore->points - tenk * 10000 - onek * 1000) / 100;
  uint8_t tens = (highscore->points - tenk * 10000 - onek * 1000 - oneh * 100) / 10;
  uint8_t single = (highscore->points - tenk * 10000 - onek * 1000 - oneh * 100 - tens * 10);
  uint8_t* tiles = score_tiles;
  *tiles++ = tenk + 0x01;
  *tiles++ = onek + 0x01;
  *tiles++ = oneh + 0x01;
  *tiles++ = tens + 0x01;
  *tiles = single + 0x01;

  set_win_tiles(5, 0, 5, 1, score_tiles);
}

void clear_window(void) {
  memset(score_tiles, 0, UINT8_ARRARY_SIZE(score_tiles));
  set_win_tiles(0, 0, UINT8_ARRARY_SIZE(score_tiles), 1, score_tiles);
}

void clear_score_data(void) {
  struct HighScore* highscore = (struct HighScore*)HIGH_SCORE_ADDRESS;
  for (uint8_t i = 0; i < 3; ++i) {
    highscore->points = 0;
    highscore->hours = 0;
    highscore->minutes = 0;
    highscore->seconds = 0;
    ++highscore;
  }
}

void show_reward_screen(void) {
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
    set_sprite_tile(0, 0);
    move_sprite(0, 9 * TILE_SIZE_PIXELS + SCREEN_L, 13 * TILE_SIZE_PIXELS + SCREEN_T);
    set_sprite_data(1, 1, player_upgrade_sprites);

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
  }

  wait_for_keys_pressed(J_START | J_A | J_B);
  wait_for_keys_released(J_START | J_A | J_B);
  HIDE_SPRITES;
}

bool update_unlocks(void) {
  const struct HighScore* highscore = (const struct HighScore*)HIGH_SCORE_ADDRESS;
  bool changed = false;
  if (highscore->points >= HARD_MODE_UNLOCK_POINTS && !hard_mode_unlocked) {
    hard_mode_unlocked = true;
    changed = true;
  }
  ++highscore;
  if (highscore->points >= TURBO_MODE_UNLOCK_POINTS && !turbo_mode_unlocked) {
    turbo_mode_unlocked = true;
    changed = true;
  }
  ++highscore;
  if (highscore->points >= UPGRADE_SPRITE_UNLOCK_POINTS && !upgrade_sprite_unlocked) {
    upgrade_sprite_unlocked = true;
    changed = true;
  }
  return changed;
}

void init_highscores(void) {
  // Turn on RAM and read the high scores (if present).
  ENABLE_RAM;
  SWITCH_RAM(0);
  uint8_t* ram_ptr = (uint8_t*)RAM_BANK0_ADDRESS;
  bool initialized = true;
  for (uint8_t i = 0; i < UINT8_ARRARY_SIZE(ram_signature); ++i) {
    if (ram_ptr[i] != ram_signature[i]) {
      initialized = false;
      break;
    }
  }
  if (initialized) {
    return;
  }
  // High scores are *not* present. Write the signature and initialize scores with zeroes.
  for (uint8_t i = 0; i < UINT8_ARRARY_SIZE(ram_signature); ++i) {
    ram_ptr[i] = ram_signature[i];
  }
  clear_score_data();
}

bool increment_timer_score(void) {
  bool changed = false;
  ++timer_frames;
  if (timer_frames == 60) {
    ++timer_seconds;
    timer_frames = 0;
    changed = true;

    if (timer_seconds == 60) {
      ++timer_minutes;
      timer_seconds = 0;

      if (timer_minutes == 60) {
        ++timer_hours;
        timer_minutes = 0;
      }
    }
  }
  return changed;
}

void update_timer_score_tiles(void) {
  // Hours
  uint8_t* tiles = score_tiles;
  uint8_t hour_tenths = timer_hours / 10;
  *tiles++ = hour_tenths + 0x01;
  *tiles++ = (timer_hours - 10 * hour_tenths) + 0x01;
  *tiles++ = CHAR_COLON;

  // Minutes
  uint8_t min_tenths = timer_minutes / 10;
  *tiles++ = min_tenths + 0x01;
  *tiles++ = (timer_minutes - 10 * min_tenths) + 0x01;
  *tiles++ = CHAR_COLON;

  // Seconds
  uint8_t sec_tenths = timer_seconds / 10;
  *tiles++ = sec_tenths + 0x01;
  *tiles = (timer_seconds - 10 * sec_tenths) + 0x01;
}

void update_point_score_tiles(void) {
  uint8_t tenk = point_score / 10000;
  uint8_t onek = (point_score - tenk * 10000) / 1000;
  uint8_t oneh = (point_score - tenk * 10000 - onek * 1000) / 100;
  uint8_t tens = (point_score - tenk * 10000 - onek * 1000 - oneh * 100) / 10;
  uint8_t single = (point_score - tenk * 10000 - onek * 1000 - oneh * 100 - tens * 10);
  uint8_t* tiles = score_tiles;
  *tiles++ = 0;
  *tiles++ = 0;
  *tiles++ = 0;
  *tiles++ = tenk + 0x01;
  *tiles++ = onek + 0x01;
  *tiles++ = oneh + 0x01;
  *tiles++ = tens + 0x01;
  *tiles = single + 0x01;
}

void write_score_to_window(void) {
  set_win_tiles(12, 0, 8, 1, score_tiles);
}

void display_highscores(void) {
  clear_window();
  set_win_tiles(0, 0, UINT8_ARRARY_SIZE(best_text), 1, best_text);

  // Get high scores from external RAM.
  const struct HighScore* highscore = (const struct HighScore*)HIGH_SCORE_ADDRESS;
  if (game_mode == HARD) {
    ++highscore;
  } else if (game_mode == TURBO) {
    ++highscore;
    ++highscore;
  }

  write_point_high_score_to_window(highscore);
  write_timer_high_score_to_window(highscore);
}

void display_gameover_scores(void) {
  // Get high scores from external RAM.
  struct HighScore* highscore = (struct HighScore*)HIGH_SCORE_ADDRESS;
  if (game_mode == HARD) {
    ++highscore;
  } else if (game_mode == TURBO) {
    ++highscore;
    ++highscore;
  }

  uint16_t last_points = highscore->points;
  uint8_t last_hours = highscore->hours;
  uint8_t last_minutes = highscore->minutes;
  uint8_t last_seconds = highscore->seconds;

  // Display current point-based score.
  update_point_score_tiles();
  set_bkg_tiles(10, 5, 8, 1, score_tiles);

  // Display current timer-based score.
  update_timer_score_tiles();
  set_bkg_tiles(10, 6, 8, 1, score_tiles);

  bool new_record = false;
  if (point_score > last_points) {
    new_record = true;
    // Save the new record to external RAM.
    highscore->points = point_score;
    highscore->hours = timer_hours;
    highscore->minutes = timer_minutes;
    highscore->seconds = timer_seconds;
  } else if (point_score == last_points) {
    // Score is the same, check if they have achieved this score in less time than before
    uint16_t last_total_seconds = last_hours * 3600 + last_minutes * 60 + last_seconds;
    uint16_t current_total_seconds = timer_hours * 3600 + timer_minutes * 60 + timer_seconds;

    if (current_total_seconds < last_total_seconds) {
      new_record = true;
      // Update the time in external RAM.
      highscore->hours = timer_hours;
      highscore->minutes = timer_minutes;
      highscore->seconds = timer_seconds;
    }
  }

  if (new_record) {
    set_bkg_tiles(4, 11, UINT8_ARRARY_SIZE(new_record_text), 1, new_record_text);
  }

  /**
   * These have to be done at the end because we use the global values
   *    timer_hours, timer_minutes, times_seconds, and point_score
   * for updating the score_tiles. Doing this earlier overwrites them
   * and we lose the scores stored in RAM
   */
  // Display the last point-based high score.
  point_score = last_points;
  update_point_score_tiles();
  set_bkg_tiles(10, 8, 8, 1, score_tiles);
  // Display the last timer-based high score.
  timer_hours = last_hours;
  timer_minutes = last_minutes;
  timer_seconds = last_seconds;
  update_timer_score_tiles();
  set_bkg_tiles(10, 9, 8, 1, score_tiles);
}

void reset_scores(void) {
  point_score = 0;
  timer_frames = 0;
  timer_seconds = 0;
  timer_minutes = 0;
  timer_hours = 0;
  memset(score_tiles, 0, UINT8_ARRARY_SIZE(score_tiles));
  update_point_score_tiles();
}
