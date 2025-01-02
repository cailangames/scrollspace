// Code that handles collisions.

#ifndef _COLLISION_H_
#define _COLLISION_H_

#include <stdbool.h>
#include <stdint.h>

#include <gb/hardware.h>

#include "common.h"
#include "sprites.h"

// Checks if the given bullet sprite has collided with anything. Returns the index of the collided
// object in the collision/background map, or `UINT16_MAX` if there was no collision.
uint16_t check_bullet_collisions(struct Sprite* sprite) {
  // For bullet sprites, we only check the front two corners of the sprite: The top right corner
  // and the bottom right corner.

  // Top right corner
  uint16_t row_top = (sprite->cb.y - SCREEN_T) >> 3;
  uint16_t row_top_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_top);
  uint8_t screen_left_col = SCX_REG >> 3;
  uint8_t col_right = MOD32(screen_left_col + ((sprite->cb.x + sprite->cb.w - SCREEN_L) >> 3));  // MOD32 is for screen wrap-around.
  uint16_t idx_top_right = row_top_offset + col_right;
  if (collision_map[idx_top_right] > 0 && collision_map[idx_top_right] < POWERUP_RESERVED_IDS) {
    return idx_top_right;
  }

  // Bottom right corner
  uint16_t row_bot = (sprite->cb.y + sprite->cb.h - SCREEN_T) >> 3;
  uint16_t row_bot_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_bot);
  uint16_t idx_bot_right = row_bot_offset + col_right;
  if (collision_map[idx_bot_right] > 0 && collision_map[idx_bot_right] < POWERUP_RESERVED_IDS) {
    return idx_bot_right;
  }

  // No collision
  return UINT16_MAX;
}

// Checks if the player sprite has collided with anything. Returns the index of the collided object
// in the collision/background map, or `UINT16_MAX` if there was no collision.
uint16_t check_player_collisions(bool pickups_only) {
  // The player sprite can collide with up to 4 tiles. Check the collision map on the top left,
  // top right, bottom left, and bottom right corners.

  // Top right corner
  uint16_t row_top = (player_sprite.cb.y - SCREEN_T) >> 3;
  uint16_t row_top_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_top);
  uint8_t screen_left_col = SCX_REG >> 3;
  uint8_t col_right = MOD32(screen_left_col + ((player_sprite.cb.x + player_sprite.cb.w - SCREEN_L) >> 3));  // MOD32 is for screen wrap-around.
  uint16_t idx_top_right = row_top_offset + col_right;
  if (pickups_only) {
    if (collision_map[idx_top_right] >= POWERUP_RESERVED_IDS) {
      return idx_top_right;
    }
  } else if (collision_map[idx_top_right] > 0) {
    return idx_top_right;
  }

  // Bottom right corner
  uint16_t row_bot = (player_sprite.cb.y + player_sprite.cb.h - SCREEN_T) >> 3;
  uint16_t row_bot_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_bot);
  uint16_t idx_bot_right = row_bot_offset + col_right;
  if (pickups_only) {
    if (collision_map[idx_bot_right] >= POWERUP_RESERVED_IDS) {
      return idx_bot_right;
    }
  } else if (collision_map[idx_bot_right] > 0) {
    return idx_bot_right;
  }

  // Top left corner
  uint8_t col_left = MOD32(screen_left_col + ((player_sprite.cb.x - SCREEN_L) >> 3));  // MOD32 is for screen wrap-around.
  uint16_t idx_top_left = row_top_offset + col_left;
  if (pickups_only) {
    if (collision_map[idx_top_left] >= POWERUP_RESERVED_IDS) {
      return idx_top_left;
    }
  } else if (collision_map[idx_top_left] > 0) {
    return idx_top_left;
  }

  // Bottom left corner
  uint16_t idx_bot_left = row_bot_offset + col_left;
  if (pickups_only) {
    if (collision_map[idx_bot_left] >= POWERUP_RESERVED_IDS) {
      return idx_bot_left;
    }
  } else if (collision_map[idx_bot_left] > 0) {
    return idx_bot_left;
  }

  // No collision
  return UINT16_MAX;
}

#endif
