#include "display_effects.h"

#include <gb/hardware.h>
#include <stdint.h>

#include "common.h"
#include "wait.h"

static const uint8_t fade_out_values[] = {0xE4, 0xF9, 0xFE, 0xFF};
static const uint8_t fade_in_values[] = {0xFE, 0xF9, 0xE4};

// Helper function for fading out/in.
static void fade(const uint8_t* values, uint8_t size) {
  for (uint8_t i = 0; i < size; ++i) {
    BGP_REG = *values;
    ++values;
    wait_frames(10);
  }
}

void fade_out(void) {
  fade(fade_out_values, UINT8_ARRARY_SIZE(fade_out_values));
}

void fade_in(void) {
  fade(fade_in_values, UINT8_ARRARY_SIZE(fade_in_values));
}
