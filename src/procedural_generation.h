// Code for the procedural map generation.

#ifndef _PROCEDURAL_GENERATION_H_
#define _PROCEDURAL_GENERATION_H_

#pragma bank 1

#include <gb/gb.h>
#include <stdint.h>

// Struct to hold state data in between calls to `generate_next_column()`.
struct GenerationState {
  uint8_t biome_id;
  uint8_t biome_column_index;
};

// Generates the next column and writes the output to collision and background maps.
// Important: `coll_map` and `bkg_map` should be pointers to the first index of the column that
// should be generated.
extern void generate_next_column(struct GenerationState* gen_state, uint8_t* coll_map, uint8_t* bkg_map) BANKED;

#endif
