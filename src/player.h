// Code that handles the player's ship.

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

#include "collision.h"
#include "common.h"
#include "score.h"
#include "sound_effects.h"
#include "sprites.h"

static uint8_t player_sprite_base_id = 0;
static bool shield_active = false;
// Used during the damage recovery to toggle between showing and hidding the player sprite.
static enum animation_state damage_animation_state = HIDDEN;
// Invincibility frames counter, either from taking damage or picking up a shield.
static uint8_t iframes_counter = 0;

// Checks if the player sprite has collided with anything. Returns the index of the collided object
// in the collision/background map, or `UINT16_MAX` if there was no collision.
static uint16_t check_player_collisions(uint8_t* coll_map, bool pickups_only) {
  // The player sprite can collide with up to 4 tiles. Check the collision map on the top left,
  // top right, bottom left, and bottom right corners.

  // Top right corner
  uint16_t row_top = (player_sprite.cb.y - SCREEN_T) >> 3;
  uint16_t row_top_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_top);
  uint8_t screen_left_col = SCX_REG >> 3;
  uint8_t col_right = MOD32(screen_left_col + ((player_sprite.cb.x + player_sprite.cb.w - SCREEN_L) >> 3));  // MOD32 is for screen wrap-around.
  uint16_t idx_top_right = row_top_offset + col_right;
  if (pickups_only) {
    if (coll_map[idx_top_right] >= POWERUP_RESERVED_IDS) {
      return idx_top_right;
    }
  } else if (coll_map[idx_top_right] > 0) {
    return idx_top_right;
  }

  // Bottom right corner
  uint16_t row_bot = (player_sprite.cb.y + player_sprite.cb.h - SCREEN_T) >> 3;
  uint16_t row_bot_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row_bot);
  uint16_t idx_bot_right = row_bot_offset + col_right;
  if (pickups_only) {
    if (coll_map[idx_bot_right] >= POWERUP_RESERVED_IDS) {
      return idx_bot_right;
    }
  } else if (coll_map[idx_bot_right] > 0) {
    return idx_bot_right;
  }

  // Top left corner
  uint8_t col_left = MOD32(screen_left_col + ((player_sprite.cb.x - SCREEN_L) >> 3));  // MOD32 is for screen wrap-around.
  uint16_t idx_top_left = row_top_offset + col_left;
  if (pickups_only) {
    if (coll_map[idx_top_left] >= POWERUP_RESERVED_IDS) {
      return idx_top_left;
    }
  } else if (coll_map[idx_top_left] > 0) {
    return idx_top_left;
  }

  // Bottom left corner
  uint16_t idx_bot_left = row_bot_offset + col_left;
  if (pickups_only) {
    if (coll_map[idx_bot_left] >= POWERUP_RESERVED_IDS) {
      return idx_bot_left;
    }
  } else if (coll_map[idx_bot_left] > 0) {
    return idx_bot_left;
  }

  // No collision
  return UINT16_MAX;
}

// Initializes the player's variables.
void init_player(void) {
  player_sprite.type = PLAYER;
  player_sprite.sprite_id = PLAYER_SPRITE_ID;
  player_sprite.sprite_tile_id = 0;
  player_sprite.x = PLAYER_START_X;
  player_sprite.y = PLAYER_START_Y;
  player_sprite.speed = PLAYER_SPEED;
  player_sprite.dir = RIGHT;
  player_sprite.cb_x_offset = 1;
  player_sprite.cb_y_offset = 2;
  player_sprite.cb.x = PLAYER_START_X + player_sprite.cb_x_offset;
  player_sprite.cb.y = PLAYER_START_Y + player_sprite.cb_y_offset;
  player_sprite.cb.w = 5;
  player_sprite.cb.h = 4;
  player_sprite.health = PLAYER_MAX_HEALTH;
  player_sprite.lifespan = 0;  // Note: lifespan isn't used by the player sprite.
  player_sprite.active = true;
  move_sprite(PLAYER_SPRITE_ID, PLAYER_START_X, PLAYER_START_Y);

  player_sprite_base_id = 0;
  shield_active = false;
  damage_animation_state = HIDDEN;
  iframes_counter = 0;
}

// Moves the player's ship based on the given input.
void move_player(uint8_t input) {
  uint8_t dx = 0;
  uint8_t dy = 0;
  player_sprite.dir = RIGHT;
  player_sprite.sprite_tile_id = player_sprite_base_id;
  // Reset player collision box to default.
  player_sprite.cb_x_offset = 1;
  player_sprite.cb_y_offset = 2;
  player_sprite.cb.w = 5;
  player_sprite.cb.h = 4;

  // Check input. Note that the player can move in two directions, e.g. "up and right" or "down and left".
  if (KEY_PRESSED(input, J_RIGHT)) {
    dx = player_sprite.speed;
  } else if (KEY_PRESSED(input, J_LEFT)) {
    dx = -player_sprite.speed;
    player_sprite.dir = LEFT;
  }

  if (KEY_PRESSED(input, J_UP)) {
    dy = -player_sprite.speed;
    player_sprite.dir |= UP;
    player_sprite.sprite_tile_id = player_sprite_base_id + 1;

    // Make collision box smaller when plane is "tilted".
    player_sprite.cb_x_offset = 2;
    player_sprite.cb_y_offset = 3;
    player_sprite.cb.w = 3;
    player_sprite.cb.h = 1;
  } else if (KEY_PRESSED(input, J_DOWN)) {
    dy = player_sprite.speed;
    player_sprite.dir |= DOWN;
    player_sprite.sprite_tile_id = player_sprite_base_id + 2;

    // Make collision box smaller when plane is "tilted".
    player_sprite.cb_x_offset = 2;
    player_sprite.cb_y_offset = 4;
    player_sprite.cb.w = 3;
    player_sprite.cb.h = 1;
  }

  // Update player position.
  player_sprite.x += dx;
  if (player_sprite.x < SCREEN_L) {
    player_sprite.x = SCREEN_L;
  } else if (player_sprite.x > SCREEN_R) {
    player_sprite.x = SCREEN_R;
  }

  player_sprite.y += dy;
  if (player_sprite.y < SCREEN_T) {
    player_sprite.y = SCREEN_T;
  } else if (player_sprite.y > SCREEN_B) {
    player_sprite.y = SCREEN_B;
  }

  // Update collision box.
  player_sprite.cb.x = player_sprite.x + player_sprite.cb_x_offset;
  player_sprite.cb.y = player_sprite.y + player_sprite.cb_y_offset;

  // Move the player's sprite.
  set_sprite_tile(PLAYER_SPRITE_ID, player_sprite.sprite_tile_id);
  move_sprite(PLAYER_SPRITE_ID, player_sprite.x, player_sprite.y);
}

// Checks if the player collided with anything and updates the player's position (e.g. due to
// knockback) accordingly. Returns true if the player collided with anything, false otherwise.
bool handle_player_collisions(uint8_t* coll_map, uint8_t* bkg_map) {
  bool player_collided = false;
  bool health_changed = false;
  if (iframes_counter == 0) {
    // The player is *not* in the invincibility frames state.
    // First check if the damage animation or shield powerup is still active. If so, deactivate it.
    if (shield_active) {
      shield_active = false;
      player_sprite_base_id -= 10;
    }
    if (damage_animation_state == HIDDEN) {
      move_sprite(PLAYER_SPRITE_ID, player_sprite.x, player_sprite.y);
      damage_animation_state = SHOWN;
    }
    // Normal collision check for player
    uint16_t collision_idx = check_player_collisions(coll_map, false);
    if (collision_idx != UINT16_MAX) {
      player_collided = true;
      if (coll_map[collision_idx] == HEALTH_KIT_ID) {
        // Pick up health kit.
        coll_map[collision_idx] = 0;
        bkg_map[collision_idx] = 0;
        // Update player health.
        // When updating this code, be wary that the max value of an int8_t is 127.
        if (player_sprite.health < 20) {
          player_sprite.health += 4*HEALTH_KIT_VALUE;
        } else if (player_sprite.health < 50) {
          player_sprite.health += 2*HEALTH_KIT_VALUE;
        } else {
          player_sprite.health += HEALTH_KIT_VALUE;
          if (player_sprite.health > PLAYER_MAX_HEALTH) {
            player_sprite.health = PLAYER_MAX_HEALTH;
          }
        }
        health_changed = true;
        play_health_sound();
      }
      else if (coll_map[collision_idx] == SHIELD_ID) {
        // Pick up shield.
        coll_map[collision_idx] = 0;
        bkg_map[collision_idx] = 0;
        shield_active = true;
        iframes_counter = SHIELD_DURATION;
        player_sprite_base_id += 10;
        play_shield_sound();
      }
      else {
        // The player hit a wall or a mine.
        player_sprite.health -= COLLISION_DAMAGE;
        health_changed = true;
        iframes_counter = COLLISION_TIMEOUT;
        if (coll_map[collision_idx] <= PLAYER_COLLISION_DAMAGE) {
          // The wall or mine is destroyed.
          if (bkg_map[collision_idx] == MINE_IDX) {
            increment_point_score(POINTS_PER_MINE);
          }
          coll_map[collision_idx] = 0;
          bkg_map[collision_idx] = 0;
        }
        else {
          // Apply damage to the wall or mine.
          coll_map[collision_idx] -= PLAYER_COLLISION_DAMAGE;
        }
        // Handle knockback: Push sprite in the opposite direction that it's moving.
        if (player_sprite.dir & RIGHT) {
          // Move the sprite to the left.
          player_sprite.x -= PLAYER_COLLISION_KNOCKBACK;
        } else if (player_sprite.dir & LEFT) {
          // Move the sprite to the right.
          player_sprite.x += PLAYER_COLLISION_KNOCKBACK;
        }
        if (player_sprite.dir & UP) {
          // Move the sprite down.
          player_sprite.y += PLAYER_COLLISION_KNOCKBACK;
        } else if (player_sprite.dir & DOWN) {
          // Move the sprite up.
          player_sprite.y -= PLAYER_COLLISION_KNOCKBACK;
        }
        move_sprite(PLAYER_SPRITE_ID, player_sprite.x, player_sprite.y);
      }
    }
  }
  else {
    // The player *is* in the invincibility frames state.
    --iframes_counter;
    if (!shield_active) {
      if (damage_animation_state == HIDDEN) {
        move_sprite(PLAYER_SPRITE_ID, player_sprite.x, player_sprite.y);
        damage_animation_state = SHOWN;
      } else {
        move_sprite(PLAYER_SPRITE_ID, 0, 0);
        damage_animation_state = HIDDEN;
      }
      // Check for collision and only process pickups if the shield is not active.
      // This will allow the player to pick up items while in the iframes state.
      uint16_t collision_idx = check_player_collisions(coll_map, true);
      if (collision_idx != UINT16_MAX) {
        player_collided = true;
        if (coll_map[collision_idx] == HEALTH_KIT_ID) {
          // Pick up health kit.
          coll_map[collision_idx] = 0;
          bkg_map[collision_idx] = 0;
          // Update player health.
          // When updating this code, be wary that the max value of an int8_t is 127.
          if (player_sprite.health < 20) {
            player_sprite.health += 4*HEALTH_KIT_VALUE;
          } else if (player_sprite.health < 50) {
            player_sprite.health += 2*HEALTH_KIT_VALUE;
          } else {
            player_sprite.health += HEALTH_KIT_VALUE;
            if (player_sprite.health > PLAYER_MAX_HEALTH) {
              player_sprite.health = PLAYER_MAX_HEALTH;
            }
          }
          health_changed = true;
          play_health_sound();
        }
        else {
          // Pick up shield.
          coll_map[collision_idx] = 0;
          bkg_map[collision_idx] = 0;
          shield_active = true;
          iframes_counter = SHIELD_DURATION;
          player_sprite_base_id += 10;
          play_shield_sound();
        }
      }
    }
  }

  if (health_changed) {
    // Update the ship sprite based on the current health.
    if (player_sprite.health > 50) {
      player_sprite_base_id = 0;
    } else if (player_sprite.health > 25) {
      player_sprite_base_id = 3;
    } else {
      player_sprite_base_id = 6;
    }
  }

  return player_collided;
}

#endif
