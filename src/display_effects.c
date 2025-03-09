#include "display_effects.h"

#include <gb/hardware.h>
#include <stdint.h>

#include "wait.h"

void fade_out(void) {
  for (uint8_t i = 0; i < 4; ++i) {
    switch (i) {
      case 0:
        BGP_REG = 0xE4;
        break;
      case 1:
        BGP_REG = 0xF9;
        break;
      case 2:
        BGP_REG = 0xFE;
        break;
      case 3:
        BGP_REG = 0xFF;
        break;
    }
    wait_frames(10);
  }
}

void fade_in(void) {
  for (uint8_t i = 0; i < 3; ++i) {
    switch (i) {
      case 0:
        BGP_REG = 0xFE;
        break;
      case 1:
        BGP_REG = 0xF9;
        break;
      case 2:
        BGP_REG = 0xE4;
        break;
    }
    wait_frames(10);
  }
}
