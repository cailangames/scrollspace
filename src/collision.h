// Code that handles collisions.

#ifndef _COLLISION_H_
#define _COLLISION_H_

#include <stdint.h>

#include <gb/hardware.h>

#include "common.h"
#include "sprites.h"

// Checks if the given bullet sprite has collided with anything. Returns the index of the collided
// object in the collision/background map, or `UINT16_MAX` if there was no collision.
uint16_t check_bullet_collisions(struct Sprite* sprite, uint8_t* coll_map) {
  // For bullet sprites, we only check the front two corners of the sprite: The top right corner
  // and the bottom right corner.

  // Top right corner
  uint16_t row_top = (sprite->cb.y - SCREEN_T) >> 3;
  uint16_t row_top_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_top);
  uint8_t screen_left_col = SCX_REG >> 3;
  uint8_t col_right = MOD32(screen_left_col + ((sprite->cb.x + sprite->cb.w - SCREEN_L) >> 3));  // MOD32 is for screen wrap-around.
  uint16_t idx_top_right = row_top_offset + col_right;
  if (coll_map[idx_top_right] > 0 && coll_map[idx_top_right] < POWERUP_RESERVED_IDS) {
    return idx_top_right;
  }

  // Bottom right corner
  uint16_t row_bot = (sprite->cb.y + sprite->cb.h - SCREEN_T) >> 3;
  uint16_t row_bot_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_bot);
  uint16_t idx_bot_right = row_bot_offset + col_right;
  if (coll_map[idx_bot_right] > 0 && coll_map[idx_bot_right] < POWERUP_RESERVED_IDS) {
    return idx_bot_right;
  }

  // No collision
  return UINT16_MAX;
}

#endif
