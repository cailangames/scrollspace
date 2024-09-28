#pragma bank 1

#include <gb/gb.h>
#include <stdint.h>
#include <rand.h>

#include "common.h"
#include "procedural_generation.h"

#define BIOME_COUNT 3
#define COLUMNS_PER_BIOME 20

#define BIOME_OPEN_AREA 0
#define BIOME_DOWNWARD_STRIPE 1
#define BIOME_UPWARD_STRIPE 2

// Data for biome columns.
// Format: 20 [top, bottom] pairs for each biome
static const uint8_t biome_columns[BIOME_COUNT][COLUMNS_PER_BIOME * 2] = {
  /* BIOME_OPEN_AREA */       { 3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13,  3, 13},
  /* BIOME_DOWNWARD_STRIPE */ { 1,  3,  1,  3,  2,  4,  2,  4,  3,  5,  4,  6,  5,  7,  5,  7,  6,  8,  7,  9,  8, 10,  8, 10,  9, 11, 10, 12, 10, 12, 11, 13, 11, 13, 12, 14, 13, 15, 13, 15},
  /* BIOME_UPWARD_STRIPE */   {13, 15, 13, 15, 12, 14, 11, 13, 11, 13, 10, 12,  9, 11,  9, 11,  8, 10,  8, 10,  7,  9,  6,  8,  6,  8,  5,  7,  4,  6,  4,  6,  3,  5,  2,  4,  2,  4,  1,  3},
  // TODO: Inward funnel, outward funnel, stalagmite
};

// Defines how much variance each "cave" in a column is allowed.
static const uint8_t biome_variances[BIOME_COUNT] = {
  /* BIOME_OPEN_AREA */       2,
  /* BIOME_DOWNWARD_STRIPE */ 2,
  /* BIOME_UPWARD_STRIPE */   2,
};

// Defines which biomes are valid after the current biome.
// TODO: Fix this.
static const uint8_t next_possible_biomes[BIOME_COUNT][1] = {
  /* BIOME_OPEN_AREA */       {BIOME_DOWNWARD_STRIPE},
  /* BIOME_DOWNWARD_STRIPE */ {BIOME_UPWARD_STRIPE},
  /* BIOME_UPWARD_STRIPE */   {BIOME_OPEN_AREA},
};

void generate_next_column(struct GenerationState* gen_state, uint8_t* coll_map, uint8_t* bkg_map) BANKED {
  // Add tiles to collision and background maps.
  uint8_t cave_top = biome_columns[gen_state->biome_id][gen_state->biome_column_index * 2];
  uint8_t cave_bottom = biome_columns[gen_state->biome_id][gen_state->biome_column_index * 2 + 1];
  uint16_t n;

  for (uint16_t row = 0; row < COLUMN_HEIGHT; ++row) {
    // TODO: Add random variance.
    uint16_t map_index = row*ROW_WIDTH;
    if (row < cave_top || row > cave_bottom) {
      // Create a block.
      coll_map[map_index] = BLOCK_HEALTH;
      bkg_map[map_index] = MAPBLOCK_IDX;
      continue;
    }

    n = randw();
    if (n > 65300){
      // Create a health tile.
      coll_map[map_index] = HELATH_KIT_HEALTH;
      bkg_map[map_index] = HEALTH_KIT_TILE;
    }
    else if (n > 65000) {
      // Create a shield tile.
      coll_map[map_index] = SHIELD_HEALTH;
      bkg_map[map_index] = SHIELD_TILE;
    }
    else if (n > 55000){
      // Create a mine tile.
      coll_map[map_index] = MINE_HEALTH;
      bkg_map[map_index] = MINE_IDX;
    }
    else{
      // Create an empty tile.
      coll_map[map_index] = 0;
      bkg_map[map_index] = EMPTY_TILE_IDX;
    }
  }

  // Update GenerationState.
  if (++gen_state->biome_column_index >= COLUMNS_PER_BIOME) {
    gen_state->biome_id = next_possible_biomes[gen_state->biome_id][0];  // TODO: Add randomness.
    gen_state->biome_column_index = 0;
  }
}
