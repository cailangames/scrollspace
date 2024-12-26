// Code that handles collisions.

#ifndef _COLLISION_H_
#define _COLLISION_H_

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

#include "common.h"
#include "score.h"
#include "sprites.h"

// Checks if the given sprite has collided with anything. Returns the ID of the collided object,
// or 0 if there was no collision.
uint8_t check_collisions(struct Sprite* sprite, uint8_t* coll_map, uint8_t* bkg_map, bool is_player, bool pickups_only) {
  /*
   * The player sprite can collide with up to 4 tiles.
   * Check the collision map on the top_left, top_right
   * bottom_left, and bottom right corners.
   */

  uint16_t idx_topl, idx_topr;
  uint16_t idx_botl, idx_botr;
  uint16_t row_topl, row_topr;
  uint16_t row_botl, row_botr;
  uint16_t col_topl, col_topr;
  uint16_t col_botl, col_botr;
  uint8_t collision = 0;
  uint8_t collision_block = 0;
  int8_t block_health = 0;

  row_topl = (sprite->cb.y - 16) / 8;
  col_topl = ((SCX_REG + sprite->cb.x - 8) % 256) / 8;
  idx_topl = col_topl + row_topl*32;

  row_topr = row_topl;
  col_topr = ((SCX_REG + sprite->cb.x + sprite->cb.w - 8) % 256) / 8;
  idx_topr = col_topr + row_topr*32;

  row_botl = ((sprite->cb.y + sprite->cb.h - 16) % 256) / 8;
  col_botl = col_topl;
  idx_botl = col_botl + row_botl*32;

  row_botr = row_botl;
  col_botr = col_topr;
  idx_botr = col_botr + row_botr*32;

  if (coll_map[idx_topr] > 0) {
    collision = coll_map[idx_topr];
    collision_block = bkg_map[idx_topr];

    // Check if it is not a power up
    if (collision < 235 && !pickups_only) {
      // Apply damage to the background element
      if (is_player) {
        block_health = coll_map[idx_topr] - 2;
      }
      else {
        // Bullet
        block_health = coll_map[idx_topr] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & UP) {
          // Top Right collision detected while sprite is moving up
          sprite->y += 8;
        }
        if (sprite->dir & RIGHT) {
          // Top Right collision detected while sprite is moving up
          sprite->x -= 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_topr] = 0;
        coll_map[idx_topr] = 0;

        if (collision_block == MAPBLOCK_IDX + 2) {
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_topr] = block_health;
      }
    }
    else if (collision < 235 && pickups_only) {
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_topr] = 0;
    }
    else if (collision >= 235 && is_player) {
      // Pick up power up
      bkg_map[idx_topr] = 0;
      coll_map[idx_topr] = 0;
    }
  }
  else if (coll_map[idx_botr] > 0) {
    collision = coll_map[idx_botr];
    collision_block = bkg_map[idx_botr];

    // Check if it is not a power up
    if (collision < 235 && !pickups_only) {
      // Apply damage to the background element
      if (is_player) {
        block_health = coll_map[idx_botl] - 2;
      }
      else {
        // Bullet
        block_health = coll_map[idx_botl] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & DOWN) {
          // Bottom Right collision detected while sprite is moving down
          sprite->y -= 8;
        }
        if (sprite->dir & RIGHT) {
          // Bottom Right collision detected while sprite is moving down
          sprite->x -= 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_botr] = 0;
        coll_map[idx_botr] = 0;

        if (collision_block == MAPBLOCK_IDX + 2) {
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_botr] = block_health;
      }
    }
    else if (collision < 235 && pickups_only) {
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_botr] = 0;
    }
    else if (collision >= 235 && is_player) {
      // Pick up power up
      bkg_map[idx_botr] = 0;
      coll_map[idx_botr] = 0;
    }
  }
  else if (is_player && coll_map[idx_topl] > 0) {
    // Only the player has left side collisions
    collision = coll_map[idx_topl];
    collision_block = bkg_map[idx_topl];

    // Check if it is not a power up
    if (collision < 235 && !pickups_only) {
      // Apply damage to the background element
      if (is_player) {
        block_health = coll_map[idx_topl] - 2;
      }
      else {
        // Bullet
        block_health = coll_map[idx_topl] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & UP) {
          // Top Left collision detected while sprite is moving up
          sprite->y += 8;
        }
        if (sprite->dir & LEFT) {
          // Top Left collision detected while sprite is moving up
          sprite->x += 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_topl] = 0;
        coll_map[idx_topl] = 0;

        if (collision_block == MAPBLOCK_IDX + 2) {
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_topl] = block_health;
      }
    }
    else if (collision < 235 && pickups_only) {
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_topl] = 0;
    }
    else if (collision >= 235 && is_player) {
      // Pick up power up
      bkg_map[idx_topl] = 0;
      coll_map[idx_topl] = 0;
    }
  }
  else if (is_player && coll_map[idx_botl] > 0) {
    // Only the player has left side collisions
    collision = coll_map[idx_botl];
    collision_block = bkg_map[idx_botl];

    // Check if it is not a power up
    if (collision < 235 && !pickups_only) {
      // Apply damage to the background element
      if (is_player) {
        block_health = coll_map[idx_botl] - 2;
      }
      else {
        // Bullet
        block_health = coll_map[idx_botl] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & DOWN) {
          // Bottom Left collision detected while sprite is moving down
          sprite->y -= 8;
        }
        if (sprite->dir & LEFT) {
          // Bottom Left collision detected while sprite is moving down
          sprite->x += 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_botl] = 0;
        coll_map[idx_botl] = 0;

        if (collision_block == MAPBLOCK_IDX + 2) {
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_botl] = block_health;
      }
    }
    else if (collision < 235 && pickups_only) {
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_botl] = 0;
    }
    else if (collision >= 235 && is_player) {
      // Pick up power up
      bkg_map[idx_botl] = 0;
      coll_map[idx_botl] = 0;
    }
  }

  if (collision > 0 && block_health > 0 && is_player) {
    // Move the player sprite after collision
    move_sprite(sprite->sprite_id, sprite->x, sprite->y);
  }

  return collision;
}

#endif
