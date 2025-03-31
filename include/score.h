// Code that handles scores, including the high scores
//
// There are two scores:
//   1. A point-based score that is incremented whenever the player destroys objects or clears screens.
//   2. A timer-based score that is incremented every frame.
//
// There are also high scores, which are persisted across restarts via (battery backed)  al RAM
// at bank 0 (starting address: 0xA000). These scores are stored in the following format:
//   * Bytes 0 and 1: High points score
//   * Byte 2: High timer score (hours)
//   * Byte 3: High timer score (minutes)
//   * Byte 4: High timer score (seconds)

#ifndef _SCORE_H_
#define _SCORE_H_

#include <stdbool.h>

// Clears the window with empty tiles.
void clear_window(void);

// Clears (zeroes out) the high score data in the external RAM.
void clear_score_data(void);

// Checks if the high scores meet the thresholds for unlocking, e.g., harder game modes, and sets
// the `*_unlocked` global variables accordingly. Returns true if any of the unlocked bools
// changed, false otherwise.
bool update_unlocks(void);

// Initializes the external RAM and reads each mode's high scores from it. If no high scores are
// found, then they're initialized with zeroes.
void init_highscores(void);

// Increments the timer-based score.
bool increment_timer_score(void);

// Updates the score tiles with the timer-based score. Note: This does *not* update the window;
// call `write_score_to_window()` for that.
void update_timer_score_tiles(void);

// Updates the score tiles with the point-based score. Note: This does *not* update the window;
// call `write_score_to_window()` for that.
void update_point_score_tiles(void);

// Writes the score tiles to the window.
void write_score_to_window(void);

// Converts the current high scores to tiles and displays the tiles in the window.
// The high scores are retrieved from RAM bank 0 as described in this file's header comment.
void display_highscores(void);

// Converts the point-based and timer-based scores to tiles and displays them in the gameover screen.
void display_gameover_scores(void);

// Resets all scores.
void reset_scores(void);

#endif
