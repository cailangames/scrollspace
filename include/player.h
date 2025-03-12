// Code that handles the player's ship

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <stdbool.h>
#include <stdint.h>

// Updates the health bar tiles based on the given health. Call `write_health_bar_to_window()` to
// write these tiles to the window.
void update_health_bar_tiles(int8_t health);

// Writes the health bar tiles to the window.
void write_health_bar_to_window(void);

// Initializes the player's variables.
void init_player(void);

// Moves the player's ship based on the given input.
void move_player(uint8_t input);

// Checks if the player collided with anything and updates the player's position (e.g. due to
// knockback) accordingly. Sets `player_sprite.collided` to true if the player collided with
// anything. Returns a bool specifying whether or not the player health changed.
bool handle_player_collisions(void);

#endif
