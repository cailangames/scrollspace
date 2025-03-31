// Code that handles collisions

#ifndef _COLLISION_H_
#define _COLLISION_H_

#include <stdbool.h>
#include <stdint.h>

#include "sprites.h"

// Checks if the given bullet sprite has collided with anything. Returns the index of the collided
// object in the collision/background map, or `UINT16_MAX` if there was no collision. Also sets the
// `collided_*` fields for given Sprite if there is a collision.
uint16_t check_bullet_collisions(struct Sprite* sprite);

// Checks if the player sprite has collided with anything. Returns the index of the collided object
// in the collision/background map, or `UINT16_MAX` if there was no collision. Also sets the
// `collided_*` fields for `player_sprite` if there is a collision.
uint16_t check_player_collisions(bool pickups_only);

#endif
