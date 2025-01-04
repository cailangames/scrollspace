// Code for the procedural map generation.

#ifndef _PROCEDURAL_GENERATION_H_
#define _PROCEDURAL_GENERATION_H_

#pragma bank 1

#include <stdint.h>

#include <gb/gb.h>

// Resets the state of the procedural generation variables.
extern void reset_generation_state(void) BANKED;

// Generates the collision and background tile data for the given column and writes the output to
// the collision and background maps.
extern void generate_column(uint8_t column_idx) BANKED;

#endif
