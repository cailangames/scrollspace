// Code for the procedural map generation

#ifndef _PROCEDURAL_GENERATION_H_
#define _PROCEDURAL_GENERATION_H_

#include <gb/gb.h>
#include <stdint.h>

// Resets the state of the procedural generation variables.
// Note: This function needs to be called after `initrand()` has been called.
extern void reset_generation_state(void) BANKED;

// Generates the collision and background tile data for the given column and writes the output to
// the collision and background maps.
extern void generate_column(uint8_t column_idx) BANKED;

#endif
