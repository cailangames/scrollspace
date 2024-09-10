#ifndef _PROCEDURAL_GENERATION_H_
#define _PROCEDURAL_GENERATION_H_

#pragma bank 1

#include "common.h"

extern uint8_t update_obstacle_max_width(uint8_t new_gap_w_min) BANKED;
extern int8_t new_gap_row_idx_offset(void) BANKED;
extern int8_t new_w_offset(void) BANKED;
extern uint8_t new_obstacle_idx(uint8_t gap_w, uint8_t gap_row_idx) BANKED;
extern void generate_new_column(uint8_t *col_idx, uint8_t *gap_row_idx, 
                         uint8_t *gap_w, uint8_t *new_column, 
                         uint8_t gap_w_min, uint8_t obs_w_max,
                         uint8_t *coll_map, uint8_t *bkg_map) BANKED;
                         
#endif