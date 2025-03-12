// Code that handles the weapons (bullets and bombs)

#ifndef _WEAPONS_H_
#define _WEAPONS_H_

#include <stdint.h>

// Initializes variables for the weapons.
void init_weapons(void);

// Updates the bullets and bombs based on the given input. Sets `Sprite.collided` to true if the
// bullet sprite collided with anything.
void update_weapons(uint8_t input, uint8_t prev_input);

// Updates the background tiles that were hit by bullets and bombs.
void update_tiles_hit_by_weapons(void);

// Updates the "bomb ready" icon in the window, if necessary.
void update_bomb_ready_icon(void);

// Hides the bullet sprites.
void hide_bullet_sprites(void);

#endif
