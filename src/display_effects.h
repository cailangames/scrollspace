// Code for creating effects on the display

#ifndef _DISPLAY_EFFECTS_H_
#define _DISPLAY_EFFECTS_H_

#include <gb/hardware.h>
#include "common.h"

static void fade_out(void) {
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
    wait(10);
  }
}

static void fade_in(void) {
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
    wait(10);
  }
}

#endif