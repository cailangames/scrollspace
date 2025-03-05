#include <stdbool.h>
#include <stdint.h>

#include <gb/gb.h>

#include "wait.h"

void wait_frames(uint8_t n) {
  for (uint8_t i = 0; i < n; ++i) {
    vsync();
  }
}

void wait_for_keys_pressed(uint8_t keys) {
  while (!(joypad() & keys)) {
    vsync();
  }
}

void wait_for_keys_released(uint8_t keys) {
  while (joypad() & keys) {
    vsync();
  }
}
