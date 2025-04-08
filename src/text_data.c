#pragma bank 1

#include "text_data.h"

#include <stdint.h>

#include "common.h"

// The below data is generated with a specific format and thus shouldn't be auto-formatted.
// clang-format off

const uint8_t press_text[] = {CHAR_P, CHAR_R, CHAR_E, CHAR_S, CHAR_S};
const uint8_t start_text[] = {CHAR_S, CHAR_T, CHAR_A, CHAR_R, CHAR_T};
const uint8_t normal_text[] = {CHAR_N, CHAR_O, CHAR_R, CHAR_M, CHAR_A, CHAR_L};
const uint8_t hard_text[] = {CHAR_H, CHAR_A, CHAR_R, CHAR_D};
const uint8_t turbo_text[] = {CHAR_T, CHAR_U, CHAR_R, CHAR_B, CHAR_O};
const uint8_t unlock_hard_mode_text[] = {CHAR_U, CHAR_N, CHAR_L, CHAR_O, CHAR_C, CHAR_K, CHAR_COLON, 0, CHAR_2, CHAR_0, CHAR_0, CHAR_0, 0, CHAR_N, CHAR_O, CHAR_R, CHAR_M, CHAR_A, CHAR_L, 0};
const uint8_t unlock_turbo_mode_text[] = {CHAR_U, CHAR_N, CHAR_L, CHAR_O, CHAR_C, CHAR_K, CHAR_COLON, 0, CHAR_1, CHAR_0, CHAR_0, CHAR_0, 0, CHAR_H, CHAR_A, CHAR_R, CHAR_D, 0, 0, 0};
const uint8_t clear_data_text[] = {CHAR_C, CHAR_L, CHAR_E, CHAR_A, CHAR_R, 0, CHAR_D, CHAR_A, CHAR_T, CHAR_A};
const uint8_t clear_data_description_text[] = {CHAR_D, CHAR_E, CHAR_L, CHAR_E, CHAR_T, CHAR_E, CHAR_S, 0, CHAR_H, CHAR_I, CHAR_G, CHAR_H, 0, CHAR_S, CHAR_C, CHAR_O, CHAR_R, CHAR_E, CHAR_S, 0};
const uint8_t confirmation_prompt_text[] = {0, 0, 0, 0, CHAR_A, CHAR_R, CHAR_E, 0, CHAR_Y, CHAR_O, CHAR_U, 0, CHAR_S, CHAR_U, CHAR_R, CHAR_E, CHAR_QUESTION_MARK, 0, 0, 0};
const uint8_t yes_or_no_text[] = {0, 0, 0, 0, 0, CHAR_Y, CHAR_E, CHAR_S, 0, 0, 0, 0, CHAR_CURSOR, 0, CHAR_N, CHAR_O, 0, 0, 0, 0};
const uint8_t shoot_text[] = {CHAR_S, CHAR_H, CHAR_O, CHAR_O, CHAR_T};
const uint8_t bomb_text[] = {CHAR_B, CHAR_O, CHAR_M, CHAR_B};
const uint8_t pause_text[] = {CHAR_P, CHAR_A, CHAR_U, CHAR_S, CHAR_E};
const uint8_t game_over_text[] = {CHAR_G, CHAR_A, CHAR_M, CHAR_E, 0, CHAR_O, CHAR_V, CHAR_E, CHAR_R};
const uint8_t score_text[] = {CHAR_S, CHAR_C, CHAR_O, CHAR_R, CHAR_E};
const uint8_t best_text[] = {CHAR_B, CHAR_E, CHAR_S, CHAR_T};
const uint8_t new_record_text[] = {CHAR_N, CHAR_E, CHAR_W, 0, CHAR_R, CHAR_E, CHAR_C, CHAR_O, CHAR_R, CHAR_D, CHAR_EXCLAMATION_MARK};
const uint8_t tip_text[] = {CHAR_T, CHAR_I, CHAR_P, CHAR_COLON};
const uint8_t congratulations_text[] = {CHAR_C, CHAR_O, CHAR_N, CHAR_G, CHAR_R, CHAR_A, CHAR_T, CHAR_U, CHAR_L, CHAR_A, CHAR_T, CHAR_I, CHAR_O, CHAR_N, CHAR_S, CHAR_EXCLAMATION_MARK};
const uint8_t you_unlocked_text[] = {CHAR_Y, CHAR_O, CHAR_U, 0, CHAR_U, CHAR_N, CHAR_L, CHAR_O, CHAR_C, CHAR_K, CHAR_E, CHAR_D};
const uint8_t hard_mode_text[] = {CHAR_H, CHAR_A, CHAR_R, CHAR_D, 0, CHAR_M, CHAR_O, CHAR_D, CHAR_E};
const uint8_t turbo_mode_text[] = {CHAR_T, CHAR_U, CHAR_R, CHAR_B, CHAR_O, 0, CHAR_M, CHAR_O, CHAR_D, CHAR_E};
const uint8_t the_xenobird_text[] = {CHAR_T, CHAR_H, CHAR_E, 0, CHAR_X, CHAR_E, CHAR_N, CHAR_O, CHAR_B, CHAR_I, CHAR_R, CHAR_D};

// Each tip message is 28 characters wide and is wrapped to a new line after the 16th character.
const uint8_t tip_messages[] = {
  // "BULLETS BREAK WALLS"
  CHAR_B, CHAR_U, CHAR_L, CHAR_L, CHAR_E, CHAR_T, CHAR_S, 0, CHAR_B, CHAR_R, CHAR_E, CHAR_A, CHAR_K, 0, 0, 0,
  CHAR_W, CHAR_A, CHAR_L, CHAR_L, CHAR_S, 0, 0, 0, 0, 0, 0, 0,
  // "DIAGONAL IS FASTER"
  CHAR_D, CHAR_I, CHAR_A, CHAR_G, CHAR_O, CHAR_N, CHAR_A, CHAR_L, 0, CHAR_I, CHAR_S, 0, 0, 0, 0, 0,
  CHAR_F, CHAR_A, CHAR_S, CHAR_T, CHAR_E, CHAR_R, 0, 0, 0, 0, 0, 0,
  // "MINE 2 PTS, PICK UP 2, WALL 0"
  CHAR_M, CHAR_I, CHAR_N, CHAR_E, 0, CHAR_2, 0, CHAR_P, CHAR_T, CHAR_S, CHAR_COMMA, 0, CHAR_P, CHAR_I, CHAR_C, CHAR_K,
  CHAR_U, CHAR_P, 0, CHAR_2, 0, CHAR_W, CHAR_A, CHAR_L, CHAR_L, 0, CHAR_0, 0,
  // "BOMB CLUSTERS FOR MAX PTS"
  CHAR_B, CHAR_O, CHAR_M, CHAR_B, 0, CHAR_C, CHAR_L, CHAR_U, CHAR_S, CHAR_T, CHAR_E, CHAR_R, CHAR_S, 0, 0, 0,
  CHAR_F, CHAR_O, CHAR_R, 0, CHAR_M, CHAR_A, CHAR_X, 0, CHAR_P, CHAR_T, CHAR_S, 0,
  // "LOW HEALTH, HIGH RECOVERY"
  CHAR_L, CHAR_O, CHAR_W, 0, CHAR_H, CHAR_E, CHAR_A, CHAR_L, CHAR_T, CHAR_H, CHAR_COMMA, 0, CHAR_H, CHAR_I, CHAR_G, CHAR_H,
  CHAR_R, CHAR_E, CHAR_C, CHAR_O, CHAR_V, CHAR_E, CHAR_R, CHAR_Y, 0, 0, 0, 0,
  // "FLASH THROUGH OBSTACLES"
  CHAR_F, CHAR_L, CHAR_A, CHAR_S, CHAR_H, 0, CHAR_T, CHAR_H, CHAR_R, CHAR_O, CHAR_U, CHAR_G, CHAR_H, 0, 0, 0,
  CHAR_O, CHAR_B, CHAR_S, CHAR_T, CHAR_A, CHAR_C, CHAR_L, CHAR_E, CHAR_S, 0, 0, 0,
  // "GET POINTS EARLY"
  CHAR_G, CHAR_E, CHAR_T, 0, CHAR_P, CHAR_O, CHAR_I, CHAR_N, CHAR_T, CHAR_S, 0, CHAR_E, CHAR_A, CHAR_R, CHAR_L, CHAR_Y,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  // "SHIELD THROUGH MINE FIELDS"
  CHAR_S, CHAR_H, CHAR_I, CHAR_E, CHAR_L, CHAR_D, 0, CHAR_T, CHAR_H, CHAR_R, CHAR_O, CHAR_U, CHAR_G, CHAR_H, 0, 0,
  CHAR_M, CHAR_I, CHAR_N, CHAR_E, 0, CHAR_F, CHAR_I, CHAR_E, CHAR_L, CHAR_D, CHAR_S, 0
};
