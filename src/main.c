#include <gb/gb.h>
#include <gbdk/font.h>
#include <rand.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gbdk/bcd.h>

#include "hUGEDriver.h"
#include "common.h"
#include "procedural_generation.h"
#include "score.h"
#include "sound_effects.h"

#include "font_extras_tiles.h"
#include "sprites.h"
#include "title_screens.h"
#include "tutorial_screen_tiles.h"
#include "tutorial_screen_map.h"
#include "player_sprites.h"
#include "player_shield_sprites.h"
#include "block_tiles.h"
#include "progressbar_tiles.h"
#include "projectiles_sprites.h"
#include "powerups_tiles.h"

static const uint8_t blank_win_tiles[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static bool game_paused = true;

static void increment_timer_score_isr(void) {
  if (game_paused) {
    return;
  }
  increment_timer_score();
}

void wait(uint8_t num_frames) {
  for (uint8_t i = 0; i < num_frames; ++i) {
    vsync();
  }
}

void fadeout(void) {
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

void fadein(void) {
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

void show_gameover_screen(void) {
  set_bkg_tiles(0,0,20,18,gameover_titlescreen);
  move_bkg(0,0);

  display_gameover_scores();

  SHOW_BKG;
  fadein();
  waitpad(J_START);
  waitpadup();
  fadeout();
  HIDE_BKG;
}

// Updates the collision map and background map in response to a dropped bomb. The bomb explosion
// is a square in front of the player with sides of length `2*BOMB_RADIUS+1` tiles.
void drop_bomb(const struct Sprite *player, uint8_t *coll_map, uint8_t *bkg_map) {
  uint8_t row_count = (BOMB_RADIUS * 2) + 1;  // +1 for the center row, which is centered on the ship.
  uint8_t row_top = (player->y - SCREEN_T) >> 3;
  // Because of integer division, the bomb explosion can look like it's not centered with the ship.
  // The following code fixes that.
  uint8_t row_pixel_delta = (player->y - SCREEN_T) - (row_top << 3);
  if (row_pixel_delta >= 4) {
    ++row_top;
  }

  if (row_top >= BOMB_RADIUS) {
    row_top -= BOMB_RADIUS;
  } else {
    // The player is at the top of the screen, so we need to shorten the bomb explosion so that it
    // doesn't go off screen.
    row_count = row_count - (BOMB_RADIUS - row_top);
    row_top = 0;
  }
  uint8_t col_left = (SCX_REG >> 3) + ((player->x - SCREEN_L) >> 3) + 1;  // Add 1 to be in front of the player.
  uint8_t incremental_score = 0;
  for (uint8_t i = 0; i < row_count; ++i) {
    uint8_t row = row_top + i;
    if (row >= COLUMN_HEIGHT) {
      // No need to calculate or draw the part of the bomb explosion that's off screen.
      break;
    }
    uint16_t row_offset = MAP_ARRAY_INDEX_ROW_OFFSET(row);
    for (uint8_t j = 0; j < (BOMB_RADIUS * 2) + 1; ++j) {
      uint8_t col = col_left + j;
      if (col >= ROW_WIDTH) {
        // We're past the edge of the collision and background maps and need to wrap around.
        col -= ROW_WIDTH;
      }

      uint16_t idx = col + row_offset;
      // If we are destroying a wall or a mine, add to the score.
      if (bkg_map[idx] == MAPBLOCK_IDX || bkg_map[idx] == MINE_IDX) {
        incremental_score += 1;
      }
      coll_map[idx] = 0;
      bkg_map[idx] = CRATERBLOCK_IDX;
    }
  }
  increment_point_score(incremental_score);
}

uint8_t check_collisions(struct Sprite *sprite, uint8_t *coll_map, uint8_t *bkg_map, uint8_t player_sprite, uint8_t pickups_only){
  /*
   * The player sprite can collide with up to 4 tiles.
   * Check the collision map on the top_left, top_right
   * bottom_left, and bottom right corners.
   */

  uint16_t idx_topl, idx_topr; 
  uint16_t idx_botl, idx_botr; 
  uint16_t row_topl, row_topr;
  uint16_t row_botl, row_botr;
  uint16_t col_topl, col_topr;
  uint16_t col_botl, col_botr;
  uint8_t collision = 0;
  uint8_t collision_block = 0;
  int8_t block_health = 0;

  row_topl = (sprite->cb.y - 16) / 8;
  col_topl = ((SCX_REG + sprite->cb.x - 8) %256) / 8;
  idx_topl = col_topl + row_topl*32;

  row_topr = row_topl;
  col_topr = ((SCX_REG + sprite->cb.x + sprite->cb.w - 8)%256) / 8;
  idx_topr = col_topr + row_topr*32;

  row_botl = ((sprite->cb.y + sprite->cb.h - 16)%256) / 8;
  col_botl = col_topl; 
  idx_botl = col_botl + row_botl*32;

  row_botr = row_botl;
  col_botr = col_topr; 
  idx_botr = col_botr + row_botr*32;

  if (coll_map[idx_topr] > 0) {
    collision = coll_map[idx_topr]; 
    collision_block = bkg_map[idx_topr];

    // Check if it is not a power up
    if ((collision < 235) && (!pickups_only)){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_topr] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_topr] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & UP){
          // Top Right collision detected while sprite is moving up 
          sprite->y += 8;
        }
        if (sprite->dir & RIGHT){
          // Top Right collision detected while sprite is moving up 
          sprite->x -= 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_topr] = 0;
        coll_map[idx_topr] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_topr] = block_health;
      }
    }
    else if ((collision < 235) && (pickups_only)){
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_topr] = 0;
    }
    else if ((collision >= 235) && (player_sprite)) {
      // Pick up power up
      bkg_map[idx_topr] = 0;
      coll_map[idx_topr] = 0;
    }
  }
  else if (coll_map[idx_botr] > 0){
    collision = coll_map[idx_botr];
    collision_block = bkg_map[idx_botr];

    // Check if it is not a power up
    if ((collision < 235) && (!pickups_only)){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_botl] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_botl] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & DOWN){
          // Bottom Right collision detected while sprite is moving down 
          sprite->y -= 8;
        }
        if (sprite->dir & RIGHT){
          // Bottom Right collision detected while sprite is moving down 
          sprite->x -= 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_botr] = 0;
        coll_map[idx_botr] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_botr] = block_health;
      }
    }
    else if ((collision < 235) && (pickups_only)){
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_botr] = 0;
    }
    else if ((collision >= 235) && (player_sprite)) {
      // Pick up power up
      bkg_map[idx_botr] = 0;
      coll_map[idx_botr] = 0;
    }
  }
  else if ((player_sprite) && (coll_map[idx_topl] > 0)){
    // Only the player has left side collisions
    collision = coll_map[idx_topl];
    collision_block = bkg_map[idx_topl];

    // Check if it is not a power up
    if ((collision < 235) && (!pickups_only)){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_topl] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_topl] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & UP){
          // Top Left collision detected while sprite is moving up 
          sprite->y += 8;
        }
        if (sprite->dir & LEFT){
          // Top Left collision detected while sprite is moving up 
          sprite->x += 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_topl] = 0;
        coll_map[idx_topl] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_topl] = block_health;
      }
    }
    else if ((collision < 235) && (pickups_only)){
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_topl] = 0;
    }
    else if ((collision >= 235) && (player_sprite)) {
      // Pick up power up
      bkg_map[idx_topl] = 0;
      coll_map[idx_topl] = 0;
    }
  }
  else if ((player_sprite) && (coll_map[idx_botl] > 0)){
    // Only the player has left side collisions
    collision = coll_map[idx_botl];
    collision_block = bkg_map[idx_botl];

    // Check if it is not a power up
    if ((collision < 235) && (!pickups_only)){
      // Apply damage to the background element
      if (player_sprite){
        block_health = coll_map[idx_botl] - 2;
      }
      else{
        // Bullet 
        block_health = coll_map[idx_botl] - 1;
      }

      if (block_health > 0) {
        // Hit a wall
        if (sprite->dir & DOWN){
          // Bottom Left collision detected while sprite is moving down 
          sprite->y -= 8;
        }
        if (sprite->dir & LEFT){
          // Bottom Left collision detected while sprite is moving down 
          sprite->x += 8;
        }
      }

      if (block_health <= 0) {
        // Background element destroyed
        // Replace obstacle tile and remove collision
        bkg_map[idx_botl] = 0;
        coll_map[idx_botl] = 0;

        if (collision_block == MAPBLOCK_IDX + 2){
          increment_point_score(2);
        }
      }
      else {
        coll_map[idx_botl] = block_health;
      }
    }
    else if ((collision < 235) && (pickups_only)){
      // Clear collision for this object so we dont trigger
      // on the next check (we entered this object with immunity
      // so we can skip colliding with it later)
      coll_map[idx_botl] = 0;
    }
    else if ((collision >= 235) && (player_sprite)) {
      // Pick up power up
      bkg_map[idx_botl] = 0;
      coll_map[idx_botl] = 0;
    }
  } 
  
  if ((collision > 0) && (block_health > 0) && (player_sprite)){
    // Move the player sprite after collision
    move_sprite(sprite->sprite_id, sprite->x, sprite->y);
  }

  return collision;
}

void update_health_bar(struct Sprite *player, uint8_t *progressbar_tiles, uint8_t *player_sprite_base_id){
  uint8_t i, idx;
  if (player->health == 100){
    progressbar_tiles[0] = HEALTH_BAR_START; // left edge of bar
    for (i = 1; i < 7; i++){
      progressbar_tiles[i] = HEALTH_BAR_START + 1; // center of bar
    }
    progressbar_tiles[7] = HEALTH_BAR_START + 2; // right edge of bar
  }
  else if (player->health >= 88) {
    progressbar_tiles[0] = HEALTH_BAR_START; // left edge of bar
    for (i = 1; i < 7; i++){
      progressbar_tiles[i] = HEALTH_BAR_START + 1; // center of bar
    }
    progressbar_tiles[7] = HEALTH_BAR_START + 5; // right edge of bar
  }
  else if (player->health >= 16) {
    idx = player->health / 12;
    progressbar_tiles[0] = HEALTH_BAR_START; // left edge of bar
    for (i=1; i < 7; i++){
      if (i < idx){
        progressbar_tiles[i] = HEALTH_BAR_START + 1; // fill 
      }
      else {
        progressbar_tiles[i] = HEALTH_BAR_START + 4; // clear
      }
    }
    progressbar_tiles[7] = HEALTH_BAR_START + 5; // clear right edge of bar
  }
  else if (player->health > 0){
    progressbar_tiles[1] = HEALTH_BAR_START + 4; 
    progressbar_tiles[0] = HEALTH_BAR_START;
  }
  else{
    progressbar_tiles[1] = HEALTH_BAR_START + 4; // Clear bottom 2 tiles 
    progressbar_tiles[0] = HEALTH_BAR_START + 3;
  }
  set_win_tiles(0, 0, 8, 1, progressbar_tiles);

  if (player->health > 50){
    *player_sprite_base_id = 0;
  }
  else if (player->health > 25){
    *player_sprite_base_id = 3;
  }
  else if (player->health > 0){
    *player_sprite_base_id = 6;
  }
}

void main(void) {
  /*
   * Load background and sprite data
   */
  // Load font tiles to background map
  font_t min_font;
  font_init();
  min_font = font_load(font_min);
  font_set(min_font);

  // Load background tiles
  uint8_t tile_index = MAPBLOCK_IDX;
  set_bkg_data(tile_index, sizeof(block_tiles)/TILE_SIZE_BYTES, block_tiles);
  tile_index += sizeof(block_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(powerups_tiles)/TILE_SIZE_BYTES, powerups_tiles);
  tile_index += sizeof(powerups_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(progressbar_tiles)/TILE_SIZE_BYTES, progressbar_tiles);
  tile_index += sizeof(progressbar_tiles)/TILE_SIZE_BYTES;
  // Note: This is hardcoded to load only 1 tile for some reason, instead of loading all the tiles in
  // font_extras_tiles.
  set_bkg_data(tile_index, 1, font_extras_tiles);
  tile_index += 1;
  set_bkg_data(tile_index, sizeof(tutorial_screen_tiles)/TILE_SIZE_BYTES, tutorial_screen_tiles);

  // Load sprite data
  set_sprite_data(0,10,player_data);
  set_sprite_data(10,9,player_shield_data);
  set_sprite_data(19,3,projectiles_data);

  // Load title screen
  set_bkg_tiles(0,0,20,COLUMN_HEIGHT,game_titlescreen);

  // Load Window
  move_win(7,136);

  /**
   * Load music 
   */
  extern const hUGESong_t intro_song;
  extern const hUGESong_t main_song;
  extern const hUGESong_t main_song_fast;

  // Enable sound playback.
  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x33;

  // Mute all channels.
  mute_all_channels();
  
  // Add hUGE driver to VBL interrupt handler.
  add_VBL(hUGE_dosound);

  // Add timer score incrementer to VBL interrupt handler.
  add_VBL(increment_timer_score_isr);

  /*
   * Turn on display and show background 
   */
  DISPLAY_ON;
  SHOW_BKG;

  uint8_t bomb_tiles[2];
  uint8_t progressbar_tiles[8];
  bool show_time = true;

  // Initialize high scores.
  if (init_highscores()) {
    display_highscores();
    SHOW_WIN;
  }

  struct Sprite player;
  struct Sprite bullets[MAX_BULLETS];
  uint8_t active_bullet_count = 0;
  bool bomb_dropped;
  bool shield_active;
  enum powerup active_powerup;
  uint8_t n_bombs;

  // Collision and background maps
  uint8_t coll_map[COLUMN_HEIGHT*ROW_WIDTH];
  uint8_t bkg_map[COLUMN_HEIGHT*ROW_WIDTH];
  bool copy_bkgmap_to_vram = false;

  // Map generation variables.
  struct GenerationState gen_state;
  uint8_t gen_column_index;  // The index of the next column to generate.

  uint8_t input;
  uint8_t prev_input;
  uint8_t dx;
  uint8_t dy;
  uint8_t frame_count;
  uint8_t scroll_count;
  int16_t damage_animation_counter; // When hit, skip checking collisions for this long
  uint8_t col_count;  // counter for number of columns scrolled. Used to calculate screen_count
  uint16_t screen_count; // number of screens scrolled
  uint8_t scroll_frames_per_pixel;  // Slower scrolling: How many frames to wait before scrolling one pixel
  uint8_t scroll_pixels_per_frame;  // Faster scrolling: How many pixels to scroll each frame
  uint8_t scroll_frames_count;  // Counts frames for scroll_frames_per_pixel scrolling
  uint8_t player_collision;
  uint8_t bullet_collision;
  enum animation_state damage_animation_state; // Used during the damage recovery to toggle between showing and hidding the player sprite

  uint8_t player_sprite_base_id;

  // Temporary variables (loop counters, array indices, etc)
  uint8_t i, j;
  uint16_t ii;

  // Banking variables
  uint8_t last_bank;

  while (true) {
    // Load the window contents
    show_time = true;
    n_bombs = MAX_BOMBS;

    bomb_tiles[0] = BOMB_ICON_IDX;
    bomb_tiles[1] = n_bombs + 1;

    progressbar_tiles[0] = HEALTH_BAR_START; // left edge of bar
    for (i = 1; i < 7; i++){
      progressbar_tiles[i] = HEALTH_BAR_MIDDLE; // center of bar
    }
    progressbar_tiles[7] = HEALTH_BAR_END; // right edge of bar

    /*
    * Create a player and display the sprite 
    */
    player.sprite_id = 0;
    player.sprite_tile_id = 0;
    player.x = 20;
    player.y = 72 + 8;
    player.speed = 1;
    player.dir = RIGHT;
    player.cb_x_offset = 1;
    player.cb_y_offset = 2;
    player.cb.x = player.x + player.cb_x_offset;
    player.cb.y = player.y + player.cb_y_offset;
    player.cb.w = 5;
    player.cb.h = 4;
    player.health = 100;
    player.active = true;
    player.type = PLAYER;
    move_sprite(player.sprite_id, player.x, player.y);

    bomb_dropped = false;
    shield_active = false;
    active_powerup = GUN;

    // Create bullets.
    for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
      struct Sprite* b = &(bullets[i]);
      b->active = false;
      b->type = BULLET;
      b->sprite_id = i + 1;  // +1 so we don't override the player (always sprite_id 0)
      b->sprite_tile_id = 19;
      b->lifespan = 0;
      b->speed = 4;
      b->x = 0;
      b->y = 0;
      // Collision box
      b->cb_x_offset = 0;
      b->cb_y_offset = 2;
      b->cb.x = b->x + b->cb_x_offset;
      b->cb.y = b->y + b->cb_y_offset;
      b->cb.h = 4;
      b->cb.w = 4;
      set_sprite_tile(b->sprite_id, b->sprite_tile_id);
      move_sprite(b->sprite_id, b->x, b->y);
    }
  
    // Clear collision map and background map
    for (ii=0; ii<COLUMN_HEIGHT*ROW_WIDTH; ++ii) {
      coll_map[ii] = 0;
      bkg_map[ii] = 0;
    }

    /*
    * Game Loop
    */
    input = 0;
    prev_input = 0;
    dx = 0;
    dy = 0;
    frame_count = 0;
    scroll_count = 0;
    damage_animation_counter = 0; // When hit, skip checking collisions for this long
    col_count = 0;  // counter for number of columns scrolled. Used to calculate screen_count
    screen_count = 0; // number of screens scrolled
    scroll_frames_per_pixel = 0;
    scroll_pixels_per_frame = 1;
    scroll_frames_count = 0;
    player_collision = 0;
    bullet_collision = 0;
    damage_animation_state = HIDDEN;
    gen_state.biome_id = 0;
    gen_state.biome_column_index = 0;
    gen_column_index = 20;

    // Title Screen
    hUGE_init(&intro_song);
    play_all_channels();
    waitpad(J_START);
    waitpadup();

    // Speed Selection Screen    
    set_bkg_tiles(0,0,20,COLUMN_HEIGHT,speed_titlescreen);
    set_sprite_tile(0, 0);
    move_sprite(0, 32, 72);
    SHOW_SPRITES;

    while (true) {
      input = joypad();
      if (KEY_FIRST_PRESS(input, prev_input, J_UP) || KEY_FIRST_PRESS(input, prev_input, J_RIGHT)) {
        if (scroll_pixels_per_frame == 1) {
          move_sprite(0, 32, 88);
          scroll_pixels_per_frame = 2;
        }
        else {
          move_sprite(0, 32, 72);
          scroll_pixels_per_frame = 1;
        }
      }
      else if (KEY_FIRST_PRESS(input, prev_input, J_DOWN) || KEY_FIRST_PRESS(input, prev_input, J_LEFT)) {
        if (scroll_pixels_per_frame == 2) {
          move_sprite(0, 32, 72);
          scroll_pixels_per_frame = 1;
        }
        else {
          move_sprite(0, 32, 88);
          scroll_pixels_per_frame = 2;
        }
      }
      else if (KEY_FIRST_PRESS(input, prev_input, J_START) || KEY_FIRST_PRESS(input, prev_input, J_A) || KEY_FIRST_PRESS(input, prev_input, J_B)) {
        break;
      }
      vsync();
      prev_input = input;
    }
    HIDE_SPRITES;

    // Game Start
    // Set player start location
    move_sprite(player.sprite_id, player.x, player.y);

    // Copy tutorial screen to bkg_map
    for (i=0; i<COLUMN_HEIGHT; i++){
      for (j=0; j<SCREEN_TILE_WIDTH; j++){
        bkg_map[i*ROW_WIDTH+j] = tutorial_screen_map[i*SCREEN_TILE_WIDTH+j];
      }
    }
    set_bkg_tiles(0, 0, 32, COLUMN_HEIGHT, bkg_map);

    set_win_tiles(0, 0, 20, 1, blank_win_tiles); 
    set_win_tiles(9, 0, 2, 1, bomb_tiles);
    set_win_tiles(0, 0, 8, 1, progressbar_tiles);
    mute_all_channels();

    wait(10);
    
    if (scroll_pixels_per_frame == 1){
      hUGE_init(&main_song);
    }
    else{
      hUGE_init(&main_song_fast);
    }
    play_all_channels();

    SHOW_SPRITES;
    SHOW_WIN;
    initrand(DIV_REG);

    last_bank = CURRENT_BANK;
    SWITCH_ROM(1);
    for (uint8_t i = 0; i < ROW_WIDTH - SCREEN_TILE_WIDTH; ++i) {
      generate_next_column(&gen_state, coll_map+gen_column_index, bkg_map+gen_column_index);
      if (++gen_column_index >= ROW_WIDTH) {
        gen_column_index = 0;
      }
    }
    set_bkg_tiles(0, 0, 32, COLUMN_HEIGHT, bkg_map);
    SWITCH_ROM(last_bank);

    wait(15);

    player_sprite_base_id = 0;

    copy_bkgmap_to_vram = false;

    // Reset scores and window tiles.
    reset_scores();
    display_timer_score();

    game_paused = false;

    while (true) {
      input = joypad();
      player_collision = 0;
      bullet_collision = 0;
      copy_bkgmap_to_vram = false;

      if (KEY_PRESSED(input, J_START)) {
        game_paused = true;
        mute_all_channels();
        waitpadup();
        waitpad(J_START);
        waitpadup();
        play_all_channels();
        damage_animation_state = SHOWN;
        game_paused = false;
        continue;
      }

      if (KEY_FIRST_PRESS(input, prev_input, J_SELECT)) {
        show_time = !show_time;
      }

      dx = 0;
      dy = 0;
      player.sprite_tile_id = player_sprite_base_id;
      player.dir = RIGHT;
      // Reset player collision box to default.
      player.cb_x_offset = 1;
      player.cb_y_offset = 2;
      player.cb.h = 4;
      player.cb.w = 5;
      if (KEY_PRESSED(input, J_RIGHT)) {
        dx = player.speed;
      }
      if (KEY_PRESSED(input, J_LEFT)) {
        dx = -player.speed;
        player.dir |= LEFT;
      }
      if (KEY_PRESSED(input, J_UP)) {
        dy = -player.speed;
        player.sprite_tile_id = player_sprite_base_id + 1;
        player.dir |= UP;
        
        // Make collision box smaller when plane is "tilted".
        player.cb_x_offset = 2;
        player.cb_y_offset = 3;
        player.cb.h = 1;
        player.cb.w = 3;
      }
      if (KEY_PRESSED(input, J_DOWN)) {
        dy = player.speed;
        player.sprite_tile_id = player_sprite_base_id + 2;
        player.dir |= DOWN;

        // Make collision box smaller when plane is "tilted".
        player.cb_x_offset = 2;
        player.cb_y_offset = 4;
        player.cb.h = 1;
        player.cb.w = 3;
      }

      if (KEY_FIRST_PRESS(input, prev_input, J_A) && active_bullet_count < MAX_BULLETS) {
        // Find first non-active bullet in `bullets` array and activate it.
        for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
          if (bullets[i].active) {
            continue;
          }
          struct Sprite* b = &(bullets[i]);
          b->active = true;
          b->lifespan = BULLET_LIFESPAN;
          b->x = player.x;
          b->cb.x = b->x + b->cb_x_offset;
          b->y = player.y;
          b->cb.y = b->y + b->cb_y_offset;
          move_sprite(b->sprite_id, b->x, b->y);
          ++active_bullet_count;
          play_gun_sound();
          break;
        }
      }

      if (KEY_FIRST_PRESS(input, prev_input, J_B) && n_bombs > 0) {
        bomb_dropped = true;
        n_bombs--;
        play_bomb_sound();
      }

      prev_input = input;

      // Update player position
      player.x += dx; 
      // Bounds check
      if (player.x < SCREEN_L) {
        player.x = SCREEN_L;
      } else if (player.x > SCREEN_R) {
        player.x = SCREEN_R;
      }
      
      player.y += dy; 
      if (player.y < SCREEN_T) {
        player.y = SCREEN_T;
      } else if (player.y > SCREEN_B) {
        player.y = SCREEN_B;
      }

      // Update collision box
      player.cb.x = player.x + player.cb_x_offset;
      player.cb.y = player.y + player.cb_y_offset;

      set_sprite_tile(player.sprite_id, player.sprite_tile_id);
      move_sprite(player.sprite_id, player.x, player.y);
      
      /**
       * Update bullets, if any are active.
       */
      if (active_bullet_count != 0) {
        // Move bullets.
        for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
          if (!bullets[i].active) {
            continue;
          }
          struct Sprite* b = &(bullets[i]);
          b->x += b->speed;
          b->cb.x = b->x + b->cb_x_offset;
          b->lifespan--;
          if (b->x > SCREEN_R || b->lifespan == 0) {
            // Hide sprite.
            b->active = false;
            b->x = 0;
            b->y = 0;
            --active_bullet_count;
          } else {
            bullet_collision = check_collisions(b, coll_map, bkg_map, false, false);
            // Check that the bullet collided with something it can destroy
            if (bullet_collision > 0 && bullet_collision < 235) {
              // Hide sprite.
              b->active = false;
              b->x = 0;
              b->y = 0;
              --active_bullet_count;
            }
          }
          // Update bullet's sprite position.
          move_sprite(b->sprite_id, b->x, b->y);
        }
      }

      if (bomb_dropped){
        drop_bomb(&player, coll_map, bkg_map);
        copy_bkgmap_to_vram = true;
        bomb_dropped = false;
      }

      /*
       * Continue processing 
       */

      if (damage_animation_counter == 0){
        // Damage animation or shield powerup expired.
        // Reset so that player is shown
        if (shield_active){
          shield_active = false;
          player_sprite_base_id -= 10;
        }

        if (damage_animation_state == HIDDEN){
          // SHOW_SPRITES;
          move_sprite(player.sprite_id, player.x, player.y);
          damage_animation_state = SHOWN;
        }

        // Normal collision check for player
        player_collision = check_collisions(&player, coll_map, bkg_map, true, false);
        if ((player_collision > 0) && (player_collision < 235)) {
          player.health -= COLLISION_DAMAGE;
          damage_animation_counter = COLLISION_TIMEOUT;
          player_collision = 1;
        }
        else if (player_collision == HEALTH_KIT_ID) {
          if (player.health < 100){
            // Prevent health overflow (int8 maxes at 128)
            if ((100 - player.health) >= HEALTH_KIT_VALUE){
              if(player.health < 20){
                player.health += 4*HEALTH_KIT_VALUE;
              }
              else if (player.health < 50){
                player.health += 2*HEALTH_KIT_VALUE;
              }
              else{
                player.health += HEALTH_KIT_VALUE;
              }
            }
            else {
              player.health += 100 - player.health;
            }
          }
          player_collision = 1;
          play_health_sound();
        }
        else if (player_collision == SHIELD_ID) {
          shield_active = true;
          damage_animation_counter = SHIELD_DURATION;
          player_sprite_base_id += 10;
          player_collision = 0;
          play_shield_sound();
        }

        if (player.health <= 0){
          game_paused = true;

          mute_all_channels();
          player.sprite_tile_id = 9;
          set_sprite_tile(player.sprite_id, player.sprite_tile_id);
          wait(10);
          HIDE_BKG;
          play_gameover_sound();
          fadeout();
          HIDE_SPRITES;

          // Hide all bullets.
          for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
            struct Sprite* b = &(bullets[i]);
            b->active = false;
            b->x = 0;
            b->y = 0;
            move_sprite(b->sprite_id, b->x, b->y);
          }
          active_bullet_count = 0;

          update_health_bar(&player, progressbar_tiles, &player_sprite_base_id);
          show_gameover_screen();

          // Load title screen
          set_bkg_tiles(0,0,20,COLUMN_HEIGHT,game_titlescreen);
          move_bkg(0,0);
          player_sprite_base_id = 0; // Reset to initial sprite
          player.sprite_tile_id = player_sprite_base_id;
          set_sprite_tile(player.sprite_id, player.sprite_tile_id);
          display_highscores();
          SHOW_WIN;
          SHOW_BKG;
          wait(60);
          fadein();
          break;
        }

        // Update health bar after a collision
        if (player_collision){
          update_health_bar(&player, progressbar_tiles, &player_sprite_base_id);
        }
      }
      else {
        damage_animation_counter--;
        if (damage_animation_counter < 0){
          damage_animation_counter = 0;
        }

        if (!shield_active){
          if (damage_animation_state == HIDDEN){
            // SHOW_SPRITES;
            move_sprite(player.sprite_id, player.x, player.y);
            damage_animation_state = SHOWN;
          }
          else{
            // HIDE_SPRITES;
            move_sprite(player.sprite_id, 0, 0);
            damage_animation_state = HIDDEN;
          }

          // Check for collision and only process pickups if the shield is not active
          // This will allow the player to pick up items while the flashing animation is playing
          player_collision = check_collisions(&player, coll_map, bkg_map, true, true);

          if (player_collision == HEALTH_KIT_ID) {
            if (player.health < 100){
              // Prevent health overflow (int8 maxes at 128)
              if ((100 - player.health) >= HEALTH_KIT_VALUE){
                if(player.health < 20){
                  player.health += 4*HEALTH_KIT_VALUE;
                }
                else if (player.health < 50){
                  player.health += 2*HEALTH_KIT_VALUE;
                }
                else{
                  player.health += HEALTH_KIT_VALUE;
                }
              }
              else {
                player.health += 100 - player.health;
              }
            }
            player_collision = 1;
            play_health_sound();
          }
          else if (player_collision == SHIELD_ID) {
            shield_active = true;
            damage_animation_counter = SHIELD_DURATION;
            player_sprite_base_id += 10;
            player_collision = 0;
            play_shield_sound();
          }
        }
      }

      // Scroll the screen.
      if (scroll_frames_per_pixel != 0) {
        ++scroll_frames_count;
        if (scroll_frames_count == scroll_frames_per_pixel) {
          scroll_frames_count = 0;
          scroll_bkg(1, 0);
          ++scroll_count;
        }
      } else {
        scroll_bkg(scroll_pixels_per_frame, 0);
        scroll_count += scroll_pixels_per_frame;
      }

      if (scroll_count >= 8){
        scroll_count = 0;
        last_bank = CURRENT_BANK;
        SWITCH_ROM(1);
        generate_next_column(&gen_state, coll_map+gen_column_index, bkg_map+gen_column_index);
        if (++gen_column_index >= ROW_WIDTH) {
          gen_column_index = 0;
        }
        SWITCH_ROM(last_bank);
        copy_bkgmap_to_vram = true;

        ++col_count;
        if (col_count == 20) {
          col_count = 0;
          ++screen_count;
          increment_point_score(5);

          // Add a bomb every few screens we scroll.
          if (screen_count % 3 == 0 && n_bombs < MAX_BOMBS) {
            ++n_bombs;
          }

          // // Increase scroll speed after some time.
          // if (screen_count == 3) {
            // scroll_frames_per_pixel = 0;
            // scroll_frames_count = 0;
            // scroll_pixels_per_frame = 1;
          // } else if (screen_count == 400) {
            // mute_all_channels();
            // hUGE_init(&main_song_fast);
            // scroll_pixels_per_frame = 2;
            // player.speed = 2;
            // play_all_channels();
          // }
        }
      }

      if ((frame_count & 0x3) == 0) { // %4
        // Update HUD
        bomb_tiles[1] = n_bombs + 1;
        set_win_tiles(9, 0, 2, 1, bomb_tiles);
      }
      
      frame_count++;
      if (frame_count >= 255){
        frame_count = 0;
      }

      if ((player_collision) || (bullet_collision)){
        // Update bkg_map if there are collisions
        copy_bkgmap_to_vram = true;
      }

      // Wait for frame to finish drawing
      vsync();
      
      if (show_time) {
        display_timer_score();
      } else {
        display_point_score();
      }

      if (copy_bkgmap_to_vram){
        // Write the entire map to VRAM
        set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, bkg_map);
      }
    }
  }
}
