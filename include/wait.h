// Code for wait functions

#ifndef _WAIT_H_
#define _WAIT_H_

#include <stdint.h>

// Waits for the given number of (vblank) frames.
void wait_frames(uint8_t n);

// Waits for *any* of the given keys to be pressed.
void wait_for_keys_pressed(uint8_t keys);

// Waits for *all* of the given keys to be released.
void wait_for_keys_released(uint8_t keys);

#endif
