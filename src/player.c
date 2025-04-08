#include "player.h"

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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
// Used during the damage recovery to toggle between showing and hiding the player sprite.
static enum AnimationState damage_animation_state = SHOWN;
static uint8_t damage_animation_counter = 0;
// Invincibility frames counter, either from taking damage or picking up a shield.
static uint8_t iframes_counter = 0;
// Keeps track of how many frames the player has been in the knockback state.
static uint8_t knockback_counter = 0;
// Tile data for the health bar.
static uint8_t health_bar_window_tiles[10];

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
  // We have tiles for 5% increments in the health

  // Fill in the top part of the bar
  if (health > 95) {
    // Full
    health_bar_window_tiles[9] = HEALTH_BAR_START + 2;
  } else if (health > 90) {
    // Half filled
    health_bar_window_tiles[9] = HEALTH_BAR_START + 5;
  } else {
    // Empty
    health_bar_window_tiles[9] = HEALTH_BAR_START + 8;
  }

  // Fill in the center part of the bar
  uint8_t full_count = MAX(0, MIN(8, health / 10 - 1));
  int8_t half_count = MOD2(MIN(18, health / 5));
  memset(health_bar_window_tiles + 1, HEALTH_BAR_START + 1, full_count);                   // Full tile
  memset(health_bar_window_tiles + 1 + full_count, HEALTH_BAR_START + 7, 8 - full_count);  // Empty tile
  if (half_count && (health > 5)) {
    health_bar_window_tiles[full_count + 1] = HEALTH_BAR_START + 4;
  }

  // Fill in the bottom part of the bar
  if (health >= 10) {
    // Full
    health_bar_window_tiles[0] = HEALTH_BAR_START;

  } else if (health > 0) {
    // Half filled
    health_bar_window_tiles[0] = HEALTH_BAR_START + 3;
  } else {
    // Empty
    health_bar_window_tiles[0] = HEALTH_BAR_START + 6;
  }
}

void write_health_bar_to_window(void) {
  set_win_tiles(0, 0, 10, 1, health_bar_window_tiles);
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

  player_base_sprite = PLAYER_BASE_SPRITE;
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
      player_sprite.sprite_tile += 1;

      // Make collision box smaller when ship is "tilted".
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
      player_sprite.sprite_tile += 2;

      // Make collision box smaller when ship is "tilted".
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
  bool pickups_only = false;
  // First, update animations and counters.
  if (iframes_counter != 0) {
    // The player *is* in the invincibility frames (iframes) state.
    --iframes_counter;
    if (!shield_active) {
      // When the player is in the iframes state without the shield, collisions with mines and
      // walls are ignored.
      pickups_only = true;
    }
    --damage_animation_counter;
    if (damage_animation_counter == 0) {
      // Toggle the animation state.
      if (damage_animation_state == HIDDEN) {
        if (shield_active) {
          OBP0_REG = 0xE4;  // 0b1110 0100 - black, dark gray, light gray, white
        } else {
          move_sprite(PLAYER_SPRITE_ID, player_sprite.x.h, player_sprite.y.h);
        }
        damage_animation_state = SHOWN;
      } else {
        if (shield_active) {
          OBP0_REG = 0xD0;  // 0b1101 0000 - black, white, white, white
        } else {
          move_sprite(PLAYER_SPRITE_ID, 0, 0);
        }
        damage_animation_state = HIDDEN;
      }
      damage_animation_counter = shield_active ? SHIELD_ANIMATION_CYCLE : IFRAMES_ANIMATION_CYCLE;
    }
  } else {
    // The player is *not* in the invincibility frames (iframes) state.
    // Check if the damage animation or shield powerup is still active. If so, deactivate it.
    shield_active = false;
    if (damage_animation_state == HIDDEN) {
      move_sprite(PLAYER_SPRITE_ID, player_sprite.x.h, player_sprite.y.h);
      OBP0_REG = 0xE4;  // 0b1110 0100 - black, dark gray, light gray, white
      damage_animation_state = SHOWN;
    }
  }

  // Then, check collisions.
  bool health_changed = false;
  uint16_t collision_idx = check_player_collisions(pickups_only);
  if (collision_idx != UINT16_MAX) {
    switch (collision_map[collision_idx]) {
      case HEALTH_KIT_ID:
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
        point_score += POINTS_PER_PICKUP;
        play_health_sound();
        break;
      case SHIELD_ID:
        // Pick up shield.
        shield_active = true;
        iframes_counter = SHIELD_DURATION;
        if (damage_animation_state == HIDDEN) {
          move_sprite(PLAYER_SPRITE_ID, player_sprite.x.h, player_sprite.y.h);
          OBP0_REG = 0xE4;  // 0b1110 0100 - black, dark gray, light gray, white
        }
        damage_animation_state = SHOWN;
        damage_animation_counter = SHIELD_ANIMATION_START_DURATION;
        point_score += POINTS_PER_PICKUP;
        play_shield_sound();
        break;
      default:
        // The player hit a wall or a mine.
        if (!shield_active) {
          player_sprite.health -= COLLISION_DAMAGE;
          health_changed = true;
          iframes_counter = IFRAMES_DURATION;
          damage_animation_counter = IFRAMES_ANIMATION_CYCLE;
          knockback_counter = PLAYER_KNOCKBACK_DURATION;
        }
        // The wall or mine is destroyed.
        if (background_map[collision_idx] == MINE_TILE) {
          point_score += POINTS_PER_MINE;
        }
        play_collision_sound();
        break;
    }

    // Remove whatever the player collided into.
    collision_map[collision_idx] = 0;
    background_map[collision_idx] = 0;
  }

  // Lastly, update UI and sprites based on any collisions.
  if (health_changed) {
    update_health_bar_tiles(player_sprite.health);
  }
  // Update the ship sprite based on the current health.
  if (player_sprite.health > PLAYER_DAMAGED_THRESHOLD) {
    player_base_sprite = 0;
  } else if (player_sprite.health > PLAYER_CRITICALLY_DAMAGED_THRESHOLD) {
    player_base_sprite = 3;
  } else {
    player_base_sprite = 6;
  }
  // Additionally, update the ship sprite if the shield is active.
  if (shield_active) {
    player_base_sprite += PLAYER_SHIELD_SPRITES_OFFSET;
  }

  return health_changed;
}
