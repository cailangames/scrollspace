#include "player.h"

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>

#include "collision.h"
#include "common.h"
#include "sound_effects.h"
#include "sprites.h"

enum AnimationState {
  HIDDEN = 0,
  SHOWN = 1,
};

static uint8_t player_base_sprite = 0;
static bool shield_active = false;
// Used during the damage recovery to toggle between showing and hidding the player sprite.
static enum AnimationState damage_animation_state = SHOWN;
static uint8_t damage_animation_counter = 0;
// Invincibility frames counter, either from taking damage or picking up a shield.
static uint8_t iframes_counter = 0;
// Keeps track of how many frames the player has been in the knockback state.
static uint8_t knockback_counter = 0;
// Tile data for the health bar.
static uint8_t health_bar_window_tiles[8];

// Knocks back the player (i.e. pushes the player sprite in the opposite direction that the player
// was moving).
static void handle_knockback(void) {
  if (player_sprite.direction & RIGHT) {
    // Push the sprite to the left (accounting for the scroll speed).
    player_sprite.x.w -= (scroll_speed.w + PLAYER_KNOCKBACK_SPEED);
    if (player_sprite.x.h < SCREEN_L) {
      player_sprite.x.w = ((uint16_t)(SCREEN_L) << 8);
    }
  } else if (player_sprite.direction & LEFT) {
    // Push the sprite to the right.
    player_sprite.x.w += PLAYER_KNOCKBACK_SPEED;
    if (player_sprite.x.h > SCREEN_R) {
      player_sprite.x.w = ((uint16_t)(SCREEN_R) << 8);
    }
  }

  if (player_sprite.direction & UP) {
    // Push the sprite down.
    player_sprite.y.w += PLAYER_KNOCKBACK_SPEED;
    if (player_sprite.y.h > SCREEN_B) {
      player_sprite.y.w = ((uint16_t)(SCREEN_B) << 8);
    }
  } else if (player_sprite.direction & DOWN) {
    // Push the sprite up.
    player_sprite.y.w -= PLAYER_KNOCKBACK_SPEED;
    if (player_sprite.y.h < SCREEN_T) {
      player_sprite.y.w = ((uint16_t)(SCREEN_T) << 8);
    }
  }
}

void update_health_bar_tiles(int8_t health) {
  if (health == 100) {
    health_bar_window_tiles[0] = HEALTH_BAR_START;  // left edge of bar
    for (uint8_t i = 1; i < 7; ++i) {
      health_bar_window_tiles[i] = HEALTH_BAR_START + 1;  // center of bar
    }
    health_bar_window_tiles[7] = HEALTH_BAR_START + 2;  // right edge of bar
  } else if (health >= 88) {
    health_bar_window_tiles[0] = HEALTH_BAR_START;  // left edge of bar
    for (uint8_t i = 1; i < 7; ++i) {
      health_bar_window_tiles[i] = HEALTH_BAR_START + 1;  // center of bar
    }
    health_bar_window_tiles[7] = HEALTH_BAR_START + 5;  // right edge of bar
  } else if (health >= 16) {
    uint8_t idx = health / 12;
    health_bar_window_tiles[0] = HEALTH_BAR_START;  // left edge of bar
    for (uint8_t i = 1; i < 7; ++i) {
      if (i < idx) {
        health_bar_window_tiles[i] = HEALTH_BAR_START + 1;  // fill
      } else {
        health_bar_window_tiles[i] = HEALTH_BAR_START + 4;  // clear
      }
    }
    health_bar_window_tiles[7] = HEALTH_BAR_START + 5;  // clear right edge of bar
  } else if (health > 0) {
    health_bar_window_tiles[1] = HEALTH_BAR_START + 4;
    health_bar_window_tiles[0] = HEALTH_BAR_START;
  } else {
    health_bar_window_tiles[1] = HEALTH_BAR_START + 4;  // clear bottom 2 tiles
    health_bar_window_tiles[0] = HEALTH_BAR_START + 3;
  }
}

void write_health_bar_to_window(void) {
  set_win_tiles(0, 0, 8, 1, health_bar_window_tiles);
}

void init_player(void) {
  player_sprite.sprite_id = PLAYER_SPRITE_ID;
  player_sprite.sprite_tile = PLAYER_BASE_SPRITE;
  player_sprite.x.w = PLAYER_START_X;
  player_sprite.y.w = PLAYER_START_Y;
  player_sprite.direction = RIGHT;
  player_sprite.cb_x_offset = 1;
  player_sprite.cb_y_offset = 2;
  player_sprite.cb.x = player_sprite.x.h + player_sprite.cb_x_offset;
  player_sprite.cb.y = player_sprite.y.h + player_sprite.cb_y_offset;
  player_sprite.cb.w = 5;
  player_sprite.cb.h = 4;
  player_sprite.health = PLAYER_MAX_HEALTH;
  player_sprite.lifespan = 0;  // Note: lifespan isn't used by the player sprite.
  player_sprite.active = true;
  player_sprite.collided = false;
  player_sprite.collided_row = 0;
  player_sprite.collided_col = 0;
  move_sprite(PLAYER_SPRITE_ID, player_sprite.x.h, player_sprite.y.h);

  player_base_sprite = 0;
  shield_active = false;
  damage_animation_state = SHOWN;
  damage_animation_counter = 0;
  iframes_counter = 0;
  knockback_counter = 0;

  update_health_bar_tiles(PLAYER_MAX_HEALTH);

  if (game_mode == NORMAL) {
    player_sprite.speed.w = PLAYER_SPEED_NORMAL;
  } else if (game_mode == HARD) {
    player_sprite.speed.w = PLAYER_SPEED_HARD;
  } else {
    player_sprite.speed.w = PLAYER_SPEED_TURBO;
  }
}

void move_player(uint8_t input) {
  player_sprite.sprite_tile = player_base_sprite;
  // Reset player collision box to default.
  player_sprite.cb_x_offset = 1;
  player_sprite.cb_y_offset = 2;
  player_sprite.cb.w = (player_sprite.health > PLAYER_DAMAGED_THRESHOLD) ? 5 : 4;
  player_sprite.cb.h = 4;

  if (knockback_counter != 0) {
    --knockback_counter;
    handle_knockback();
  } else {
    player_sprite.direction = RIGHT;
    // Check input. Note that the player can move in two directions, e.g. "up and right" or "down and left".
    if (KEY_PRESSED(input, J_RIGHT)) {
      player_sprite.x.w += player_sprite.speed.w;
      if (player_sprite.x.h > SCREEN_R) {
        player_sprite.x.w = ((uint16_t)(SCREEN_R) << 8);
      }
    } else if (KEY_PRESSED(input, J_LEFT)) {
      player_sprite.direction = LEFT;
      // When moving left, add some extra speed to account for the background scroll speed.
      player_sprite.x.w -= (player_sprite.speed.w + (scroll_speed.w >> 1));
      if (player_sprite.x.h < SCREEN_L) {
        player_sprite.x.w = ((uint16_t)(SCREEN_L) << 8);
      }
    }

    if (KEY_PRESSED(input, J_UP)) {
      player_sprite.direction |= UP;
      player_sprite.y.w -= player_sprite.speed.w;
      if (player_sprite.y.h < SCREEN_T) {
        player_sprite.y.w = ((uint16_t)(SCREEN_T) << 8);
      }
      player_sprite.sprite_tile = player_base_sprite + 1;

      // Make collision box smaller when plane is "tilted".
      player_sprite.cb_x_offset = 2;
      player_sprite.cb_y_offset = 3;
      --player_sprite.cb.w;
      player_sprite.cb.h = 1;
    } else if (KEY_PRESSED(input, J_DOWN)) {
      player_sprite.direction |= DOWN;
      player_sprite.y.w += player_sprite.speed.w;
      if (player_sprite.y.h > SCREEN_B) {
        player_sprite.y.w = ((uint16_t)(SCREEN_B) << 8);
      }
      player_sprite.sprite_tile = player_base_sprite + 2;

      // Make collision box smaller when plane is "tilted".
      player_sprite.cb_x_offset = 2;
      player_sprite.cb_y_offset = 4;
      --player_sprite.cb.w;
      player_sprite.cb.h = 1;
    }
  }

  // Update the position of the collision box.
  player_sprite.cb.x = player_sprite.x.h + player_sprite.cb_x_offset;
  player_sprite.cb.y = player_sprite.y.h + player_sprite.cb_y_offset;

  // Move the player's sprite.
  set_sprite_tile(PLAYER_SPRITE_ID, player_sprite.sprite_tile);
  move_sprite(PLAYER_SPRITE_ID, player_sprite.x.h, player_sprite.y.h);
}

bool handle_player_collisions(void) {
  bool health_changed = false;
  if (iframes_counter == 0) {
    // The player is *not* in the invincibility frames state.
    // First check if the damage animation or shield powerup is still active. If so, deactivate it.
    if (shield_active) {
      shield_active = false;
      player_base_sprite -= PLAYER_SHIELD_SPRITES_OFFSET;
    }
    if (damage_animation_state == HIDDEN) {
      move_sprite(PLAYER_SPRITE_ID, player_sprite.x.h, player_sprite.y.h);
      damage_animation_state = SHOWN;
      OBP0_REG = 0xE4;  // 0b1110 0100 - black, dark gray, light gray, white
    }
    // Normal collision check for player
    uint16_t collision_idx = check_player_collisions();
    if (collision_idx != UINT16_MAX) {
      uint8_t collision_id = collision_map[collision_idx];
      switch (collision_id) {
        case HEALTH_KIT_ID:
          // Pick up health kit.
          collision_map[collision_idx] = 0;
          background_map[collision_idx] = 0;
          // Update player health.
          // When updating this code, be wary that the max value of an int8_t is 127.
          if (player_sprite.health > PLAYER_DAMAGED_THRESHOLD) {
            player_sprite.health += HEALTH_KIT_VALUE;
            if (player_sprite.health > PLAYER_MAX_HEALTH) {
              player_sprite.health = PLAYER_MAX_HEALTH;
            }
          } else if (player_sprite.health > PLAYER_CRITICALLY_DAMAGED_THRESHOLD) {
            player_sprite.health += HEALTH_KIT_DAMAGED_VALUE;
          } else {
            player_sprite.health += HEALTH_KIT_CRITICALLY_DAMAGED_VALUE;
          }
          health_changed = true;
          point_score += POINTS_PER_PICKUP;
          play_health_sound();
          break;
        case SHIELD_ID:
          // Pick up shield.
          collision_map[collision_idx] = 0;
          background_map[collision_idx] = 0;
          shield_active = true;
          iframes_counter = SHIELD_DURATION;
          player_base_sprite += PLAYER_SHIELD_SPRITES_OFFSET;
          point_score += POINTS_PER_PICKUP;
          play_shield_sound();
          break;
        default:
          // The player hit a wall or a mine.
          player_sprite.health -= COLLISION_DAMAGE;
          health_changed = true;
          iframes_counter = IFRAMES_DURATION;
          damage_animation_counter = IFRAMES_ANIMATION_CYCLE;
          knockback_counter = PLAYER_KNOCKBACK_DURATION;
          if (collision_id <= PLAYER_COLLISION_DAMAGE) {
            // The wall or mine is destroyed.
            if (background_map[collision_idx] == MINE_TILE) {
              point_score += POINTS_PER_MINE;
            }
            collision_map[collision_idx] = 0;
            background_map[collision_idx] = 0;
          } else {
            // Apply damage to the wall or mine.
            collision_map[collision_idx] -= PLAYER_COLLISION_DAMAGE;
          }
          play_collision_sound();
          break;
      }
    }
  } else {
    // The player *is* in the invincibility frames state.
    --iframes_counter;
    if (!shield_active) {
      --damage_animation_counter;
      if (damage_animation_counter == 0) {
        // Toggle the animation state.
        if (damage_animation_state == HIDDEN) {
          move_sprite(PLAYER_SPRITE_ID, player_sprite.x.h, player_sprite.y.h);
          damage_animation_state = SHOWN;
        } else {
          move_sprite(PLAYER_SPRITE_ID, 0, 0);
          damage_animation_state = HIDDEN;
        }
        damage_animation_counter = IFRAMES_ANIMATION_CYCLE;
      }
    } else if (iframes_counter <= IFRAMES_DURATION - 2 * IFRAMES_ANIMATION_CYCLE) {
      // Toggle the sprite to signify the end of the shield powerup
      // Use the same logic as the damage animation
      --damage_animation_counter;
      if (damage_animation_counter == 0) {
        // Toggle the animation state.
        if (damage_animation_state == HIDDEN) {
          OBP0_REG = 0xE4;  // 0b1110 0100 - black, dark gray, light gray, white
          damage_animation_state = SHOWN;
        } else {
          OBP0_REG = 0xD0;  // 0b1101 0000 - black, white, white, white
          damage_animation_state = HIDDEN;
        }
        damage_animation_counter = 2 * IFRAMES_ANIMATION_CYCLE;
      }
    }
    // Check for collision with pickups only, not mines, to allow players to pick up items in the
    // iframes state.
    uint16_t collision_idx = check_player_collisions();
    if (collision_idx != UINT16_MAX) {
      uint8_t collision_id = collision_map[collision_idx];
      if (collision_id == HEALTH_KIT_ID) {
        // Pick up health kit and update player health.
        // When updating this code, be wary that the max value of an int8_t is 127.
        if (player_sprite.health > PLAYER_DAMAGED_THRESHOLD) {
          player_sprite.health += HEALTH_KIT_VALUE;
          if (player_sprite.health > PLAYER_MAX_HEALTH) {
            player_sprite.health = PLAYER_MAX_HEALTH;
          }
        } else if (player_sprite.health > PLAYER_CRITICALLY_DAMAGED_THRESHOLD) {
          player_sprite.health += HEALTH_KIT_DAMAGED_VALUE;
        } else {
          player_sprite.health += HEALTH_KIT_CRITICALLY_DAMAGED_VALUE;
        }
        health_changed = true;
        play_health_sound();
        collision_map[collision_idx] = 0;
        background_map[collision_idx] = 0;
        point_score += POINTS_PER_PICKUP;
      } else if (collision_id == SHIELD_ID) {
        // Pick up shield.
        if (!shield_active) {
          player_base_sprite += PLAYER_SHIELD_SPRITES_OFFSET;
          shield_active = true;
          damage_animation_counter = 2 * IFRAMES_ANIMATION_CYCLE;
          damage_animation_state = SHOWN;
          OBP0_REG = 0xE4;  // 0b1110 0100 - black, dark gray, light gray, white
        }
        iframes_counter = SHIELD_DURATION;
        play_shield_sound();
        collision_map[collision_idx] = 0;
        background_map[collision_idx] = 0;
        point_score += POINTS_PER_PICKUP;
      } else if (shield_active) {
        // The player hit a wall or a mine with the shield on
        if (collision_id <= SHIELD_COLLISION_DAMAGE) {
          // The wall or mine is destroyed.
          if (background_map[collision_idx] == MINE_TILE) {
            point_score += POINTS_PER_MINE;
          }
          collision_map[collision_idx] = 0;
          background_map[collision_idx] = 0;
        } else {
          // Apply damage to the wall or mine.
          collision_map[collision_idx] -= SHIELD_COLLISION_DAMAGE;
        }
        play_collision_sound();
      }
    }
  }

  if (health_changed) {
    // Update health bar after a collision.
    update_health_bar_tiles(player_sprite.health);
    // Update the ship sprite based on the current health.
    if (player_sprite.health > PLAYER_DAMAGED_THRESHOLD) {
      player_base_sprite = 0;
    } else if (player_sprite.health > PLAYER_CRITICALLY_DAMAGED_THRESHOLD) {
      player_base_sprite = 3;
    } else {
      player_base_sprite = 6;
    }

    if (shield_active) {
      player_base_sprite += PLAYER_SHIELD_SPRITES_OFFSET;
    }
  }

  return health_changed;
}
