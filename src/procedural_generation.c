#pragma bank 1

#include <gb/gb.h>
#include <rand.h>
#include <stdlib.h>
#include <stdbool.h>
#include "procedural_generation.h"

uint8_t update_obstacle_max_width(uint8_t new_gap_w_min) BANKED{
  uint8_t obstacle_max = new_gap_w_min / 3;
  if (obstacle_max == 0){
    obstacle_max = 1;
  }

  return obstacle_max;
}

int8_t new_gap_row_idx_offset(void) BANKED{
  uint8_t n = rand();
  int8_t idx_offset;
  
  if (n < 24) {
    idx_offset = 0;
  }
  else if (n < 140){
    idx_offset = 1;
  }
  else {
    idx_offset = -1;
  }

  return idx_offset;
}

int8_t new_w_offset(void) BANKED{
  uint8_t n = rand();
  int8_t w_offset;
  
  if (n < 24) {
    w_offset = 1;
  }
  else if (n < 140){
    w_offset = -1;
  }
  else {
    w_offset = 0;
  }

  return w_offset;
}

uint8_t new_obstacle_idx(uint8_t gap_w, uint8_t gap_row_idx) BANKED{
  uint16_t n = (gap_w * rand())/255 + gap_row_idx;

  return (uint8_t) n;
}

void generate_new_column(uint8_t *col_idx, uint8_t *gap_row_idx, 
                         uint8_t *gap_w, uint8_t *new_column, 
                         uint8_t gap_w_min, uint8_t obs_w_max,
                         uint8_t *coll_map, uint8_t *bkg_map) BANKED{
                         
  // initrand(DIV_REG);
  uint8_t n;  // random number
  uint8_t i;  // Loop counter
  int8_t tmp_gap_row_idx;
  int8_t idx_offset = 0;
  int8_t w_offset = 0;
  uint8_t tmp_col_idx;
  
  // Generate a random number to determine if we are going to 
  // update both idx and w, or just one of them
  n = rand();

  if (n < 24) {
    idx_offset = new_gap_row_idx_offset();
    w_offset = new_w_offset();
  }
  else if (n < 140) {
    idx_offset = new_gap_row_idx_offset();
  }
  else {
    w_offset = new_w_offset();
  }
  
  // Update the gap_row_idx and the gap_w
  tmp_gap_row_idx = *gap_row_idx + idx_offset;
  *gap_w = *gap_w + w_offset;

  // Bound checking
  if (tmp_gap_row_idx < 1){
    *gap_row_idx = 1; // leave 1 row at top
  }
  else if (tmp_gap_row_idx > (COLUMN_HEIGHT - 2 - gap_w_min)) {
    *gap_row_idx = (COLUMN_HEIGHT - 2 - gap_w_min);  // leave 1 row at bottom
  }
  else {
    *gap_row_idx = (uint8_t) tmp_gap_row_idx;
  }

  if (*gap_w < gap_w_min){
    *gap_w = gap_w_min;
  }
  else if (*gap_w > COLUMN_HEIGHT) {
    *gap_w = COLUMN_HEIGHT;
  }
  obs_w_max = update_obstacle_max_width(*gap_w);

  // Generate new column
  for (i=0; i<COLUMN_HEIGHT;i++){
    if ((i >= *gap_row_idx) && (i < *gap_row_idx+*gap_w)){
      new_column[i] = 0;
      bkg_map[(*col_idx) + i*32] = 0;
      // coll_map[(*col_idx)*COLUMN_HEIGHT + i] = 0;
      coll_map[(*col_idx) + i*32] = 0;
    }
    else{
      new_column[i] = MAPBLOCK_IDX;
      bkg_map[(*col_idx) + i*32] = MAPBLOCK_IDX;
      // coll_map[(*col_idx)*COLUMN_HEIGHT + i] = 3;
      coll_map[(*col_idx) + i*32] = 3;
    }
  }

  // Get another random number to see if we should add
  // Obstacles in the gap
  n = rand();
  if (n < 96){
    uint8_t idx = new_obstacle_idx(*gap_w, *gap_row_idx);
    if ((idx > *gap_row_idx) && (idx < *gap_row_idx + *gap_w)){
      for (i=0; i<obs_w_max; i++){
        new_column[idx+i] = MAPBLOCK_IDX + 2;
        bkg_map[(*col_idx) + (idx+i)*32] = MAPBLOCK_IDX + 2;
        // coll_map[(*col_idx)*COLUMN_HEIGHT + idx + i] = 1; 
        coll_map[(*col_idx) + (idx+i)*32] = 1; 
      }
    }
  }
  else {
    // Drop a powerup in the gap
    if ((n > 250) && (n < 253)) {
      // Shield
      new_column[*gap_row_idx] = MAPBLOCK_IDX + 6;
      bkg_map[(*col_idx) + (*gap_row_idx + (*gap_w >> 1))*32] = MAPBLOCK_IDX + 6;
      coll_map[(*col_idx) + (*gap_row_idx + (*gap_w >> 1))*32] = 254;
      // coll_map[(*col_idx)*COLUMN_HEIGHT + (*gap_row_idx +(*gap_w >> 1))] = 254;
    }
    else if (n >= 253) {
      // Health
      new_column[*gap_row_idx] = MAPBLOCK_IDX + 7;
      bkg_map[(*col_idx) + (*gap_row_idx + (*gap_w >> 1))*32] = MAPBLOCK_IDX + 7;
      coll_map[(*col_idx) + (*gap_row_idx + (*gap_w >> 1))*32] = 255;
      // coll_map[(*col_idx)*COLUMN_HEIGHT + (*gap_row_idx + (*gap_w >> 1))] = 255;
    }
  }

  // Increment col_idx
  tmp_col_idx = *col_idx + 1;
  if (tmp_col_idx > 31){
    *col_idx = 0;
  }
  else{
    *col_idx = tmp_col_idx;
  }
}