#include "collision.h"

#include <gb/hardware.h>
#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "sprites.h"

uint16_t check_bullet_collisions(struct Sprite* sprite) {
  // For bullet sprites, we only check the front two corners of the sprite: The top right corner
  // and the bottom right corner.

  // Top right corner
  uint16_t row_top = (sprite->cb.y - SCREEN_T) >> 3;
  uint16_t x_right = sprite->cb.x + sprite->cb.w - SCREEN_L;
  uint16_t col_right = MOD32((x_right + SCX_REG) >> 3);  // MOD32 is for screen wrap-around.
  uint16_t idx_top_right = MAP_INDEX(row_top, col_right);
  if (collision_map[idx_top_right] > 0 && collision_map[idx_top_right] < POWERUP_RESERVED_IDS) {
    sprite->collided = true;
    sprite->collided_row = row_top;
    sprite->collided_col = col_right;
    return idx_top_right;
  }

  // Bottom right corner
  uint16_t row_bot = (sprite->cb.y + sprite->cb.h - SCREEN_T) >> 3;
  uint16_t idx_bot_right = MAP_INDEX(row_bot, col_right);
  if (collision_map[idx_bot_right] > 0 && collision_map[idx_bot_right] < POWERUP_RESERVED_IDS) {
    sprite->collided = true;
    sprite->collided_row = row_bot;
    sprite->collided_col = col_right;
    return idx_bot_right;
  }

  // No collision
  return UINT16_MAX;
}

uint16_t check_player_collisions(bool pickups_only) {
  // The player sprite can collide with up to 4 tiles. Check the collision map on the top left,
  // top right, bottom left, and bottom right corners.

  // Top right corner
  uint16_t row_top = (player_sprite.cb.y - SCREEN_T) >> 3;
  uint16_t row_top_offset = MAP_INDEX_ROW_OFFSET(row_top);
  uint16_t x_right = player_sprite.cb.x + player_sprite.cb.w - SCREEN_L;
  uint16_t col_right = MOD32((x_right + SCX_REG) >> 3);  // MOD32 is for screen wrap-around.
  uint16_t idx_top_right = row_top_offset + col_right;
  if (pickups_only) {
    if (collision_map[idx_top_right] >= POWERUP_RESERVED_IDS) {
      player_sprite.collided = true;
      player_sprite.collided_row = row_top;
      player_sprite.collided_col = col_right;
      return idx_top_right;
    }
  } else if (collision_map[idx_top_right] > 0) {
    player_sprite.collided = true;
    player_sprite.collided_row = row_top;
    player_sprite.collided_col = col_right;
    return idx_top_right;
  }

  // Bottom right corner
  uint16_t row_bot = (player_sprite.cb.y + player_sprite.cb.h - SCREEN_T) >> 3;
  uint16_t row_bot_offset = MAP_INDEX_ROW_OFFSET(row_bot);
  uint16_t idx_bot_right = row_bot_offset + col_right;
  if (pickups_only) {
    if (collision_map[idx_bot_right] >= POWERUP_RESERVED_IDS) {
      player_sprite.collided = true;
      player_sprite.collided_row = row_bot;
      player_sprite.collided_col = col_right;
      return idx_bot_right;
    }
  } else if (collision_map[idx_bot_right] > 0) {
    player_sprite.collided = true;
    player_sprite.collided_row = row_bot;
    player_sprite.collided_col = col_right;
    return idx_bot_right;
  }

  // Top left corner
  uint16_t x_left = player_sprite.cb.x - SCREEN_L;
  uint16_t col_left = MOD32((x_left + SCX_REG) >> 3);  // MOD32 is for screen wrap-around.
  uint16_t idx_top_left = row_top_offset + col_left;
  if (pickups_only) {
    if (collision_map[idx_top_left] >= POWERUP_RESERVED_IDS) {
      player_sprite.collided = true;
      player_sprite.collided_row = row_top;
      player_sprite.collided_col = col_left;
      return idx_top_left;
    }
  } else if (collision_map[idx_top_left] > 0) {
    player_sprite.collided = true;
    player_sprite.collided_row = row_top;
    player_sprite.collided_col = col_left;
    return idx_top_left;
  }

  // Bottom left corner
  uint16_t idx_bot_left = row_bot_offset + col_left;
  if (pickups_only) {
    if (collision_map[idx_bot_left] >= POWERUP_RESERVED_IDS) {
      player_sprite.collided = true;
      player_sprite.collided_row = row_bot;
      player_sprite.collided_col = col_left;
      return idx_bot_left;
    }
  } else if (collision_map[idx_bot_left] > 0) {
    player_sprite.collided = true;
    player_sprite.collided_row = row_bot;
    player_sprite.collided_col = col_left;
    return idx_bot_left;
  }

  // No collision
  return UINT16_MAX;
}
