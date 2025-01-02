#include <rand.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <gb/gb.h>
#include <gbdk/bcd.h>
#include <gbdk/font.h>
#include <hUGEDriver.h>

#include "collision.h"
#include "common.h"
#include "procedural_generation.h"
#include "score.h"
#include "sound_effects.h"
#include "sprites.h"

// Sprite data
#include "player_shield_sprites.h"
#include "player_sprites.h"
#include "projectiles_sprites.h"

// Tile data
#include "block_tiles.h"
#include "font_extras_tiles.h"
#include "powerups_tiles.h"
#include "lock_tiles.h"
#include "progressbar_tiles.h"
#include "title_screens.h"
#include "tutorial_screen_map.h"
#include "tutorial_screen_tiles.h"
#include "cailan_games_logo_tiles.h"
#include "cailan_games_logo_map.h"

// The following are used for performance testing of individual components.
#define ENABLE_SCORING 1
#define ENABLE_MUSIC 1
#define ENABLE_WEAPONS 1
#define ENABLE_COLLISIONS 1

extern const hUGESong_t intro_song;
extern const hUGESong_t main_song;
extern const hUGESong_t main_song_fast;

static const uint8_t blank_win_tiles[SCREEN_TILE_WIDTH] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static uint8_t health_bar_tiles[8];

static struct Sprite bullets[MAX_BULLETS];
static bool game_paused = true;

#if ENABLE_SCORING
static void increment_timer_score_isr(void) {
  if (game_paused) {
    return;
  }
  increment_timer_score();
}
#endif

static void wait(uint8_t num_frames) {
  for (uint8_t i = 0; i < num_frames; ++i) {
    vsync();
  }
}

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

// Loads sprite, tile, and font data.
static void load_data(void) {
  // Load font tiles to background map.
  font_init();
  font_t min_font = font_load(font_min);
  font_set(min_font);

  // Load background tiles.
  uint8_t tile_index = MAPBLOCK_IDX;
  set_bkg_data(tile_index, sizeof(block_tiles)/TILE_SIZE_BYTES, block_tiles);
  tile_index += sizeof(block_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(powerups_tiles)/TILE_SIZE_BYTES, powerups_tiles);
  tile_index += sizeof(powerups_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(progressbar_tiles)/TILE_SIZE_BYTES, progressbar_tiles);
  tile_index += sizeof(progressbar_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(font_extras_tiles)/TILE_SIZE_BYTES, font_extras_tiles);
  tile_index += sizeof(font_extras_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(tutorial_screen_tiles)/TILE_SIZE_BYTES, tutorial_screen_tiles);
  tile_index += sizeof(tutorial_screen_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(lock_tiles)/TILE_SIZE_BYTES, lock_tiles);
  tile_index += sizeof(lock_tiles)/TILE_SIZE_BYTES;
  set_bkg_data(tile_index, sizeof(cailan_games_logo_tiles)/TILE_SIZE_BYTES, cailan_games_logo_tiles);

  // Load sprite data.
  set_sprite_data(0, 10, player_data);
  set_sprite_data(10, 9, player_shield_data);
  set_sprite_data(19, 3, projectiles_data);
}

static void update_health_bar(int8_t health) {
  if (health == 100) {
    health_bar_tiles[0] = HEALTH_BAR_START;  // left edge of bar
    for (uint8_t i = 1; i < 7; ++i) {
      health_bar_tiles[i] = HEALTH_BAR_START + 1;  // center of bar
    }
    health_bar_tiles[7] = HEALTH_BAR_START + 2;  // right edge of bar
  }
  else if (health >= 88) {
    health_bar_tiles[0] = HEALTH_BAR_START;  // left edge of bar
    for (uint8_t i = 1; i < 7; ++i) {
      health_bar_tiles[i] = HEALTH_BAR_START + 1;  // center of bar
    }
    health_bar_tiles[7] = HEALTH_BAR_START + 5;  // right edge of bar
  }
  else if (health >= 16) {
    uint8_t idx = health / 12;
    health_bar_tiles[0] = HEALTH_BAR_START;  // left edge of bar
    for (uint8_t i = 1; i < 7; ++i) {
      if (i < idx) {
        health_bar_tiles[i] = HEALTH_BAR_START + 1;  // fill
      }
      else {
        health_bar_tiles[i] = HEALTH_BAR_START + 4;  // clear
      }
    }
    health_bar_tiles[7] = HEALTH_BAR_START + 5;  // clear right edge of bar
  }
  else if (health > 0) {
    health_bar_tiles[1] = HEALTH_BAR_START + 4;
    health_bar_tiles[0] = HEALTH_BAR_START;
  }
  else {
    health_bar_tiles[1] = HEALTH_BAR_START + 4;  // clear bottom 2 tiles
    health_bar_tiles[0] = HEALTH_BAR_START + 3;
  }
  set_win_tiles(0, 0, 8, 1, health_bar_tiles);
}

// Shows the logo screen.
static void show_logo_screen(bool with_transition) {
  HIDE_BKG;
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT+1, cailan_games_logo_map);
  play_all_channels();
  if (with_transition) {
    wait(60);
    fade_in();
    SHOW_BKG;
  } else {
    DISPLAY_ON;
  }
  // Initialize the CBT SFX and add it to the VBL
  CBTFX_PLAY_SFX_02;
  add_VBL(CBTFX_update);
  wait(60*2);

  // Remove the CBT SFX from tge VBL
  remove_VBL(CBTFX_update);
  mute_all_channels();

  // // Wait for the player to press start before going to the next screen.
  // waitpad(J_START);
  // waitpadup();
  fade_out();
  SHOW_WIN;
}
// Shows the title screen.
static void show_title_screen(bool with_transition) {
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT, game_titlescreen);
  if (with_transition) {
    wait(30);
    fade_in();
  } else {
    DISPLAY_ON;
  }
  SHOW_BKG;
  display_highscores();
  SHOW_WIN;
#if ENABLE_MUSIC
  hUGE_init(&intro_song);
  play_all_channels();
#endif
  // Wait for the player to press start before going to the next screen.
  waitpad(J_START);
  waitpadup();
}

// Shows the speed selection screen and returns the speed chosen by the player.
static uint8_t show_speed_selection_screen(void) {
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, COLUMN_HEIGHT, speed_titlescreen);
  set_sprite_tile(PLAYER_SPRITE_ID, 0);
  move_sprite(PLAYER_SPRITE_ID, 32, 72);
  SHOW_SPRITES;
  uint8_t scroll_speed = SCROLL_SPEED_NORMAL;
  uint8_t prev_input = 0;
  while (true) {
    uint8_t input = joypad();
    if (KEY_FIRST_PRESS(input, prev_input, J_UP)) {
      if (scroll_speed == SCROLL_SPEED_NORMAL) {
        move_sprite(PLAYER_SPRITE_ID, 32, 104);
        scroll_speed = SCROLL_SPEED_TURBO;
      }
      else if (scroll_speed == SCROLL_SPEED_HARD) {
        move_sprite(PLAYER_SPRITE_ID, 32, 72);
        scroll_speed = SCROLL_SPEED_NORMAL;
      }
      else {
        move_sprite(PLAYER_SPRITE_ID, 32, 88);
        scroll_speed = SCROLL_SPEED_HARD;
      }
    }
    else if (KEY_FIRST_PRESS(input, prev_input, J_DOWN)) {
      if (scroll_speed == SCROLL_SPEED_NORMAL) {
        move_sprite(PLAYER_SPRITE_ID, 32, 88);
        scroll_speed = SCROLL_SPEED_HARD;
      }
      else if (scroll_speed == SCROLL_SPEED_HARD) {
        move_sprite(PLAYER_SPRITE_ID, 32, 104);
        scroll_speed = SCROLL_SPEED_TURBO;
      }
      else {
        move_sprite(PLAYER_SPRITE_ID, 32, 72);
        scroll_speed = SCROLL_SPEED_NORMAL;
      }
    }
    else if (KEY_FIRST_PRESS(input, prev_input, J_START) || KEY_FIRST_PRESS(input, prev_input, J_A) || KEY_FIRST_PRESS(input, prev_input, J_B)) {
      break;
    }
    prev_input = input;
    vsync();
  }
  HIDE_SPRITES;
  return scroll_speed;
}

#if ENABLE_COLLISIONS
static void show_gameover_screen(void) {
  set_bkg_tiles(0, 0, SCREEN_TILE_WIDTH, 18, gameover_titlescreen);
  move_bkg(0, 0);

  display_gameover_scores();

  SHOW_BKG;
  fade_in();
  waitpad(J_START);
  waitpadup();
  fade_out();
  HIDE_BKG;
}

static void handle_gameover(void) {
  game_paused = true;
  mute_all_channels();
  set_sprite_tile(PLAYER_SPRITE_ID, DEATH_SPRITE);
  wait(10);
  HIDE_BKG;
  play_gameover_sound();
  fade_out();
  HIDE_SPRITES;

  // Hide all bullets.
  for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
    move_sprite(bullets[i].sprite_id, 0, 0);
  }
  update_health_bar(0);

  // Show gameover screen, then transition to the title screen.
  show_gameover_screen();
  show_title_screen(true);
}
#endif

#if ENABLE_WEAPONS
// Updates the collision map and background map in response to a dropped bomb. The bomb explosion
// is a square in front of the player with sides of length `2*BOMB_RADIUS+1` tiles.
static void drop_bomb(const struct Sprite* player, uint8_t* coll_map, uint8_t* bkg_map) {
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
#endif

void main(void) {
  /*
   * SETUP
   */
  load_data();
  // Load Window.
  move_win(7, 136);
  // Initialize high scores.
  init_highscores();

#if ENABLE_MUSIC
  // Load music.
  // Enable sound playback.
  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x33;

  // Mute all channels.
  mute_all_channels();

  // Add hUGE driver to VBL interrupt handler.
  add_VBL(hUGE_dosound);
#endif

#if ENABLE_SCORING
  // Add timer score incrementer to VBL interrupt handler.
  add_VBL(increment_timer_score_isr);
#endif

  /*
   * TITLE SCREEN
   */
  show_logo_screen(true);
  show_title_screen(true);

  uint8_t bomb_tiles[2];
  bool show_time = true;

  struct Sprite player;
  uint8_t active_bullet_count;
  bool bomb_dropped;
  bool shield_active;
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
  int16_t damage_animation_counter;  // When hit, skip checking collisions for this long
  uint8_t col_count;  // counter for number of columns scrolled. Used to calculate screen_count
  uint16_t screen_count;  // number of screens scrolled
  uint8_t scroll_pixels_per_frame;  // How many pixels to scroll each frame
  uint8_t player_collision;
  uint8_t bullet_collision;
  enum animation_state damage_animation_state;  // Used during the damage recovery to toggle between showing and hidding the player sprite

  uint8_t player_sprite_base_id;

  // Temporary variables (loop counters, array indices, etc)
  uint8_t i, j;
  uint16_t ii;

  // Banking variables
  uint8_t last_bank;

  while (true) {
    /*
     * Set up for main game loop.
     */
    initrand(DIV_REG);
    // Load the window contents.
    n_bombs = MAX_BOMBS;
    bomb_tiles[0] = BOMB_ICON_IDX;
    bomb_tiles[1] = 0; //n_bombs + 1;
    health_bar_tiles[0] = HEALTH_BAR_START;  // left edge of bar
    for (i = 1; i < 7; ++i) {
      health_bar_tiles[i] = HEALTH_BAR_MIDDLE;  // center of bar
    }
    health_bar_tiles[7] = HEALTH_BAR_END;  // right edge of bar

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
    player.lifespan = 0;  // lifespan isn't used by the player sprite.
    player.active = true;
    player.type = PLAYER;
    move_sprite(player.sprite_id, player.x, player.y);

    // Create bullets.
    for (uint8_t i = 0; i < MAX_BULLETS; ++i) {
      struct Sprite* b = &(bullets[i]);
      b->active = false;
      b->type = BULLET;
      b->sprite_id = i + 1;  // +1 so we don't override the player (always sprite_id 0)
      b->sprite_tile_id = 19;
      b->lifespan = 0;
      b->health = 0;
      b->dir = RIGHT;
      b->speed = 4;
      b->x = 0;
      b->y = 0;
      // Collision box
      b->cb_x_offset = 0;
      b->cb_y_offset = 2;
      b->cb.x = b->x + b->cb_x_offset;
      b->cb.y = b->y + b->cb_y_offset;
      b->cb.w = 4;
      b->cb.h = 4;
      set_sprite_tile(b->sprite_id, b->sprite_tile_id);
      move_sprite(b->sprite_id, b->x, b->y);
    }
    active_bullet_count = 0;

    // Clear collision map and background map
    for (ii = 0; ii < COLUMN_HEIGHT*ROW_WIDTH; ++ii) {
      coll_map[ii] = 0;
      bkg_map[ii] = 0;
    }

    // Initialize other variables.
    input = 0;
    prev_input = 0;
    dx = 0;
    dy = 0;
    frame_count = 0;
    scroll_count = 0;
    damage_animation_counter = 0;
    col_count = 0;
    screen_count = 0;
    player_collision = 0;
    bullet_collision = 0;
    damage_animation_state = HIDDEN;
    gen_state.biome_id = 0;
    gen_state.biome_column_index = 0;
    gen_column_index = SCREEN_TILE_WIDTH;
    bomb_dropped = false;
    shield_active = false;
    copy_bkgmap_to_vram = false;
    player_sprite_base_id = 0;

    /*
     * SPEED SELECTION SCREEN
     */
    scroll_pixels_per_frame = show_speed_selection_screen();

    // Game Start
    // Set player start location
    move_sprite(player.sprite_id, player.x, player.y);

    // Copy tutorial screen to bkg_map
    for (i = 0; i < COLUMN_HEIGHT; ++i) {
      for (j = 0; j < SCREEN_TILE_WIDTH; ++j) {
        bkg_map[i*ROW_WIDTH+j] = tutorial_screen_map[i*SCREEN_TILE_WIDTH+j];
      }
    }
    set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, bkg_map);

    set_win_tiles(0, 0, SCREEN_TILE_WIDTH, 1, blank_win_tiles);
    set_win_tiles(9, 0, 2, 1, bomb_tiles);
    set_win_tiles(0, 0, 8, 1, health_bar_tiles);

#if ENABLE_MUSIC
    mute_all_channels();
#endif

    wait(10);

#if ENABLE_MUSIC
    if (scroll_pixels_per_frame == SCROLL_SPEED_NORMAL) {
      hUGE_init(&main_song);
    }
    else {
      hUGE_init(&main_song_fast);
    }
    play_all_channels();
#endif

    SHOW_SPRITES;
    SHOW_WIN;

    last_bank = CURRENT_BANK;
    SWITCH_ROM(1);
    for (uint8_t i = 0; i < ROW_WIDTH - SCREEN_TILE_WIDTH; ++i) {
      generate_next_column(&gen_state, coll_map+gen_column_index, bkg_map+gen_column_index);
      if (++gen_column_index >= ROW_WIDTH) {
        gen_column_index = 0;
      }
    }
    set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, bkg_map);
    SWITCH_ROM(last_bank);

    wait(15);

    // Reset scores and window tiles.
    reset_scores();
    display_timer_score();

    // Unpausing the game turns on the timer, so this should be done as close as possible to the
    // main game loop.
    game_paused = false;

    /*
     * MAIN GAME LOOP
     */
    while (true) {
      input = joypad();
      player_collision = 0;
      bullet_collision = 0;
      copy_bkgmap_to_vram = false;

      if (KEY_PRESSED(input, J_START)) {
        game_paused = true;
#if ENABLE_MUSIC
        mute_all_channels();
#endif
        waitpadup();
        waitpad(J_START);
        waitpadup();
#if ENABLE_MUSIC
        play_all_channels();
#endif
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

#if ENABLE_WEAPONS
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
        --n_bombs;
        play_bomb_sound();
        if (n_bombs == 0){
          bomb_tiles[0] = BOMB_SIL_ICON_IDX;
        }
        else {
          bomb_tiles[0] = BOMB_ICON_IDX;
        }
      }
#endif

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

#if ENABLE_WEAPONS
      /*
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
          }
#if ENABLE_COLLISIONS
          else {
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
#endif
          // Update bullet's sprite position.
          move_sprite(b->sprite_id, b->x, b->y);
        }
      }

      if (bomb_dropped) {
        drop_bomb(&player, coll_map, bkg_map);
        copy_bkgmap_to_vram = true;
        bomb_dropped = false;
      }
#endif

      /*
       * Continue processing
       */
#if ENABLE_COLLISIONS
      if (damage_animation_counter == 0) {
        // Damage animation or shield powerup expired.
        // Reset so that player is shown
        if (shield_active) {
          shield_active = false;
          player_sprite_base_id -= 10;
        }

        if (damage_animation_state == HIDDEN) {
          move_sprite(player.sprite_id, player.x, player.y);
          damage_animation_state = SHOWN;
        }

        // Normal collision check for player
        player_collision = check_collisions(&player, coll_map, bkg_map, true, false);
        if (player_collision > 0 && player_collision < 235) {
          player.health -= COLLISION_DAMAGE;
          damage_animation_counter = COLLISION_TIMEOUT;
          player_collision = 1;
        }
        else if (player_collision == HEALTH_KIT_ID) {
          if (player.health < 100) {
            // Prevent health overflow (int8 maxes at 128)
            if ((100 - player.health) >= HEALTH_KIT_VALUE) {
              if(player.health < 20) {
                player.health += 4*HEALTH_KIT_VALUE;
              }
              else if (player.health < 50) {
                player.health += 2*HEALTH_KIT_VALUE;
              }
              else {
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
          // player_sprite_base_id += 10;
          player_collision = 0;
          play_shield_sound();
        }

        if (player.health <= 0) {
          handle_gameover();
          break;
        }

        // Update health bar after a collision
        if (player_collision) {
          update_health_bar(player.health);
        }
      }
      else {
        --damage_animation_counter;
        if (damage_animation_counter < 0) {
          damage_animation_counter = 0;
        }

        if (!shield_active) {
          if (damage_animation_state == HIDDEN) {
            move_sprite(player.sprite_id, player.x, player.y);
            damage_animation_state = SHOWN;
          }
          else {
            move_sprite(player.sprite_id, 0, 0);
            damage_animation_state = HIDDEN;
          }

          // Check for collision and only process pickups if the shield is not active
          // This will allow the player to pick up items while the flashing animation is playing
          player_collision = check_collisions(&player, coll_map, bkg_map, true, true);

          if (player_collision == HEALTH_KIT_ID) {
            if (player.health < 100) {
              // Prevent health overflow (int8 maxes at 128)
              if ((100 - player.health) >= HEALTH_KIT_VALUE) {
                if(player.health < 20) {
                  player.health += 4*HEALTH_KIT_VALUE;
                }
                else if (player.health < 50) {
                  player.health += 2*HEALTH_KIT_VALUE;
                }
                else {
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
            // player_sprite_base_id += 10;
            player_collision = 0;
            play_shield_sound();
          }
        }
      }

      // Update the ship sprite based on the current health.
      if (player.health > 50) {
        player_sprite_base_id = 0;
      }
      else if (player.health > 25) {
        player_sprite_base_id = 3;
      }
      else if (player.health > 0) {
        player_sprite_base_id = 6;
      }

      if (shield_active){
        player_sprite_base_id += 10;
      }
#endif

      // Scroll the screen.
      scroll_bkg(scroll_pixels_per_frame, 0);
      scroll_count += scroll_pixels_per_frame;

      if (scroll_count >= 8) {
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

#if ENABLE_WEAPONS
          // Add a bomb every few screens we scroll.
          if (screen_count % 3 == 0 && n_bombs < MAX_BOMBS) {
            ++n_bombs;
            bomb_tiles[0] = BOMB_ICON_IDX;
          }
#endif
        }
      }

#if ENABLE_WEAPONS
      if ((frame_count & 0x3) == 0) { // %4
        // Update HUD
        bomb_tiles[1] = 0;// n_bombs + 1;
        set_win_tiles(9, 0, 2, 1, bomb_tiles);
      }

      ++frame_count;
      if (frame_count >= 255) {
        frame_count = 0;
      }
#endif

#if ENABLE_COLLISIONS
      if (player_collision || bullet_collision) {
        // Update bkg_map if there are collisions
        copy_bkgmap_to_vram = true;
      }
#endif

      // Wait for frame to finish drawing
      vsync();

#if ENABLE_SCORING
      if (update_score_tiles){
        if (show_time) {
          display_timer_score();
          update_score_tiles = false;
        } else {
          display_point_score();
          update_score_tiles = false;
        }
      }
#endif

      if (copy_bkgmap_to_vram) {
        // Write the entire map to VRAM
        set_bkg_tiles(0, 0, ROW_WIDTH, COLUMN_HEIGHT, bkg_map);
      }
    }
  }
}
