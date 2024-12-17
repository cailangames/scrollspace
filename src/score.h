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

// Initializes the external RAM and reads the high scores from it. Returns true if high scores are
// found in the external RAM, false otherwise. If no high scores are found, then they're initialized
// with zeroes.
bool init_highscores(void);

// Increments the timer-based score.
void increment_timer_score(void);

// Increments the point-based score by the given value.
void increment_point_score(uint16_t increment_value);

// Converts the current point-based score to tiles and displays the tiles in the window.
void display_point_score(void);

// Converts the current timer-based score to tiles and displays the tiles in the window.
void display_timer_score(void);

// Converts the current high scores to tiles and displays the tiles in the window.
// The high scores are retrieved from RAM bank 0 as described in this file's header comment.
void display_highscores(void);

// Converts the point-based and timer-based scores to tiles and displays them in the gameover screen.
void display_gameover_scores(void);

// Resets all scores.
void reset_scores(void);

#endif
