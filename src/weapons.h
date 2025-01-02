// Code that handles the weapons (bullets and bombs).

#ifndef _WEAPONS_H_
#define _WEAPONS_H_

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>
#include <gb/hardware.h>

#include "collision.h"
#include "common.h"
#include "score.h"
#include "sound_effects.h"

// Sprite data for all the bullets the player can use.
static struct Sprite bullet_sprites[MAX_BULLETS];
// How many bullets are actively flying across the screen.
static uint8_t active_bullet_count = 0;
// How many frames left until the bomb is ready again. 0 means the bomb is ready.
static uint16_t bomb_cooldown_frames = 0;

// Updates the collision and background maps in response to a dropped bomb. The bomb explosion is a
// square in front of the player with sides of length `2*BOMB_RADIUS+1` tiles.
static void drop_bomb(void) {
  uint8_t row_count = (BOMB_RADIUS * 2) + 1;  // +1 for the center row, which is centered on the ship.
  uint8_t row_top = (player_sprite.y - SCREEN_T) >> 3;
  // Because of integer division, the bomb explosion can look like it's not centered with the ship.
  // The following code fixes that.
  uint8_t row_pixel_delta = (player_sprite.y - SCREEN_T) - (row_top << 3);
  if (row_pixel_delta >= 4) {
    ++row_top;
  }

  if (row_top >= BOMB_RADIUS) {
    row_top -= BOMB_RADIUS;
  } else {
    // The player is at the top of the screen, so we need to shorten the bomb explosion so that it
    // doesn't go off screen.
    row_count = row_count - (BOMB_RADIUS - row_top);
    row_top = 0;
  }
  uint8_t col_left = (SCX_REG >> 3) + ((player_sprite.x - SCREEN_L) >> 3) + 1;  // Add 1 to be in front of the player.
  uint8_t incremental_score = 0;
  for (uint8_t i = 0; i < row_count; ++i) {
    uint16_t row = row_top + i;
    if (row >= COLUMN_HEIGHT) {
      // No need to calculate or draw the part of the bomb explosion that's off screen.
      break;
    }
    uint16_t row_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row);
    for (uint8_t j = 0; j < (BOMB_RADIUS * 2) + 1; ++j) {
      uint8_t col = MOD32(col_left + j);  // MOD32 is for screen wrap-around.

      uint16_t idx = row_offset + col;
      // If we are destroying a mine, add to the score.
      if (background_map[idx] == MINE_IDX) {
        incremental_score += POINTS_PER_MINE;
      }
      collision_map[idx] = 0;
      background_map[idx] = CRATERBLOCK_IDX;
    }
  }
  increment_point_score(incremental_score);
}

void init_weapons(void) {
  // Initialize bullets.
  active_bullet_count = 0;
  for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
    struct Sprite* b = &(bullet_sprites[i]);
    b->active = false;
    b->type = BULLET;
    b->sprite_id = i + 1;  // +1 so we don't override the player (always sprite_id 0)
    b->sprite_tile_id = BULLET_SPRITE;
    b->lifespan = 0;
    b->health = 0;
    b->dir = RIGHT;
    b->speed = 4;
    b->x = 0;
    b->y = 0;
    // Collision box
    b->cb_x_offset = 0;
    b->cb_y_offset = 2;
    b->cb.x = b->x + b->cb_x_offset;
    b->cb.y = b->y + b->cb_y_offset;
    b->cb.w = 4;
    b->cb.h = 4;
    set_sprite_tile(b->sprite_id, b->sprite_tile_id);
    move_sprite(b->sprite_id, b->x, b->y);
  }
  // Initialize the bomb.
  bomb_cooldown_frames = 0;
  set_win_tile_xy(9, 0, BOMB_ICON_IDX);
}

// Updates the bullets and bombs based on the given input. Returns true if the background map/tiles
// need to be updated, false otherwise.
bool update_weapons(uint8_t input, uint8_t prev_input) {
  bool update_bkg_map = false;

  // Activate a new bullet if the A button is pressed.
  if (KEY_FIRST_PRESS(input, prev_input, J_A) && active_bullet_count < MAX_BULLETS) {
    // Find first non-active bullet in `bullets` array and activate it.
    for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
      if (bullet_sprites[i].active) {
        continue;
      }
      struct Sprite* b = &(bullet_sprites[i]);
      b->active = true;
      b->lifespan = BULLET_LIFESPAN;
      b->x = player_sprite.x;
      b->cb.x = b->x + b->cb_x_offset;
      b->y = player_sprite.y;
      b->cb.y = b->y + b->cb_y_offset;
      move_sprite(b->sprite_id, b->x, b->y);
      ++active_bullet_count;
      play_gun_sound();
      break;
    }
  }

  // Update the bomb.
  if (bomb_cooldown_frames != 0) {
    --bomb_cooldown_frames;
    if (bomb_cooldown_frames == 0) {
      set_win_tile_xy(9, 0, BOMB_ICON_IDX);
    }
  }
  if (KEY_FIRST_PRESS(input, prev_input, J_B) && bomb_cooldown_frames == 0) {
    play_bomb_sound();
    drop_bomb();
    bomb_cooldown_frames = BOMB_COOLDOWN_FRAMES;
    set_win_tile_xy(9, 0, BOMB_SILHOUETTE_ICON_IDX);
    update_bkg_map = true;
  }

  // Update active bullets.
  if (active_bullet_count != 0) {
    // Move bullets.
    for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
      if (!bullet_sprites[i].active) {
        continue;
      }
      struct Sprite* b = &(bullet_sprites[i]);
      b->x += b->speed;
      b->cb.x = b->x + b->cb_x_offset;
      b->lifespan--;
      if (b->x > SCREEN_R || b->lifespan == 0) {
        // Hide sprite.
        b->active = false;
        b->x = 0;
        b->y = 0;
        --active_bullet_count;
      }
      else {
        uint16_t collision_idx = check_bullet_collisions(b);
        if (collision_idx != UINT16_MAX) {
          // The bullet collided with a wall or mine.
          if (collision_map[collision_idx] <= BULLET_DAMAGE) {
            // The wall or mine is destroyed.
            if (background_map[collision_idx] == MINE_IDX) {
              increment_point_score(POINTS_PER_MINE);
            }
            collision_map[collision_idx] = 0;
            background_map[collision_idx] = 0;
            update_bkg_map = true;
          }
          else {
            // Apply damage to the wall or mine.
            collision_map[collision_idx] -= BULLET_DAMAGE;
          }
          // Hide bullet sprite.
          b->active = false;
          b->x = 0;
          b->y = 0;
          --active_bullet_count;
        }
      }
      // Update bullet's sprite position.
      move_sprite(b->sprite_id, b->x, b->y);
    }
  }

  return update_bkg_map;
}

void hide_bullet_sprites(void) {
  for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
    move_sprite(bullet_sprites[i].sprite_id, 0, 0);
  }
}

#endif
