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

#define RAM_BANK0_ADDRESS ((uint16_t*)0xA000)

static uint16_t point_score = 0;
static uint8_t timer_frames = 0;
static uint8_t timer_seconds = 0;
static uint8_t timer_minutes = 0;
static uint8_t timer_hours = 0;

static uint8_t score_tiles[8] = {0,0,0,0,0,0,0,0};
static uint8_t high_score_tiles[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static void timerscore2tiles(void) {
  // Seconds
  uint8_t tenths = timer_seconds/10;
  score_tiles[7] = (timer_seconds - 10*tenths) + 0x01;
  score_tiles[6] = tenths + 0x01;
  score_tiles[5] = COLON_FONT_IDX;

  // Minutes
  tenths = timer_minutes/10;
  score_tiles[4] = (timer_minutes - 10*tenths) + 0x01;
  score_tiles[3] = tenths + 0x01;
  score_tiles[2] = COLON_FONT_IDX;

  // Hours
  tenths = timer_hours/10;
  score_tiles[1] = (timer_hours - 10*tenths) + 0x01;
  score_tiles[0] = tenths + 0x01;
}

static void pointscore2tiles(void) {
  uint8_t tenk = point_score/10000;
  uint8_t onek = (point_score - tenk*10000) / 1000;
  uint8_t oneh = (point_score - tenk*10000 - onek*1000) / 100;
  uint8_t tens = (point_score - tenk*10000 - onek*1000 - oneh*100) / 10;
  uint8_t single = (point_score - tenk*10000 - onek*1000 - oneh*100 - tens*10);
  score_tiles[0] = 0;
  score_tiles[1] = 0;
  score_tiles[2] = 0;
  score_tiles[3] = tenk + 0x01;
  score_tiles[4] = onek + 0x1;
  score_tiles[5] = oneh + 0x1;
  score_tiles[6] = tens + 0x1;
  score_tiles[7] = single + 0x1;
}

static void highscores2tiles(void) {
  // Get high scores from external RAM.
  uint16_t* high_score_ptr = RAM_BANK0_ADDRESS;
  uint16_t points = high_score_ptr[0];
  uint8_t hours = (uint8_t)(high_score_ptr[1] >> 8);
  uint8_t minutes = (uint8_t)(high_score_ptr[1]);
  uint8_t seconds = (uint8_t)(high_score_ptr[2] >> 8);

  // Write out "BEST ".
  high_score_tiles[0] = 0xC;
  high_score_tiles[1] = 0xF;
  high_score_tiles[2] = 0x1D;
  high_score_tiles[3] = 0x1E;
  high_score_tiles[4] = 0x0;

  // Seconds
  uint8_t tenths = seconds/10;
  high_score_tiles[12] = (seconds - 10*tenths) + 0x01;
  high_score_tiles[11] = tenths + 0x01;
  high_score_tiles[10] = COLON_FONT_IDX;

  // Minutes
  tenths = minutes/10;
  high_score_tiles[9] = (minutes - 10*tenths) + 0x01;
  high_score_tiles[8] = tenths + 0x01;
  high_score_tiles[7] = COLON_FONT_IDX;

  // Hours
  tenths = hours/10;
  high_score_tiles[6] = (hours - 10*tenths) + 0x01;
  high_score_tiles[5] = tenths + 0x01;

  // Points
  uint8_t tenk = points/10000;
  uint8_t onek = (points - tenk*10000) / 1000;
  uint8_t oneh = (points - tenk*10000 - onek*1000) / 100;
  uint8_t tens = (points - tenk*10000 - onek*1000 - oneh*100) / 10;
  uint8_t single = (points - tenk*10000 - onek*1000 - oneh*100 - tens*10);
  high_score_tiles[13] = 0;
  high_score_tiles[14] = tenk + 0x01;
  high_score_tiles[15] = onek + 0x1;
  high_score_tiles[16] = oneh + 0x1;
  high_score_tiles[17] = tens + 0x1;
  high_score_tiles[18] = single + 0x1;
  high_score_tiles[19] = 0;
}

// Initializes the external RAM and reads the high scores from it. Returns true if high scores are
// found in the external RAM, false otherwise. If no high scores are found, then they're initialized
// with zeroes.
bool init_highscores(void) {
  // Turn on RAM and read the high scores (if present).
  uint16_t* high_score_ptr = RAM_BANK0_ADDRESS;
  ENABLE_RAM;
  SWITCH_RAM(0);
  if (high_score_ptr[0] == 0x0000 || high_score_ptr[0] == 0xFFFF) {
    high_score_ptr[0] = 0;
    high_score_ptr[1] = 0;
    high_score_ptr[2] = 0;
    return false;
  }
  // High scores found. Return true.
  return true;
}

// Increments the timer-based score.
void increment_timer_score(void) {
  ++timer_frames;
  if (timer_frames == 60) {
    ++timer_seconds;
    timer_frames = 0;

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

// Increments the point-based score by the given value.
void increment_point_score(uint16_t increment_value) {
  point_score += increment_value;
}

// Converts the current timer-based score to tiles and displays the tiles in the window.
void display_timer_score(void) {
  timerscore2tiles();
  set_win_tiles(12, 0, 8, 1, score_tiles);
}

// Converts the current point-based score to tiles and displays the tiles in the window.
void display_point_score(void) {
  pointscore2tiles();
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
  uint16_t* high_score_ptr = RAM_BANK0_ADDRESS;
  uint16_t last_points = high_score_ptr[0];
  uint8_t last_hours = (uint8_t)(high_score_ptr[1] >> 8);
  uint8_t last_minutes = (uint8_t)(high_score_ptr[1]);
  uint8_t last_seconds = (uint8_t)(high_score_ptr[2] >> 8);

  // Display current timer-based score.
  timerscore2tiles();
  set_bkg_tiles(6, 5, 8, 1, score_tiles);

  // Get last high score and see if the current high score is better than it.
  uint16_t last_total_seconds = last_hours*3600 + last_minutes*60 + last_seconds;
  uint16_t current_total_seconds = timer_hours*3600 + timer_minutes*60 + timer_seconds;

  if (current_total_seconds > last_total_seconds) {
    // Write the current score as the new high score.
    set_bkg_tiles(6, 10, 8, 1, score_tiles);

    // Persist the high score by updating the high_score_ptr.
    high_score_ptr[1] = timer_hours << 8 | timer_minutes;
    high_score_ptr[2] = timer_seconds << 8;
  } else {
    // Display the last high score.
    timer_hours = last_hours;
    timer_minutes = last_minutes;
    timer_seconds = last_seconds;
    timerscore2tiles();
    set_bkg_tiles(6, 10, 8, 1, score_tiles);
  }

  // Display current point-based score.
  pointscore2tiles();
  set_bkg_tiles(6, 6, 8, 1, score_tiles);

  if (point_score > last_points) {
    // Write the current score as the new high score.
    set_bkg_tiles(6, 11, 8, 1, score_tiles);

    // Persist the high score by updating the high_score_ptr.
    high_score_ptr[0] = point_score;
  } else {
    // Display the last high score.
    point_score = last_points;
    pointscore2tiles();
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
}

#endif
