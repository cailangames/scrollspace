#include "weapons.h"

#include <gb/gb.h>
#include <gb/hardware.h>
#include <stdbool.h>
#include <stdint.h>

#include "collision.h"
#include "common.h"
#include "sound_effects.h"
#include "sprites.h"

// Sprite data for all the bullets the player can use.
static struct Sprite bullet_sprites[MAX_BULLETS];
// How many bullets are actively flying across the screen.
static uint8_t active_bullet_count = 0;
// How many frames left until the bomb is ready again. 0 means the bomb is ready.
static uint16_t bomb_cooldown_frames = 0;
// The bomb icon ("ready" or "not ready") in the window.
static uint8_t bomb_icon = 0;
// Whether or not the bomb icon needs to be updated.
static bool bomb_icon_update_needed = false;
// Whether or not a bomb was dropped this frame.
static bool bomb_dropped = false;
// The top row at which the last bomb was dropped.
static uint8_t bombed_row_top = 0;
// The left column at which the last bomb was dropped.
static uint8_t bombed_col_left = 0;
// The height of the explosion from the last bomb dropped.
static uint8_t bombed_height = 0;

// Updates the collision and background maps in response to a dropped bomb. The bomb explosion is a
// square in front of the player with sides of length `2*BOMB_RADIUS+1` tiles.
static void drop_bomb(void) {
  uint8_t row_count = BOMB_LENGTH;
  uint8_t row_top = (player_sprite.y.h - SCREEN_T) >> 3;
  // Because of integer division, the bomb explosion can look like it's not centered with the ship.
  // The following code fixes that.
  uint8_t row_pixel_delta = (player_sprite.y.h - SCREEN_T) - (row_top << 3);
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
  if (row_count > COLUMN_HEIGHT - row_top) {
    // No need to calculate the part of the bomb explosion that's off the bottom of the screen.
    row_count = COLUMN_HEIGHT - row_top;
  }
  uint16_t row_top_offset = MAP_INDEX_ROW_OFFSET(row_top);
  uint8_t* coll_map_start = collision_map + row_top_offset;
  uint8_t* bkg_map_start = background_map + row_top_offset;
  uint16_t x_left = player_sprite.x.h - SCREEN_L;
  uint16_t col_left = MOD32(((x_left + SCX_REG) >> 3) + 1);  // Add 1 to be in front of the player. MOD32 is for screen wrap-around.
  uint8_t incremental_score = 0;
  uint8_t height = 0;
  for (uint8_t i = 0; i < BOMB_LENGTH; ++i) {
    uint8_t col = MOD32(col_left + i);  // MOD32 is for screen wrap-around.
    uint8_t* coll_map = coll_map_start + col;
    uint8_t* bkg_map = bkg_map_start + col;
    for (uint8_t j = 0; j < row_count; ++j) {
      // If we are destroying a mine, add to the score.
      if (*bkg_map == MINE_TILE) {
        incremental_score += POINTS_PER_MINE;
      }
      *bkg_map = CRATER_TILE;
      bkg_map += ROW_WIDTH;
      *coll_map = 0;
      coll_map += ROW_WIDTH;
    }
    ++height;
  }
  point_score += incremental_score;
  bombed_row_top = row_top;
  bombed_col_left = col_left;
  bombed_height = height;
}

void init_weapons(void) {
  // Initialize bullets.
  active_bullet_count = 0;
  struct Sprite* b = bullet_sprites;
  for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
    b->active = false;
    b->sprite_id = i + 1;  // +1 so we don't override the player (always sprite_id 0)
    b->sprite_tile_id = BULLET_SPRITE;
    b->lifespan = 0;
    b->health = 0;
    b->direction = RIGHT;
    b->speed.w = BULLET_SPEED;
    b->x.w = 0;
    b->y.w = 0;
    // Collision box
    b->cb_x_offset = BULLET_COLLISION_X_OFFSET;
    b->cb_y_offset = BULLET_COLLISION_Y_OFFSET;
    b->cb.x = b->x.h + BULLET_COLLISION_X_OFFSET;
    b->cb.y = b->y.h + BULLET_COLLISION_Y_OFFSET;
    b->cb.w = 4;
    b->cb.h = 4;
    b->collided = false;
    b->collided_row = 0;
    b->collided_col = 0;
    set_sprite_tile(b->sprite_id, b->sprite_tile_id);
    move_sprite(b->sprite_id, b->x.h, b->y.h);

    ++b;
  }
  // Initialize the bomb.
  bomb_cooldown_frames = 0;
  bomb_icon = BOMB_READY_ICON;
  bomb_icon_update_needed = true;
  bomb_dropped = false;
  bombed_row_top = 0;
  bombed_col_left = 0;
  bombed_height = 0;
}

void update_weapons(uint8_t input, uint8_t prev_input) {
  // Activate a new bullet if the A button is pressed.
  if (KEY_FIRST_PRESS(input, prev_input, J_A) && active_bullet_count < MAX_BULLETS) {
    // Find first non-active bullet in `bullets` array and activate it.
    struct Sprite* b = bullet_sprites;
    for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
      if (b->active) {
        ++b;
        continue;
      }
      b->active = true;
      b->lifespan = BULLET_LIFESPAN;
      b->x.w = player_sprite.x.w;
      b->cb.x = b->x.h + BULLET_COLLISION_X_OFFSET;
      b->y.w = player_sprite.y.w;
      b->cb.y = b->y.h + BULLET_COLLISION_Y_OFFSET;
      move_sprite(b->sprite_id, b->x.h, b->y.h);
      set_sprite_prop(b->sprite_id, 0x10); // puts the bullet on palette 1
      ++active_bullet_count;
      play_gun_sound();
      break;
    }
  }

  // Update the bomb.
  if (bomb_cooldown_frames != 0) {
    --bomb_cooldown_frames;
    if (bomb_cooldown_frames == 0) {
      bomb_icon = BOMB_READY_ICON;
      bomb_icon_update_needed = true;
    }
  }
  if (KEY_FIRST_PRESS(input, prev_input, J_B) && bomb_cooldown_frames == 0) {
    play_bomb_sound();
    drop_bomb();
    bomb_cooldown_frames = BOMB_COOLDOWN_FRAMES;
    bomb_icon = EMPTY_TILE;
    bomb_icon_update_needed = true;
    bomb_dropped = true;
  }

  // Update active bullets.
  if (active_bullet_count != 0) {
    // Move bullets.
    struct Sprite* b = bullet_sprites;
    for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
      if (!b->active) {
        ++b;
        continue;
      }
      b->x.w += b->speed.w;
      b->cb.x = b->x.h + BULLET_COLLISION_X_OFFSET;
      if (b->x.h > SCREEN_R || --b->lifespan == 0) {
        // Hide sprite.
        b->active = false;
        b->x.w = 0;
        b->y.w = 0;
        --active_bullet_count;
      } else {
        uint16_t collision_idx = check_bullet_collisions(b);
        if (collision_idx != UINT16_MAX) {
          // The bullet collided with a wall or mine.
          if (collision_map[collision_idx] <= BULLET_DAMAGE) {
            // The wall or mine is destroyed.
            if (background_map[collision_idx] == MINE_TILE) {
              point_score += POINTS_PER_MINE;
            }
            collision_map[collision_idx] = 0;
            background_map[collision_idx] = 0;
          } else {
            // Apply damage to the wall or mine.
            collision_map[collision_idx] -= BULLET_DAMAGE;
          }
          // Hide bullet sprite.
          b->active = false;
          b->x.w = 0;
          b->y.w = 0;
          --active_bullet_count;
        }
      }
      // Update bullet's sprite position.
      move_sprite(b->sprite_id, b->x.h, b->y.h);

      ++b;
    }
  }
}

void update_tiles_hit_by_weapons(void) {
  // Update background tiles hit by bullets.
  struct Sprite* b = bullet_sprites;
  for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
    if (b->collided) {
      set_bkg_tile_xy(b->collided_col, b->collided_row, background_map[MAP_INDEX(b->collided_row, b->collided_col)]);
      b->collided = false;
    }
    ++b;
  }

  // Update background tiles hit by a dropped bomb.
  if (bomb_dropped) {
    if (bombed_col_left + BOMB_LENGTH <= ROW_WIDTH) {
      set_bkg_submap(bombed_col_left, bombed_row_top, BOMB_LENGTH, bombed_height, background_map, ROW_WIDTH);
    } else {
      // The screen wraps around to the start of the background map, so we need to take that
      // into account here by writing to VRAM in two batches.
      uint8_t first_batch_width = ROW_WIDTH - bombed_col_left;
      set_bkg_submap(bombed_col_left, bombed_row_top, first_batch_width, bombed_height, background_map, ROW_WIDTH);
      set_bkg_submap(0, bombed_row_top, BOMB_LENGTH - first_batch_width, bombed_height, background_map, ROW_WIDTH);
    }
    bomb_dropped = false;
  }
}

void update_bomb_ready_icon(void) {
  if (bomb_icon_update_needed) {
    set_win_tile_xy(9, 0, bomb_icon);
    bomb_icon_update_needed = false;
  }
}

void hide_bullet_sprites(void) {
  struct Sprite* b = bullet_sprites;
  for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
    move_sprite(b->sprite_id, 0, 0);
    ++b;
  }
}
