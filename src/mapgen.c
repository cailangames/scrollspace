#include "mapgen.h"

#include <gb/gb.h>
#include <rand.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "mapgen_data.h"
#include "text_data.h"
#include "tile_data.h"

// Current generation state variables
static uint8_t biome_id = 0;             // The ID of the biome that's currently being generated
static uint8_t biome_column_index = 0;   // The column index within the current biome being generated
static uint16_t pickup_probability = 0;  // The probability of spawning a pickup in a generated map cell

void reset_generation_state(void) {
  // Start at a random biome.
  biome_id = MOD64(rand());
  biome_column_index = 0;
  pickup_probability = (game_mode == NORMAL) ? PICKUP_PROBABILITY_NORMAL : (game_mode == HARD) ? PICKUP_PROBABILITY_HARD
                                                                                               : PICKUP_PROBABILITY_TURBO;
}

void decrease_pickup_probability(void) {
  pickup_probability -= PICKUP_PROBABILITY_DECREASE;
  if (pickup_probability < PICKUP_PROBABILITY_MIN) {
    pickup_probability = PICKUP_PROBABILITY_MIN;
  }
}

void generate_tutorial(void) {
  // Fill in background and collision maps.
  memset(collision_map, BLOCK_HEALTH, ROW_WIDTH);
  memset(collision_map + ROW_WIDTH, 0, ROW_WIDTH * (COLUMN_HEIGHT - 2));
  memset(collision_map + (ROW_WIDTH * (COLUMN_HEIGHT - 1)), BLOCK_HEALTH, ROW_WIDTH);
  memset(background_map, WALL_BLOCK_TILE, ROW_WIDTH);
  memset(background_map + ROW_WIDTH, EMPTY_TILE, ROW_WIDTH * (COLUMN_HEIGHT - 2));
  memset(background_map + (ROW_WIDTH * (COLUMN_HEIGHT - 1)), WALL_BLOCK_TILE, ROW_WIDTH);
  set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, background_map);

  // Copy tutorial screen tiles and text to the background.
  set_bkg_tiles(9, 4, 7, 9, tutorial_screen_map);
  set_bkg_tiles(16, 5, UINT8_ARRARY_SIZE(shoot_text), 1, shoot_text);
  set_bkg_tiles(16, 8, UINT8_ARRARY_SIZE(bomb_text), 1, bomb_text);
  set_bkg_tiles(16, 11, UINT8_ARRARY_SIZE(pause_text), 1, pause_text);
}

void generate_column(uint8_t column_idx) {
  // Add tiles to collision and background maps.
  uint8_t* coll_map = collision_map + column_idx;
  uint8_t* bkg_map = background_map + column_idx;
  uint8_t col = biome_columns[biome_id][biome_column_index];
  uint8_t cave_top = col >> 4;
  uint8_t cave_width = (col & 0x0F) + MINIMUM_CAVE_WIDTH - 1;
  uint8_t cave_bottom = cave_top + cave_width;
  uint16_t mine_probability = (biome_id >= WIDE_OPEN_BIOMES_START) ? MINE_PROBABILITY_WIDE_OPEN : MINE_PROBABILITY_NARROW;

  for (uint8_t row = 0; row < COLUMN_HEIGHT; ++row) {
    if (row < cave_top || row > cave_bottom) {
      // Create a block.
      *coll_map = BLOCK_HEALTH;
      *bkg_map = WALL_BLOCK_TILE;
    } else {
      uint16_t n = randw();
      if (n < UINT16_MAX - (mine_probability + (pickup_probability << 1))) {
        // Create an empty tile.
        *coll_map = 0;
        *bkg_map = EMPTY_TILE;
      } else if (n < UINT16_MAX - (pickup_probability << 1)) {
        // Create a mine tile.
        *coll_map = MINE_HEALTH;
        *bkg_map = MINE_TILE;
      } else if (n < UINT16_MAX - pickup_probability) {
        // Create a shield tile.
        *coll_map = SHIELD_ID;
        *bkg_map = SHIELD_TILE;
      } else {
        // Create a health tile.
        *coll_map = HEALTH_KIT_ID;
        *bkg_map = HEALTH_KIT_TILE;
      }
    }

    coll_map += ROW_WIDTH;
    bkg_map += ROW_WIDTH;
  }

  // Update current generation state.
  if (++biome_column_index >= COLUMNS_PER_BIOME) {
    uint8_t next_biome = MOD32(rand());
    biome_id = next_possible_biomes[biome_id][next_biome];
    biome_column_index = 0;
  }
}
