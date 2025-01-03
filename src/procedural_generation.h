// Code for the procedural map generation.

#ifndef _PROCEDURAL_GENERATION_H_
#define _PROCEDURAL_GENERATION_H_

#pragma bank 1

#include <gb/gb.h>

// Resets the state of the procedural generation variables.
extern void reset_generation_state(void) BANKED;

// Generates the next column and writes the output to the collision and background maps.
extern void generate_next_column(void) BANKED;

#endif
