// Code for wait functions

#ifndef _WAIT_H_
#define _WAIT_H_

#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

// Waits for the given number of (vblank) frames.
void wait_frames(uint8_t n) {
  for (uint8_t i = 0; i < n; ++i) {
    vsync();
  }
}

// Waits for *any* of the given keys to be pressed.
void wait_for_keys_pressed(uint8_t keys) {
  while (!(joypad() & keys)) {
    vsync();
  }
}

// Waits for *all* of the given keys to be released.
void wait_for_keys_released(uint8_t keys) {
  while (joypad() & keys) {
    vsync();
  }
}

#endif
