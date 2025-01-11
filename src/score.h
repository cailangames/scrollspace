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

#ifndef _SCORE_H_
#define _SCORE_H_

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

#include "common.h"

struct HighScore {
  uint16_t points;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

static const uint8_t ram_signature[] = {CHAR_P, CHAR_G, CHAR_I, CHAR_S};
#define RAM_SIGNATURE_LENGTH (sizeof(ram_signature) / sizeof(uint8_t))
#define HIGH_SCORE_ADDRESS (RAM_BANK0_ADDRESS + RAM_SIGNATURE_LENGTH)

static uint8_t timer_frames = 0;
static uint8_t timer_seconds = 0;
static uint8_t timer_minutes = 0;
static uint8_t timer_hours = 0;
static bool score_update_needed = false;

static uint8_t score_tiles[8] = {0,0,0,0,0,0,0,0};
static uint8_t high_score_tiles[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static void highscores2tiles(void) {
  // Get high scores from external RAM.
  const struct HighScore* highscore = (const struct HighScore*)HIGH_SCORE_ADDRESS;
  if (game_mode == HARD) {
    ++highscore;
  } else if (game_mode == TURBO) {
    ++highscore;
    ++highscore;
  }

  // Write out "BEST ".
  high_score_tiles[0] = CHAR_B;
  high_score_tiles[1] = CHAR_E;
  high_score_tiles[2] = CHAR_S;
  high_score_tiles[3] = CHAR_T;
  high_score_tiles[4] = 0;

  // Seconds
  uint8_t tenths = highscore->seconds/10;
  high_score_tiles[12] = (highscore->seconds - 10*tenths) + 0x01;
  high_score_tiles[11] = tenths + 0x01;
  high_score_tiles[10] = COLON_FONT_IDX;

  // Minutes
  tenths = highscore->minutes/10;
  high_score_tiles[9] = (highscore->minutes - 10*tenths) + 0x01;
  high_score_tiles[8] = tenths + 0x01;
  high_score_tiles[7] = COLON_FONT_IDX;

  // Hours
  tenths = highscore->hours/10;
  high_score_tiles[6] = (highscore->hours - 10*tenths) + 0x01;
  high_score_tiles[5] = tenths + 0x01;

  // Points
  uint8_t tenk = highscore->points/10000;
  uint8_t onek = (highscore->points - tenk*10000) / 1000;
  uint8_t oneh = (highscore->points - tenk*10000 - onek*1000) / 100;
  uint8_t tens = (highscore->points - tenk*10000 - onek*1000 - oneh*100) / 10;
  uint8_t single = (highscore->points - tenk*10000 - onek*1000 - oneh*100 - tens*10);
  high_score_tiles[13] = 0;
  high_score_tiles[14] = tenk + 0x01;
  high_score_tiles[15] = onek + 0x01;
  high_score_tiles[16] = oneh + 0x01;
  high_score_tiles[17] = tens + 0x01;
  high_score_tiles[18] = single + 0x01;
  high_score_tiles[19] = 0;
}

// Checks if the high scores meet the thresholds for unlocking the harder game modes, and sets the
// output bool pointers accordingly.
void update_modes_unlocked(bool* hard_mode_unlocked, bool* turbo_mode_unlocked) {
  const struct HighScore* highscore = (const struct HighScore*)HIGH_SCORE_ADDRESS;
  *hard_mode_unlocked = (highscore->points >= HARD_MODE_UNLOCK_POINTS);
  ++highscore;
  *turbo_mode_unlocked = (highscore->points >= TURBO_MODE_UNLOCK_POINTS);
}

// Initializes the external RAM and reads each mode's high scores from it. If no high scores are
// found, then they're initialized with zeroes.
void init_highscores(void) {
  // Turn on RAM and read the high scores (if present).
  ENABLE_RAM;
  SWITCH_RAM(0);
  uint8_t* ram_ptr = (uint8_t*)RAM_BANK0_ADDRESS;
  bool initialized = true;
  for (uint8_t i = 0; i < RAM_SIGNATURE_LENGTH; ++i) {
    if (ram_ptr[i] != ram_signature[i]) {
      initialized = false;
    }
  }
  if (initialized) {
    return;
  }
  // High scores are *not* present. Initialize them with zeroes and write the signature.
  for (uint8_t i = 0; i < RAM_SIGNATURE_LENGTH; ++i) {
    ram_ptr[i] = ram_signature[i];
  }
  struct HighScore* highscore = (struct HighScore*)HIGH_SCORE_ADDRESS;
  for (uint8_t i = 0; i < 3; ++i) {
    highscore->points = 0;
    highscore->hours = 0;
    highscore->minutes = 0;
    highscore->seconds = 0;
    ++highscore;
  }
}

// Increments the timer-based score.
void increment_timer_score(void) {
  ++timer_frames;
  if (timer_frames == 60) {
    ++timer_seconds;
    timer_frames = 0;
    score_update_needed = true;

    if (timer_seconds == 60) {
      ++timer_minutes;
      timer_seconds = 0;

      if (timer_minutes == 60) {
        ++timer_hours;
        timer_minutes = 0;
      }
    }
  }
}

// Updates the score tiles with the timer-based score. Note: This does *not* update the window;
// call `write_score_to_window()` for that.
void update_timer_score_tiles(void) {
  // Seconds
  uint8_t sec_tenths = timer_seconds/10;
  score_tiles[7] = (timer_seconds - 10*sec_tenths) + 0x01;
  score_tiles[6] = sec_tenths + 0x01;
  score_tiles[5] = COLON_FONT_IDX;

  // Minutes
  uint8_t min_tenths = timer_minutes/10;
  score_tiles[4] = (timer_minutes - 10*min_tenths) + 0x01;
  score_tiles[3] = min_tenths + 0x01;
  score_tiles[2] = COLON_FONT_IDX;

  // Hours
  uint8_t hour_tenths = timer_hours/10;
  score_tiles[1] = (timer_hours - 10*hour_tenths) + 0x01;
  score_tiles[0] = hour_tenths + 0x01;
}

// Updates the score tiles with the point-based score. Note: This does *not* update the window;
// call `write_score_to_window()` for that.
void update_point_score_tiles(void) {
  uint8_t tenk = point_score/10000;
  uint8_t onek = (point_score - tenk*10000) / 1000;
  uint8_t oneh = (point_score - tenk*10000 - onek*1000) / 100;
  uint8_t tens = (point_score - tenk*10000 - onek*1000 - oneh*100) / 10;
  uint8_t single = (point_score - tenk*10000 - onek*1000 - oneh*100 - tens*10);
  score_tiles[0] = 0;
  score_tiles[1] = 0;
  score_tiles[2] = 0;
  score_tiles[3] = tenk + 0x01;
  score_tiles[4] = onek + 0x01;
  score_tiles[5] = oneh + 0x01;
  score_tiles[6] = tens + 0x01;
  score_tiles[7] = single + 0x01;
}

// Writes the score tiles to the window.
inline void write_score_to_window(void) {
  set_win_tiles(12, 0, 8, 1, score_tiles);
}

// Converts the current high scores to tiles and displays the tiles in the window.
// The high scores are retrieved from RAM bank 0 as described in this file's header comment.
void display_highscores(void) {
  highscores2tiles();
  set_win_tiles(0, 0, 20, 1, high_score_tiles);
}

// Converts the point-based and timer-based scores to tiles and displays them in the gameover screen.
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
  uint8_t last_minutes= highscore->minutes;
  uint8_t last_seconds = highscore->seconds;

  // Display current timer-based score.
  update_timer_score_tiles();
  set_bkg_tiles(6, 5, 8, 1, score_tiles);

  // Get last high score and see if the current high score is better than it.
  uint16_t last_total_seconds = last_hours*3600 + last_minutes*60 + last_seconds;
  uint16_t current_total_seconds = timer_hours*3600 + timer_minutes*60 + timer_seconds;

  if (current_total_seconds > last_total_seconds) {
    // Write the current score as the new high score.
    set_bkg_tiles(6, 10, 8, 1, score_tiles);

    // Persist the high score by updating it in external RAM.
    highscore->hours = timer_hours;
    highscore->minutes = timer_minutes;
    highscore->seconds = timer_seconds;
  } else {
    // Display the last high score.
    timer_hours = last_hours;
    timer_minutes = last_minutes;
    timer_seconds = last_seconds;
    update_timer_score_tiles();
    set_bkg_tiles(6, 10, 8, 1, score_tiles);
  }

  // Display current point-based score.
  update_point_score_tiles();
  set_bkg_tiles(6, 6, 8, 1, score_tiles);

  if (point_score > last_points) {
    // Write the current score as the new high score.
    set_bkg_tiles(6, 11, 8, 1, score_tiles);

    // Persist the high score by updating it in external RAM.
    highscore->points = point_score;
  } else {
    // Display the last high score.
    point_score = last_points;
    update_point_score_tiles();
    set_bkg_tiles(6, 11, 8, 1, score_tiles);
  }
}

// Resets all scores.
void reset_scores(void) {
  point_score = 0;
  timer_frames = 0;
  timer_seconds = 0;
  timer_minutes = 0;
  timer_hours = 0;
  score_update_needed = true;
  update_point_score_tiles();
  // score_update_needed = false;
  // update_timer_score_tiles();
}

#endif
